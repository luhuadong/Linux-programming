
#KERN_DIR = /home/rudy/workspace_hd/GYT/advantech/kernel/kernel_3.10.17
KERN_DIR=/lib/modules/$(shell uname -r)/build

obj-m += hello.o

all:
	make -C $(KERN_DIR) M=`pwd` modules

clean:
	make -C $(KERN_DIR) M=`pwd` modules clean
	rm -rf modules.order

