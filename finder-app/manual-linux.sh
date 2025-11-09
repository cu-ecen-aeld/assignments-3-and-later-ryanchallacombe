#!/bin/bash
# Script outline to install and build kernel.
# Author: Siddhant Jajoo.
# Student: Ryan Challacombe, Fall 2025

# Troubleshooting
#   My docker was running in rootless mode. I needed to switch to default

set -e
set -u

OUTDIR=/tmp/aeld
KERNEL_REPO=git://git.kernel.org/pub/scm/linux/kernel/git/stable/linux-stable.git
KERNEL_VERSION=v5.15.163
BUSYBOX_VERSION=1_33_1
FINDER_APP_DIR=$(realpath $(dirname $0))
ARCH=arm64
CROSS_COMPILE=aarch64-none-linux-gnu-
CONFIG_FILE_LOC=/home/ryan/projects/assignment-1-ryanchallacombe/finder-app/a3p2_kernel_config

# add path to my cross compiler
export PATH=$PATH:/home/ryan/projects/aarch64_toolchain_install_dir/install/arm-gnu-toolchain-13.3.rel1-x86_64-aarch64-none-linux-gnu/bin
#export PATH=$PATH:/home/ryan/projects/aarch64_toolchain_install_dir/install/arm-gnu-toolchain-13.3.rel1-x86_64-aarch64-none-linux-gnu/aarch64-none-linux-gnu/libc

echo "Running script as $(whoami)"
echo "Starting directory $(pwd)"

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
    
    echo "RUNNING DEFCONFIG STEP"
    make ARCH=${ARCH} CROSS_COMPILE=${CROSS_COMPILE} defconfig
    
    # This step takes ~ 50 minutes!
    echo "RUNNING vmlinux BUILD STEP (builds kernel image)"
    make -j4 ARCH=${ARCH} CROSS_COMPILE=${CROSS_COMPILE} all

    echo "RUNNING MODULE BUILD STEP"
    make ARCH=${ARCH} CROSS_COMPILE=${CROSS_COMPILE} modules

    echo "RUNNING DEVICE TREE BUILD STEP"
    make ARCH=${ARCH} CROSS_COMPILE=${CROSS_COMPILE} dtbs

    echo "***** KERNEL BUILD COMPLETED ******"
fi

echo "Adding the Image in outdir"
cp ${OUTDIR}/linux-stable/arch/${ARCH}/boot/Image ${OUTDIR}

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
else
    #remove busybox dir
    sudo rm -rf ${OUTDIR}/busybox
    git clone git://busybox.net/busybox.git
    cd busybox
    git checkout ${BUSYBOX_VERSION}
fi

# Configure busybox
make distclean
make defconfig

# Make and install busybox
echo "********* Making and installing busybox ********* "
make ARCH=${ARCH} CROSS_COMPILE=${CROSS_COMPILE}
make CONFIG_PREFIX=${OUTDIR}/rootfs ARCH=${ARCH} CROSS_COMPILE=${CROSS_COMPILE} install

# move back to root from rootfs/busybox
cd ${OUTDIR}/rootfs

echo "Library dependencies"
${CROSS_COMPILE}readelf -a bin/busybox | grep "program interpreter"
${CROSS_COMPILE}readelf -a bin/busybox | grep "Shared library"

echo "Moving to starting directory ${FINDER_APP_DIR}"
cd ${FINDER_APP_DIR}

# Add library dependencies to rootfs
# simply copy them to the apprpriate locations
# note: assumes a static cross compiler location
SYSROOT_CROSS_COMPILER=$(${CROSS_COMPILE}gcc -print-sysroot)
cp ${SYSROOT_CROSS_COMPILER}/lib/ld-linux-aarch64.so.1 ${OUTDIR}/rootfs/lib
cp ${SYSROOT_CROSS_COMPILER}/lib64/libm.so.6 ${OUTDIR}/rootfs/lib64
cp ${SYSROOT_CROSS_COMPILER}/lib64/libresolv.so.2 ${OUTDIR}/rootfs/lib64
cp ${SYSROOT_CROSS_COMPILER}/lib64/libc.so.6 ${OUTDIR}/rootfs/lib64

echo "Moving back to ${OUTDIR}/rootfs"
cd ${OUTDIR}/rootfs

# Make device nodes
echo "******** Making device nodes ..."
sudo mknod -m 666 dev/null c 1 3 
sudo mknod -m 600 dev/console c 5 1

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