# Host tests for the PicoGUS web server: real lwIP + web server code on a
# loopback interface. Needs a host C/C++ compiler, Python 3 and the Pico SDK
# (for its copy of lwIP).
#
#   make PICO_SDK_PATH=/path/to/pico-sdk test

PICO_SDK_PATH ?= $(error set PICO_SDK_PATH)
LWIP := $(PICO_SDK_PATH)/lib/lwip
SW := ../..
OUT := build

LWIP_SRC := $(addprefix $(LWIP)/src/core/, init.c def.c dns.c inet_chksum.c ip.c mem.c memp.c netif.c \
	pbuf.c raw.c stats.c sys.c tcp.c tcp_in.c tcp_out.c timeouts.c udp.c) \
	$(addprefix $(LWIP)/src/core/ipv4/, acd.c dhcp.c etharp.c icmp.c igmp.c ip4.c ip4_addr.c ip4_frag.c) \
	$(LWIP)/src/netif/ethernet.c
WEB_SRC := ../web_http.c ../web_upload.c

DEFS := -DPICOGUS_WEB_MDNS=0 -DWEBSERVER=1 -DSOUND_MPU=1 -DSOUND_SB=1 -DSOUND_OPL=1 -DPICOGUS_MULTIFW=1
# test dir first: it overrides lwipopts.h and stubs the Pico SDK headers
INC := -I. -Istubs -I.. -I$(SW) -I$(LWIP)/src/include -I$(LWIP)/contrib/ports/unix/port/include -I$(OUT)
CFLAGS := -O1 -g -Wall -Wno-unused-function -Wno-unused-variable -fsanitize=address,undefined $(DEFS) $(INC)

$(OUT)/test_web: test_web.c defaults.c $(WEB_SRC) ../web_api.cpp $(OUT)/web_assets.h ../*.h ../lwipopts.h lwipopts.h
	gcc $(CFLAGS) -std=gnu11 -c test_web.c -o $(OUT)/test_web.o
	gcc $(CFLAGS) -std=gnu11 -c defaults.c -o $(OUT)/defaults.o
	gcc $(CFLAGS) -std=gnu11 -c ../web_http.c -o $(OUT)/web_http.o
	gcc $(CFLAGS) -std=gnu11 -c ../web_upload.c -o $(OUT)/web_upload.o
	g++ $(CFLAGS) -std=gnu++17 -c ../web_api.cpp -o $(OUT)/web_api.o
	gcc $(CFLAGS) -std=gnu11 -w $(LWIP_SRC) $(OUT)/*.o -lstdc++ -o $@

$(OUT)/test_midi: test_midi.c $(SW)/mpu401/midi.c $(SW)/mpu401/export.h
	mkdir -p $(OUT)
	gcc $(CFLAGS) -std=gnu11 test_midi.c $(SW)/mpu401/midi.c -o $@

$(OUT)/web_assets.h: ../www/index.html $(SW)/scripts/web_embed.py
	mkdir -p $(OUT)
	python3 $(SW)/scripts/web_embed.py $< $@

test: $(OUT)/test_web $(OUT)/test_midi
	cd $(OUT) && ./test_midi
	cd $(OUT) && ./test_web
	python3 -c "import json; d = json.load(open('$(OUT)/status.json')); print('status JSON ok:', len(d['s']), 'settings')"

clean:
	rm -rf $(OUT)

.PHONY: test clean
