
KERN_DIR = /home/rudy/workspace/Projects/Bocon/tmp_kernel_5.4_k37x
#KERN_DIR=/lib/modules/$(shell uname -r)/build

obj-m += hello.o

all:
	make -C $(KERN_DIR) M=`pwd` modules

clean:
	make -C $(KERN_DIR) M=`pwd` modules clean
	rm -rf modules.order

