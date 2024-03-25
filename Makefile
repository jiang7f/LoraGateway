
### Environment constants

ARCH ?=
CROSS_COMPILE ?=
# CC := $(CROSS_COMPILE)gcc
CFLAGS1 := -O2 -Wall -Wextra -std=c99 -Igateway/inc -I. -Ilibtools/inc -Ilibloragw/inc -lpaho-mqtt3c -lpaho-mqtt3a
LIBS := -luser -lloragw -lpthread -ltinymt32 -lrt -lm
export

### general build targets

.PHONY: all clean install install_conf libtools libloragw 

all: libtools libloragw gateway LoraServer
# 
libtools:
	$(MAKE) all -e -C $@

libloragw: libtools
	$(MAKE) all -e -C $@

gateway: libloragw
	$(MAKE) all -e -C $@

LoraServer: gateway/src/LoraServer.c gateway libtools libloragw
	$(CC) $(CFLAGS1) -L. -L ./libloragw -L./gateway -L./libtools $< -o $@ $(LIBS)

clean:
	$(MAKE) clean -e -C libtools
	$(MAKE) clean -e -C libloragw
	$(MAKE) clean -e -C gateway
	rm -f loragw_server

### EOF

