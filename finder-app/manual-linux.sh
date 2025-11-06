#!/bin/bash
# Script outline to install and build kernel.
# Author: Siddhant Jajoo.
# Student: Ryan Challacombe, Fall 2025

# ./manual-linux.sh /home/ryan/projects/tmp/aeld/
# ./manual-linux.sh /home/ryan/projects/tmp/aeld/ 2>&1 | tee outfile_manual-linux.txt
# 

set -e
set -u

OUTDIR=/tmp/aeld
KERNEL_REPO=git://git.kernel.org/pub/scm/linux/kernel/git/stable/linux-stable.git
KERNEL_VERSION=v5.15.163
BUSYBOX_VERSION=1_33_1
FINDER_APP_DIR=$(realpath $(dirname $0))
ARCH=arm64
CROSS_COMPILE=aarch64-none-linux-gnu-
#SYSROOT_CROSS_COMPILER=/home/ryan/projects/aarch64_toolchain_install_dir/install/arm-gnu-toolchain-13.3.rel1-x86_64-aarch64-none-linux-gnu/bin/../aarch64-none-linux-gnu/libc
SYSROOT_CROSS_COMPILER=/home/ryan/projects/aarch64_toolchain_install_dir/install/arm-gnu-toolchain-13.3.rel1-x86_64-aarch64-none-linux-gnu/aarch64-none-linux-gnu/libc
CONFIG_FILE_LOC=/home/ryan/projects/assignment-1-ryanchallacombe/finder-app/a3p2_kernel_config
FILE_LOC_1=/tmp
FNAME_1=deleteme.txt
FNAME_2=lib/ld-linux-aarch64.so.1


# add path to my cross compiler
export PATH=$PATH:/home/ryan/projects/aarch64_toolchain_install_dir/install/arm-gnu-toolchain-13.3.rel1-x86_64-aarch64-none-linux-gnu/bin
export PATH=$PATH:/home/ryan/projects/aarch64_toolchain_install_dir/install/arm-gnu-toolchain-13.3.rel1-x86_64-aarch64-none-linux-gnu/aarch64-none-linux-gnu/libc

#############################
if [ -f "${FILE_LOC_1}/${FNAME_1}" ]; then
    echo "**** File ${FILE_LOC_1}/${FNAME_1} was found"
    #echo "EXITING WITH 0"
    #exit 0
else
    echo "**** UNABLE TO FIND .${FILE_LOC_1}/${FNAME_1}"
    #echo "EXITING WITH 1"
    #exit 1
fi

if [ -f "${SYSROOT_CROSS_COMPILER}/${FNAME_2}" ]; then
    echo "**** File ${SYSROOT_CROSS_COMPILER}/${FNAME_2} was found"
    echo "EXITING WITH 0"
    exit 0
else
    echo "**** UNABLE TO FIND ${SYSROOT_CROSS_COMPILER}/${FNAME_2}"
    echo "EXITING WITH 1"
    exit 1
fi

#############################

if [ $# -lt 1 ]
then
	echo "Using default directory ${OUTDIR} for output"    
else
	OUTDIR=$1
	echo "Using passed directory ${OUTDIR} for output"
fi

mkdir -p ${OUTDIR}

cd "$OUTDIR"
if [ ! -d "${OUTDIR}/linux-stable" ]; then
    #Clone only if the repository does not exist.
	echo "CLONING GIT LINUX STABLE VERSION ${KERNEL_VERSION} IN ${OUTDIR}"
	git clone ${KERNEL_REPO} --depth 1 --single-branch --branch ${KERNEL_VERSION}
fi
if [ ! -e ${OUTDIR}/linux-stable/arch/${ARCH}/boot/Image ]; then
    cd linux-stable
    echo "Checking out version ${KERNEL_VERSION}"
    git checkout ${KERNEL_VERSION}

    #export PATH=$PATH:/home/ryan/projects/aarch64_toolchain_install_dir/install/arm-gnu-toolchain-13.3.rel1-x86_64-aarch64-none-linux-gnu/bin

    #echo "RUNNING MAKE CONFIG"
    #make menuconfig

    #if [ -f "${CONFIG_FILE_LOC}/.config" ]; then
        # copy the .config file to appropriate location
    #    echo "**** COPYING A STATIC .config FILE TO ${OUTDIR}/linux-stable"
    #    cp ${CONFIG_FILE_LOC}/.config ${OUTDIR}/linux-stable
    #else
    #    echo "**** UNABLE TO FIND .config FILE AT ${CONFIG_FILE_LOC}"
    #    echo "RUNNING MAKE CONFIG"
    #    make menuconfig
    #fi


    echo "RUNNING CLEAN STEP"
    make ARCH=${ARCH} CROSS_COMPILE=${CROSS_COMPILE} mrproper
    #make ${ARCH} ${CROSS_COMPILE}mrproper
    
    echo "RUNNING DEFCONFIG STEP"
    make ARCH=${ARCH} CROSS_COMPILE=${CROSS_COMPILE} defconfig
    
    # This step takes ~ 50 minutes!
    echo "RUNNING vmlinux BUILD STEP (builds kernel image)"
    make -j4 ARCH=${ARCH} CROSS_COMPILE=${CROSS_COMPILE} all

    #echo "RUNNING MODULE BUILD STEP"
    #make ARCH=${ARCH} CROSS_COMPILE=${CROSS_COMPILE} modules

    echo "RUNNING DEVICE TREE BUILD STEP"
    make ARCH=${ARCH} CROSS_COMPILE=${CROSS_COMPILE} dtbs

    echo "***** KERNEL BUILD COMPLETED ******"

fi

echo "Adding the Image in outdir"
cp ${OUTDIR}/linux-stable/arch/${ARCH}/boot/Image ${OUTDIR}
# cp /home/ryan/projects/tmp/aeld/linux-stable/arch/arm64/boot/Image /home/ryan/projects/tmp/aeld/

echo "Creating the staging directory for the root filesystem"
cd "$OUTDIR"
if [ -d "${OUTDIR}/rootfs" ]
then
	echo "Deleting rootfs directory at ${OUTDIR}/rootfs and starting over"
    sudo rm  -rf ${OUTDIR}/rootfs
fi

# Make directories
echo "********* Printing working directory ********* "
echo $(pwd)
echo "********* ls ********* "
echo $(ls)
echo "********* Making rootfs ********* "
mkdir ${OUTDIR}/rootfs 
cd ${OUTDIR}/rootfs

echo "********* Printing working directory ********* "
echo $(pwd)
echo "********* ls ********* "
echo $(ls)
mkdir -p bin dev etc home lib lib64 proc sbin sys tmp usr var
mkdir -p usr/bin usr/lib usr/sbin var/log home/conf
echo "********* Printing working directory ********* "
echo $(pwd)
echo "********* ls ********* "
echo $(ls)

# install busybox
cd "$OUTDIR"
if [ ! -d "${OUTDIR}/busybox" ]
then
git clone git://busybox.net/busybox.git
    cd busybox
    git checkout ${BUSYBOX_VERSION}
    # TODO:  Configure busybox
else
    cd busybox
fi

echo "********* Printing working directory ********* "
echo $(pwd)
echo "********* ls ********* "
echo $(ls)

# Make and install busybox
echo "********* Making and installing busybox ********* "
make distclean
make defconfig
make ARCH=${ARCH} CROSS_COMPILE=${CROSS_COMPILE}
make CONFIG_PREFIX=${OUTDIR}/rootfs ARCH=${ARCH} CROSS_COMPILE=${CROSS_COMPILE} install
    
echo "********* Printing working directory ********* "
echo $(pwd)
echo "********* ls ********* "
echo $(ls)

# move back to root from rootfs/busybox
cd ${OUTDIR}/rootfs
echo "********* Printing working directory ********* "
echo $(pwd)
echo "********* ls ********* "
echo $(ls)


echo "Library dependencies"
${CROSS_COMPILE}readelf -a bin/busybox | grep "program interpreter"
${CROSS_COMPILE}readelf -a bin/busybox | grep "Shared library"


# Add library dependencies to rootfs
# simply copy them to the apprpriate locations
# note: assumes a static cross compiler location
cp ${SYSROOT_CROSS_COMPILER}/lib/ld-linux-aarch64.so.1 ${OUTDIR}/rootfs/lib
# /home/ryan/projects/aarch64_toolchain_install_dir/install/arm-gnu-toolchain-13.3.rel1-x86_64-aarch64-none-linux-gnu/aarch64-none-linux-gnu/libc
# /home/ryan/projects/aarch64_toolchain_install_dir/install/arm-gnu-toolchain-13.3.rel1-x86_64-aarch64-none-linux-gnu/bin/../aarch64-none-linux-gnu/libc/lib/
cp ${SYSROOT_CROSS_COMPILER}/lib64/libm.so.6 ${OUTDIR}/rootfs/lib64
cp ${SYSROOT_CROSS_COMPILER}/lib64/libresolv.so.2 ${OUTDIR}/rootfs/lib64
cp ${SYSROOT_CROSS_COMPILER}/lib64/libc.so.6 ${OUTDIR}/rootfs/lib64

# Make device nodes
sudo mknod -m 666 ${OUTDIR}/rootfs/dev/null c 1 3 
sudo mknod -m 600 ${OUTDIR}/rootfs/dev/console c 5 1

# Clean and build the writer utility
make clean -C ${FINDER_APP_DIR}
make -C ${FINDER_APP_DIR} writer CROSS_COMPILE=${CROSS_COMPILE}


# Copy the finder related scripts and executables to the /home directory
# on the target rootfs
# Copy your finder.sh, conf/username.txt, conf/assignment.txt and finder-test.sh scripts from Assignment 2 into the outdir/rootfs/home directory
cd ${FINDER_APP_DIR}
cp writer finder.sh finder-test.sh ${OUTDIR}/rootfs/home
cp writer conf/username.txt conf/assignment.txt ${OUTDIR}/rootfs/home/conf/
cp autorun-qemu.sh ${OUTDIR}/rootfs/home


# Chown the root directory
cd ${OUTDIR}/rootfs
sudo chown -R root:root *

# Create initramfs.cpio.gz
find . | cpio -H newc -ov --owner root:root > ${OUTDIR}/initramfs.cpio
gzip -f ${OUTDIR}/initramfs.cpio
