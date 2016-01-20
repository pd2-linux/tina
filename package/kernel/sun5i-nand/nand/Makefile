
################################################################################
#
# Common Variables that already set:
#     LICHEE_KDIR
#     LICHEE_MOD_DIR
#     CROSS_COMPILE
#     ARCH
#
#################################################################################
PWD=$(shell pwd)
obj-m+=nand.o
#nand-objs := nand_interface.o  \
#             nfd/nand_blk.o nfd/nand_osal_for_linux.o nfd/nand_ota_burn.o nfd/boot_head/boot0_op.o libnand.a
#    cp libnand libnand.a
nand-objs := nand_interface.o nfd/nand_osal_for_linux.o nfd/nand_ota_burn.o nfd/boot_head/boot0_op.o nfd/nand_base.o nfd/nand_blk.o nfd/nand_dev.o nfd/nand_class.o libnand.a

#install: build
#	cp nand.ko $(LICHEE_MOD_DIR)/

#build:
#	@echo $(LICHEE_KDIR)
#	cp libnand libnand.a
#	$(MAKE) -C $(LICHEE_KDIR) M=$(PWD)


#clean:
#	@rm -rf *.o *.ko .*.cmd *.mod.c *.order *.symvers .tmp_versions *~
