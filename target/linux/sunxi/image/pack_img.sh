PACK_CHIP=sun8iw5p1
PACK_PLATFORM=tina
PACK_BOARD=evb
PACK_DEBUG=uart0
PACK_SIG=none
PACK_MODE=none
PACK_DIR=

while getopts "c:p:b:d:r:sh" arg
do
	case $arg in
		c)
			PACK_CHIP=$OPTARG
			;;
		p)
			PACK_PLATFORM=$OPTARG
			;;
		d)
			PACK_DEBUG=$OPTARG
			;;
		s)
			PACK_SIG=sig
			;;
		r)
			PACK_DIR=$OPTARG
			;;
		b)
			PACK_BOARD=$OPTARG
			;;
		?)
			exit 1
			;;
	esac
done

function pack_error()
{
	echo -e "\033[47;31mERROR: $*\033[0m"
}

function pack_warn()
{
	echo -e "\033[47;34mWARN: $*\033[0m"
}

function pack_info()
{
	echo -e "\033[47;30mINFO: $*\033[0m"
}

function uart_switch()
{
	TX=`awk  '$0~"'$PACK_CHIP'"{print $2}' card_debug_pin`
	RX=`awk  '$0~"'$PACK_CHIP'"{print $3}' card_debug_pin`
	PORT=`awk  '$0~"'$PACK_CHIP'"{print $4}' card_debug_pin`
	MS=`awk  '$0~"'$PACK_CHIP'"{print $5}' card_debug_pin`
	CK=`awk  '$0~"'$PACK_CHIP'"{print $6}' card_debug_pin`
	DO=`awk  '$0~"'$PACK_CHIP'"{print $7}' card_debug_pin`
	DI=`awk  '$0~"'$PACK_CHIP'"{print $8}' card_debug_pin`

	sed -i s'/jtag_ms = /jtag_ms = '$MS'/g' awk_debug_card0
	sed -i s'/jtag_ck = /jtag_ck = '$CK'/g' awk_debug_card0
	sed -i s'/jtag_do = /jtag_do = '$DO'/g' awk_debug_card0
	sed -i s'/jtag_di = /jtag_di = '$DI'/g' awk_debug_card0
	sed -i s'/uart_debug_port =/uart_debug_port = '$PORT'/g' awk_debug_card0
	sed -i s'/uart_debug_tx =/uart_debug_tx = '$TX'/g' awk_debug_card0
	sed -i s'/uart_debug_rx =/uart_debug_rx = '$RX'/g' awk_debug_card0
	sed -i s'/uart_tx =/uart_tx = '$TX'/g' awk_debug_card0
	sed -i s'/uart_rx =/uart_rx = '$RX'/g' awk_debug_card0
	awk -f awk_debug_card0 sys_config.fex > a.fex
	rm sys_config.fex
	mv a.fex sys_config.fex
	echo "uart -> card0"
}

function do_prepare()
{
	cd ${PACK_DIR}/bootloader

	sed -i 's/\\boot-resource/\/boot-resource/g' boot-resource.ini
	sed -i 's/\\\\/\//g' image.cfg
	sed -i 's/^imagename/;imagename/g' image.cfg

	if [ "x${PACK_DEBUG}" = "xcard0" -a "x${PACK_MODE}" != "xdump" \
		-a "x${PACK_FUNC}" != "xprvt" ] ; then
		uart_switch
	fi
	IMG_NAME="${PACK_CHIP}_${PACK_PLATFORM}_${PACK_BOARD}_${PACK_DEBUG}"
	if [ "x${PACK_SIG}" != "xnone" ]; then
	IMG_NAME="${IMG_NAME}_${PACK_SIG}"
	fi

	if [ "x${PACK_MODE}" = "xdump" -o "x${PACK_MODE}" = "xota_test" ] ; then
	IMG_NAME="${IMG_NAME}_${PACK_MODE}"
	fi

	if [ "x${PACK_FUNC}" = "xprvt" ] ; then
	IMG_NAME="${IMG_NAME}_${PACK_FUNC}"
	fi

	if [ "x${PACK_FUNC}" = "xprev_refurbish" ] ; then
	IMG_NAME="${IMG_NAME}_${PACK_FUNC}"
	fi

	IMG_NAME="${IMG_NAME}.img"
	echo "imagename = $IMG_NAME" >> image.cfg
	echo "" >> image.cfg
}

function do_common()
{
	cd ${PACK_DIR}/bootloader
	busybox unix2dos sys_config.fex
	busybox unix2dos sys_partition.fex
	script  sys_config.fex > /dev/null
	script  sys_partition.fex > /dev/null
	cp -f   sys_config.bin config.fex

	# Those files for SpiNor. We will try to find sys_partition_nor.fex
	if [ -f sys_partition_nor.fex ];  then

		# Here, will create sys_partition_nor.bin
		busybox unix2dos sys_partition_nor.fex
		script  sys_partition_nor.fex > /dev/null
		update_boot0 boot0_spinor.fex   sys_config.bin SDMMC_CARD > /dev/null
		update_uboot u-boot-spinor.fex  sys_config.bin >/dev/null
	fi

	# Those files for Nand or Card
	update_boot0 boot0_nand.fex	sys_config.bin NAND > /dev/null
	update_boot0 boot0_sdcard.fex	sys_config.bin SDMMC_CARD > /dev/null
	update_uboot u-boot.fex         sys_config.bin > /dev/null
	update_fes1  fes1.fex           sys_config.bin > /dev/null
	fsbuild	     boot-resource.ini  split_xxxx.fex > /dev/null

	u_boot_env_gen env.cfg env.fex > /dev/null
}

function do_pack_linux()
{
	printf "packing for linux\n"

	#ln -s ${BIN_DIR}/vmlinux.tar.bz2 vmlinux.fex
	rm -rf boot.fex
	rm -rf rootfs.fex
        rm -rf usr.fex
	ln -s ${PACK_DIR}/boot.img        boot.fex
	ln -s ${PACK_DIR}/rootfs.img     rootfs.fex
	ln -s ${PACK_DIR}/usr.img     usr.fex

	# Those files is ready for SPINor.
	#ln -s ${BIN_DIR}/uImage          kernel.fex

}

function do_finish()
{
	if [ -f sys_partition_nor.bin ]; then 
		mv -f sys_partition.bin         sys_partition.bin_back
		cp -f sys_partition_nor.bin     sys_partition.bin
		update_mbr                      sys_partition.bin 1 > /dev/null
		BOOT1_FILE=u-boot-spinor.fex
		LOGIC_START=496 #496+16=512K
		merge_full_img --out full_img.fex \
		      --boot0 boot0_spinor.fex \
		      --boot1 ${BOOT1_FILE} \
		      --mbr sunxi_mbr.fex \
		      --logic_start ${LOGIC_START} \
		      --partition sys_partition.bin
		if [ $? -ne 0 ]; then
			pack_error "merge_full_img failed"
		fi

		mv -f sys_partition.bin_back    sys_partition.bin
	fi
	if [ ! -f full_img.fex ]; then
		echo "full_img.fex is empty" > full_img.fex
	fi

	update_mbr          sys_partition.bin 4 > /dev/null
	dragon image.cfg    sys_partition.fex

	if [ -e ${IMG_NAME} ]; then
		mv ${IMG_NAME} ../${IMG_NAME}
		echo '----------image is at----------'
		echo -e '\033[0;31;1m'
		echo ${PACK_DIR}/${IMG_NAME}
		echo -e '\033[0m'
	fi

	printf "pack finish\n"

}

do_prepare
do_common
do_pack_linux
do_finish
