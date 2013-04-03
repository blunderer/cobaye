#
# Cobaye test Framework
#

V ?= 0

ifeq ($V, 0)
E = @
P = @echo 
else
E = 
P = @true
endif

CC	:= $(CROSS_COMPILE)gcc
OBJCOPY := $(CROSS_COMPILE)objcopy

TOPDIR	:= $(PWD)/$(shell dirname $(MAKEFILE_LIST))

SHELL	= /bin/bash
SUBMK	= $(wildcard tests/*.mk)

TESTS	= $(wildcard tests/*.c)
OBJS	= framework/cobaye_main.o \
		framework/cobaye.o \
		framework/cobaye_tests.o \
		framework/cobaye_menu_core.o \
		framework/cobaye_menus.o \
		framework/cobaye_display.o \
		framework/cobaye_seq.o \
		$(TESTS:.c=.o) \
		framework/zztest.o

RES	= README

INCDIR	= -Iinclude

TARGET	= cobaye

EXT_CFLAGS	+= -g -Wall -Wextra -Wno-unused -D_GNU_SOURCE
CFLAGS		+= $(EXT_CFLAGS) -DCOBAYE_FRAMEWORK

LDFLAGS	+= -lncurses -lpthread -lrt

x86_BLACKLIST = 
arm_BLACKLIST = 

ARCH		= unknown
MACH		= $(shell machine=$$($(CC) -dumpmachine) && echo $${machine:0:3})
L_OBJS		=
L_LDFLAGS	=

include $(SUBMK)

ifeq ($(MACH), x86)
ARCH		= $(MACH)
L_OBJS		= $(filter-out $(x86_BLACKLIST), $(OBJS))
L_LDFLAGS	= $(filter-out $(x86_BLACKLIST), $(LDFLAGS))
endif

ifeq ($(MACH), arm)
ARCH		= $(MACH)
L_OBJS		= $(filter-out $(arm_BLACKLIST), $(OBJS))
L_LDFLAGS	= $(filter-out $(arm_BLACKLIST), $(LDFLAGS))
endif

all arm x86: $(TARGET)

test: $(TARGET)
	$P '  EXEC		$@'
	$E ./cobaye

$(TARGET): $(L_OBJS) cobaye.lds
	$P '  LD		TARGET'
	$E $(CC) -o $@ $^ $(L_LDFLAGS)

cobaye.lds: $(RES)
	$P '  GEN		$@' 
	$E echo "SECTIONS {" > $@
	@for res in $(RES); do \
		ressecname=.cobaye.$$(basename $$res); \
		resvarname=$$(basename $$res | tr "." "_"); \
		echo "__$${resvarname}_data = ADDR($${ressecname});" >> $@; \
		echo "__$${resvarname}_size = SIZEOF($${ressecname});" >> $@; \
	done;
	$E echo "}" >> $@

tests/%.o: tests/%.c framework/cobaye_tests.o
	$P '  CC [TST]	$@'
	$E $(TOPDIR)/validate_tests.sh $<
	$E $(CC) -c -o $@ $< $(INCDIR) $(CFLAGS) $(CFLAGS_$(shell NAME=$@ && basename $${NAME%.o}))

tests/%.ext: tests/%.c
	$P '  CC [EXT]	$@'
	$E $(CC) -o $@ $< $(INCDIR) $(EXT_CFLAGS) $(CFLAGS_$(shell NAME=$@ && basename $${NAME%.o})) $(L_LDFLAGS)

framework/zztest.o: framework/zztest.c $(RES)
	$P '  CC 		$@'
	$E $(CC) -c -o $@ $< $(INCDIR) $(CFLAGS)
	@for res in $(RES); do \
		echo "  LD [RES]  	$$res"; \
		ressecname=.cobaye.$$(basename $$res); \
		$(OBJCOPY) --add-section $$ressecname=$$res --set-section-flags $$ressecname=load,alloc,data,readonly $@; \
	done;

%.o: %.c
	$P '  CC 		$@'
	$E $(CC) -c -o $@ $^ $(INCDIR) $(CFLAGS)

.PHONY: clean
clean:
	$P '  RM		TARGET'
	$E rm -f $(TARGET)
	$P '  RM		OBJS'
	$E rm -f $(L_OBJS)
	$P '  RM		LDS'
	$E rm -f cobaye.lds

