#KERN_DIR=/lib/modules/$(shell uname -r)/build
KERN_DIR = /home/rudy/workspace/Projects/Bocon/bocon-linux-imx8
CFLAGS = 

obj-m += aidodi.o
obj-m += mcu_peripheral.o
obj-m += spi-protocol.o
obj-m += wk2xxx_spi.o

all:
	make -C $(KERN_DIR) M=`pwd` modules

clean:
	make -C $(KERN_DIR) M=`pwd` modules clean
	rm -rf modules.order
