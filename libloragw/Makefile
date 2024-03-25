### get external defined data

LIBLORAGW_VERSION := `cat ../VERSION`
include library.cfg


### constant symbols

ARCH ?=
CROSS_COMPILE ?=
CC := $(CROSS_COMPILE)gcc
AR := $(CROSS_COMPILE)ar

CFLAGS := -O2 -Wall -Wextra -std=c99 -Iinc -I. -I../libtools/inc  
OBJDIR = obj
INCLUDES = $(wildcard inc/*.h) 


### linking options

LIBS := -lloragw -ltinymt32 -lrt -lm

### general build targets

all: libloragw.a  

clean:
	rm -f libloragw.a
	rm -f test_loragw_*
	rm -f $(OBJDIR)/*.o
	
install:

### transpose library.cfg into a C header file : config.h

inc/config.h: ../VERSION library.cfg
	@echo "*** Checking libloragw library configuration ***"
	@rm -f $@
	#File initialization
	@echo "#ifndef _LORAGW_CONFIGURATION_H" >> $@
	@echo "#define _LORAGW_CONFIGURATION_H" >> $@
	# Release version
	@echo "Release version   : $(LIBLORAGW_VERSION)"
	@echo "	#define LIBLORAGW_VERSION	"\"$(LIBLORAGW_VERSION)\""" >> $@
	# Debug options
	@echo "	#define DEBUG_AUX	$(DEBUG_AUX)" >> $@
	@echo "	#define DEBUG_SPI	$(DEBUG_SPI)" >> $@
	@echo "	#define DEBUG_I2C	$(DEBUG_I2C)" >> $@
	@echo "	#define DEBUG_REG	$(DEBUG_REG)" >> $@
	@echo "	#define DEBUG_HAL	$(DEBUG_HAL)" >> $@
	@echo "	#define DEBUG_GPS	$(DEBUG_GPS)" >> $@
	@echo "	#define DEBUG_GPIO	$(DEBUG_GPIO)" >> $@
	@echo "	#define DEBUG_LBT	$(DEBUG_LBT)" >> $@
	@echo "	#define DEBUG_RAD	$(DEBUG_RAD)" >> $@
	@echo "	#define DEBUG_CAL	$(DEBUG_CAL)" >> $@
	@echo "	#define DEBUG_SX1302	$(DEBUG_SX1302)" >> $@
	# end of file
	@echo "#endif" >> $@
	@echo "*** Configuration seems ok ***"
### library module target

$(OBJDIR):
	mkdir -p $(OBJDIR)

$(OBJDIR)/%.o: src/%.c $(INCLUDES) inc/config.h | $(OBJDIR)
	$(CC) -c $(CFLAGS) $< -o $@

### static library

libloragw.a: $(OBJDIR)/loragw_spi.o $(OBJDIR)/loragw_i2c.o $(OBJDIR)/loragw_aux.o $(OBJDIR)/loragw_reg.o $(OBJDIR)/loragw_sx1250.o $(OBJDIR)/loragw_sx125x.o $(OBJDIR)/loragw_sx1302.o $(OBJDIR)/loragw_cal.o $(OBJDIR)/loragw_debug.o $(OBJDIR)/loragw_hal.o $(OBJDIR)/loragw_stts751.o $(OBJDIR)/loragw_gps.o $(OBJDIR)/loragw_sx1302_timestamp.o $(OBJDIR)/loragw_sx1302_rx.o
	$(AR) rcs $@ $^


### EOF
