#KERN_DIR=/lib/modules/$(shell uname -r)/build
KERN_DIR = /home/rudy/workspace/Projects/Bocon/bocon-linux-imx8

obj-m += uart_power.o

all:
	make -C $(KERN_DIR) M=`pwd` modules

clean:
	make -C $(KERN_DIR) M=`pwd` modules clean
	rm -rf modules.order

