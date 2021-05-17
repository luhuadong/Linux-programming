PWD := $(shell pwd)
KVER := $(shell uname -r)
KDIR :=/lib/modules/$(KVER)/build/

# Kernel modules
obj-m := hello.o

# Specify flags for the module compilation
#EXTRA_CFLAGS=-g -O0

all:
	$(MAKE) -C $(KDIR) M=$(PWD) modules
clean:
	$(MAKE) -C $(KDIR) M=$(PWD) clean