INSTALL_DIR=/usr/local
RM=/bin/rm -vf
CC=gcc
ECHO=/bin/echo -e
LD=ld
PWD=pwd
CDR=$(shell pwd)


LDFLAGS=-shared
override CFLAGS+= -O2 -std=gnu99 -fPIC -Wall -I include/

EDLDFLAGS:=$(LDFLAGS)

LIBEXT=so
LINKOPTIONS:=-shared
RMDIR=rmdir -v

TARGETLIB=libpicc_i2c_thsensor.$(LIBEXT)
LINKOPTIONS:=-L build/

lib_objects = src/picc_i2c_trh.o
lib_example = example/example.c

all: build/$(TARGETLIB) example/example

build:
	mkdir build

build/$(TARGETLIB): $(lib_objects) build
	$(LD) $(LDFLAGS) $(lib_objects) -o $@

example/example: $(lib_example)
	$(CC) -std=gnu99  -o $@ -Iinclude/ $(LINKOPTIONS) $(lib_example) -lpicc_i2c_thsensor

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@ 

install: build/$(TARGETLIB)
	cp $< $(INSTALL_DIR)/lib/$(TARGETLIB)


clean:
	$(RM) $(lib_objects) 
	$(RM) example/example

spotless: clean
	$(RM) build/$(TARGETLIB)
	$(RMDIR) build

uninstall:
	$(RM) $(INSTALL_DIR)/lib/$(TARGETLIB)
