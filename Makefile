.PHONY: test

namem = proc_mod
namef = /proc/anton/ant_proc_file

obj-m += $(namem).o

all:
		make -C /lib/modules/$(shell uname -r)/build M=$(PWD) modules

cli:
		make -C /lib/modules/$(shell uname -r)/build M=$(PWD) clean

rm:
		rmmod -f $(namem).ko

clean: rm cli

test:
		insmod $(namem).ko
		echo rofl > $(namef)
		cat $(namef)
		echo kekichya > $(namef)
		cat $(namef)
		echo end > $(namef)
		cat $(namef)