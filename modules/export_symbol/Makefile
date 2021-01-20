KERN_DIR = /lib/modules/$(shell uname -r)/build

obj-m += mod_a.o
obj-m += mod_b.o

all:
	make -C $(KERN_DIR) M=`pwd` modules

clean:
	make -C $(KERN_DIR) M=`pwd` modules clean
	rm -rf modules.order

