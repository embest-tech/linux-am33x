#!/bin/sh

export PATH=/home/chengpg/bin/arm/gcc-linaro-arm-linux-gnueabihf-4.9-2014.09_linux/bin:$PATH
export ARCH=arm
export CROSS_COMPILE=arm-linux-

DESTDIR="../../Image/ /media/sf_work/"
SRCDTB="arch/arm/boot/dts/embest-SBC-SBC8600.dtb"

if ! [ -f ".config" ]; then
    make embest_ti_sbc8600_defconfig
    [ $? != 0 ] && exit 1
fi

make $(basename $SRCDTB) zImage -j8
[ $? != 0 ] && exit 1

for d in $DESTDIR; do
    ! [ -d "$d" ] && continue
    echo "Info: COPY ${SRCDTB} -> ${d}/embest-SBC-SBC8600.dtb"
    cp -f ${SRCDTB} ${d}/embest-SBC-SBC8600.dtb
    echo "Info: COPY arch/arm/boot/zImage ->  ${d}"
    cp -f arch/arm/boot/zImage ${d}
done
