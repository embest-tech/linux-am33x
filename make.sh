#!/bin/sh

export PATH=/home/chengpg/bin/arm/gcc-linaro-4.9.4-2017.01-i686_arm-linux-gnueabihf/bin:$PATH
export ARCH=arm
export CROSS_COMPILE=arm-linux-

DESTDIR="../../Image/ /home/chengpg/tftp/"

build_weidian() {
	SRCDTB="arch/arm/boot/dts/embest-SBC-SBC8600-weidian-MMC.dtb"
	DSTDTB="embest-SBC-SBC8600.dtb"
	SRCDTB1="arch/arm/boot/dts/embest-SBC-SBC8600-weidian-NAND.dtb"
	DSTDTB1="embest-SBC-SBC8600-NAND.dtb"

	if ! [ -f ".config" ]; then
		# make omap2plus_defconfig
		make embest_ti_weidian_defconfig
		[ $? != 0 ] && exit 1
	fi
	make $(basename $SRCDTB) $(basename $SRCDTB1)  zImage -j12
	[ $? != 0 ] && exit 1
	for d in $DESTDIR; do
		! [ -d "$d" ] && continue
		echo "Info: COPY ${SRCDTB} -> ${d}/${DSTDTB}"
		cp -f ${SRCDTB} ${d}/${DSTDTB}
		echo "Info: COPY ${SRCDTB1} -> ${d}/${DSTDTB1}"
		cp -f ${SRCDTB1} ${d}/${DSTDTB1}
		echo "Info: COPY arch/arm/boot/zImage ->  ${d}"
		cp -f arch/arm/boot/zImage ${d}
	done
}

build_weidian
