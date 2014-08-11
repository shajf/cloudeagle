CFILES = csp_kernel_manager_test.c
CFILES += csp_kernel.c
CFILES += csp_kernel_manager.c
CFILES += csp_kernel_record.c
CC = gcc
KSRC = /usr/src/linux-2.6.39
EXTRA_CFLAGS = -DLINUX -D__KERNEL__ -DMODULE -O2 -pipe -Wall
EXTRA_CFLAGS += -I$(KSRC)/include -I$(PWD)

TARGET = CSPKernel.ko
obj-m += CSPKernel.o
CSPKernel-objs := $(CFILES:.c=.o)

default:
	$(MAKE) -C $(KSRC) SUBDIRS=$(shell pwd) modules

clean:
	@rm -fr $(TARGET) $(TARGET:.ko=.o) $(TARGET:.ko=.mod.c) $(TARGET:.ko=.mod.o) $(CFILES:.c=.o) .*cmd \
	.tmp_versions Module.symvers *.order
