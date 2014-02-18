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

space	:= $(empty) $(empty)
comma	:= $(empty),$(empty)

CC	?= $(CROSS_COMPILE)gcc
OBJCOPY ?= $(CROSS_COMPILE)objcopy

TOPDIR	:= $(PWD)/$(shell dirname $(MAKEFILE_LIST))

SUBMK	= $(wildcard tests/*.mk)
TESTS	= $(wildcard tests/*.c)
REPORTS	= $(wildcard framework/cobaye_report_*.c)
RTYPES	= $(subst $(space),$(comma),$(REPORTS:framework/cobaye_report_%.c=%))

RES		= README
OBJS		= $(TESTS:.c=.o) 
ROBJS		= $(REPORTS:.c=.o)

FOBJS		= framework/cobaye_main.o \
		   framework/cobaye_tests.o \
		   framework/cobaye_seq.o \
		   framework/cobaye_report.o \
		   $(ROBJS) \
		   framework/cobaye.o

INCDIR		= -Iframework
TARGET		= cobaye
EXT_CFLAGS	+= -g -Wall -Wextra -Wno-unused -Wno-unused-parameter -D_GNU_SOURCE
CFLAGS		+= $(EXT_CFLAGS) -DCOBAYE_FRAMEWORK
FW_CFLAGS	= -DRTYPE=\"$(RTYPES)\"

ARCH		= $(shell $(CC) -dumpmachine | awk -F- '{ print $$1}')
SYSTEM		= $(shell $(CC) -dumpmachine | awk -F- '{ print $$(NF-1)}')

include $(SUBMK)

MACH		?= $(ARCH)
L3_OBJS         = $(filter-out $($(MACH)_BLACKLIST), $(OBJS))
L3_LDFLAGS      = $(filter-out $($(MACH)_BLACKLIST), $(LDFLAGS))
L2_OBJS         = $(filter-out $($(SYSTEM)_BLACKLIST), $(L3_OBJS))
L2_LDFLAGS      = $(filter-out $($(SYSTEM)_BLACKLIST), $(L3_LDFLAGS))
L1_OBJS         = $(filter-out $($(TARGET)_BLACKLIST), $(L2_OBJS))
L1_LDFLAGS      = $(filter-out $($(TARGET)_BLACKLIST), $(L2_LDFLAGS))

L_OBJS		:= $(FOBJS) $(L1_OBJS) framework/zztest.o
L_LDFLAGS	= $(L1_LDFLAGS) -lpthread -lrt

all: $(TARGET)

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
	$E $(CC) -c -o $@ $^ $(INCDIR) $(FW_CFLAGS) $(CFLAGS)

.PHONY: clean
clean:
	$P '  RM		TARGET'
	$E rm -f $(TARGET)
	$P '  RM		FRAMEWORK OBJS'
	$E rm -f framework/*.o
	$P '  RM		TEST OBJS'
	$E rm -f tests/*.o
	$P '  RM		LDS'
	$E rm -f cobaye.lds
