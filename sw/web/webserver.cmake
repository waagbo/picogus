# Web server for PicoGUS on Pico W (CYW43) boards.
# Included from the top level CMakeLists.txt; see web/README.md.

set(PICOGUS_WEB_DIR ${CMAKE_CURRENT_LIST_DIR})
# mDNS (http://picogus.local/) costs ~12KB of RAM, which Sound Blaster mode needs
# for CD images made of many track files. Off by default: the address is shown
# by "pgusinit /wifistatus", and the card registers itself as "picogus" via DHCP.
option(WEB_MDNS "Answer mDNS queries for picogus.local in the web server" OFF)

if(NOT Python3_FOUND)
    message(FATAL_ERROR "The web server needs Python 3 to embed its web page")
endif()

# The web page, gzip compressed into a C array
set(WEB_ASSETS_DIR ${CMAKE_BINARY_DIR}/generated/web)
add_custom_command(
    OUTPUT ${WEB_ASSETS_DIR}/web_assets.h
    COMMAND ${CMAKE_COMMAND} -E make_directory ${WEB_ASSETS_DIR}
    COMMAND ${Python3_EXECUTABLE} ${CMAKE_SOURCE_DIR}/scripts/web_embed.py
            ${PICOGUS_WEB_DIR}/www/index.html ${WEB_ASSETS_DIR}/web_assets.h
    DEPENDS ${PICOGUS_WEB_DIR}/www/index.html ${CMAKE_SOURCE_DIR}/scripts/web_embed.py
    VERBATIM
)
add_custom_target(web_assets DEPENDS ${WEB_ASSETS_DIR}/web_assets.h)

# CYW43 WiFi firmware blob, which the multifw image stores once for all modes.
# It sits right after the 32KB bootloader; the mode firmwares follow it.
set(PICOGUS_CYW43_FW_HEADER ${PICO_SDK_PATH}/lib/cyw43-driver/firmware/w43439A0_7_95_49_00_combined.h)
set(PICOGUS_CYW43_FW_OFFSET 32768)
file(STRINGS ${PICOGUS_CYW43_FW_HEADER} _fw_len REGEX "#define CYW43_WIFI_FW_LEN")
file(STRINGS ${PICOGUS_CYW43_FW_HEADER} _clm_len REGEX "#define CYW43_CLM_LEN")
string(REGEX MATCH "\\(([0-9]+)\\)" _m "${_fw_len}")
set(PICOGUS_CYW43_WIFI_FW_LEN ${CMAKE_MATCH_1})
string(REGEX MATCH "\\(([0-9]+)\\)" _m "${_clm_len}")
set(PICOGUS_CYW43_CLM_LEN ${CMAKE_MATCH_1})
if(NOT PICOGUS_CYW43_WIFI_FW_LEN OR NOT PICOGUS_CYW43_CLM_LEN)
    message(FATAL_ERROR "Could not read the CYW43 firmware size from ${PICOGUS_CYW43_FW_HEADER}")
endif()
# blob = firmware padded to 512 bytes + CLM, region rounded up to a flash sector
math(EXPR PICOGUS_CYW43_FW_REGION
    "((((${PICOGUS_CYW43_WIFI_FW_LEN} + 511) / 512) * 512 + ${PICOGUS_CYW43_CLM_LEN} + 4095) / 4096) * 4096")
set(PICOGUS_CYW43_FW_BIN ${CMAKE_BINARY_DIR}/cyw43_firmware.bin)

function(picogus_add_cyw43_fw_bin)
    add_custom_command(
        OUTPUT ${PICOGUS_CYW43_FW_BIN}
        COMMAND ${Python3_EXECUTABLE} ${CMAKE_SOURCE_DIR}/scripts/cyw43_fw_extract.py
                ${PICOGUS_CYW43_FW_HEADER} ${PICOGUS_CYW43_FW_BIN}
        DEPENDS ${PICOGUS_CYW43_FW_HEADER} ${CMAKE_SOURCE_DIR}/scripts/cyw43_fw_extract.py
        VERBATIM
    )
    add_custom_target(cyw43_firmware_bin DEPENDS ${PICOGUS_CYW43_FW_BIN})
endfunction()

# The network stack shares the RP2040's RAM with the sound card emulation (all
# code runs from RAM), so it is compiled for size. It only ever runs at the
# lowest priority on core 1, where speed matters little. Done once, on first use,
# so firmwares without the web server (NE2000) keep their compiler flags.
function(picogus_web_optimize_for_size)
    get_property(_done GLOBAL PROPERTY PICOGUS_WEB_OS_DONE)
    if(_done)
        return()
    endif()
    set_property(GLOBAL PROPERTY PICOGUS_WEB_OS_DONE TRUE)
    if(NOT PICO_LWIP_PATH)
        set(PICO_LWIP_PATH ${PICO_SDK_PATH}/lib/lwip)
    endif()
    file(GLOB_RECURSE _net_sources
        ${PICO_LWIP_PATH}/src/*.c
        ${PICO_SDK_PATH}/lib/cyw43-driver/src/*.c
        ${PICO_SDK_PATH}/src/rp2_common/pico_cyw43_driver/*.c
        ${PICO_SDK_PATH}/src/rp2_common/pico_cyw43_arch/*.c
        ${PICO_SDK_PATH}/src/rp2_common/pico_lwip/*.c
        ${PICO_SDK_PATH}/src/rp2_common/pico_async_context/*.c
        ${PICOGUS_WEB_DIR}/*.c
        ${PICOGUS_WEB_DIR}/*.cpp
    )
    set_source_files_properties(${_net_sources} PROPERTIES COMPILE_OPTIONS "-Os")
endfunction()

# Add the web server to a sound card firmware target
function(picogus_add_webserver TARGET_NAME MULTIFW)
    picogus_web_optimize_for_size()
    target_sources(${TARGET_NAME} PRIVATE
        ${PICOGUS_WEB_DIR}/web_main.c
        ${PICOGUS_WEB_DIR}/web_http.c
        ${PICOGUS_WEB_DIR}/web_api.cpp
        ${PICOGUS_WEB_DIR}/web_upload.c
    )
    target_include_directories(${TARGET_NAME} PRIVATE ${PICOGUS_WEB_DIR} ${WEB_ASSETS_DIR})
    add_dependencies(${TARGET_NAME} web_assets)
    if(WEB_MDNS)
        set(_mdns 1)
        target_link_libraries(${TARGET_NAME} pico_lwip_mdns)
    else()
        set(_mdns 0)
    endif()
    target_compile_definitions(${TARGET_NAME} PRIVATE
        WEBSERVER=1
        PICOGUS_WEB_MDNS=${_mdns}
        CYW43_CONFIG_FILE="web_cyw43_config.h"
        # 2 PIO cycles per SPI bit: 370MHz / 4 / 2 = 46MHz, within the CYW43439's 50MHz
        CYW43_PIO_CLOCK_DIV_INT=4
        # lwIP and the CYW43 driver run on core 1: give it the whole 4KB scratch bank
        PICO_CORE1_STACK_SIZE=0x1000
    )
    if(MULTIFW)
        target_compile_definitions(${TARGET_NAME} PRIVATE
            # all modes in one image: the web UI can switch between them
            PICOGUS_MULTIFW=1
            CYW43_CHIPSET_FIRMWARE_INCLUDE_FILE="web_cyw43_fw_shared.h"
            PICOGUS_CYW43_FW_OFFSET=${PICOGUS_CYW43_FW_OFFSET}
            PICOGUS_CYW43_WIFI_FW_LEN=${PICOGUS_CYW43_WIFI_FW_LEN}
            PICOGUS_CYW43_CLM_LEN=${PICOGUS_CYW43_CLM_LEN}
        )
    endif()
    target_link_libraries(${TARGET_NAME} pico_cyw43_arch_lwip_poll)
    # Keep the CYW43 driver's waits on core 1 (see web_main.c)
    target_link_options(${TARGET_NAME} PRIVATE
        "LINKER:--wrap=cyw43_delay_ms"
        "LINKER:--wrap=cyw43_delay_us"
        "LINKER:--wrap=cyw43_await_background_or_timeout_us"
        "LINKER:--wrap=sleep_ms"
        "LINKER:--wrap=sleep_us"
    )
endfunction()
