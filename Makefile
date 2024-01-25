obj-m += networkfs.o
networkfs-objs += entrypoint.o
ccflags-y := -std=gnu11 -Wno-declaration-after-statement

all:
	make -C /lib/modules/$(shell uname -r)/build M=$(shell pwd) modules

remount:
	sudo make clean
	sudo make
	sudo rmmod networkfs.ko
	sudo insmod networkfs.ko
	sudo umount /mnt/ct
	sudo mount -t networkfs 8c6a65c8-5ca6-49d7-a33d-daec00267011 /mnt/ct

test:
	sudo make remount
	ls -ail /mnt/ct

clean:
	make -C /lib/modules/$(shell uname -r)/build M=$(shell pwd) clean
