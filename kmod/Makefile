obj-m += ./kmod.o

PWD=$(shell pwd)

all: clean build remove install

build:
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) modules

clean:
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) clean

install: build
	sudo insmod ./kmod.ko

remove:
	sudo rmmod kmod

