#!/bin/bash
# Script outline to install and build kernel.
# Author: Siddhant Jajoo.

set -e
set -u

OUTDIR=/tmp/aeld
KERNEL_REPO=git://git.kernel.org/pub/scm/linux/kernel/git/stable/linux-stable.git
KERNEL_VERSION=v5.1.10
BUSYBOX_VERSION=1_33_1
FINDER_APP_DIR=$(realpath $(dirname $0))
ARCH=arm64
CROSS_COMPILE=aarch64-none-linux-gnu-
SYSROOT=$(aarch64-none-linux-gnu-gcc -print-sysroot)

if [ $# -lt 1 ]
then
	echo "Using default directory $OUTDIR for output"
else
	OUTDIR=$1
	echo "Using passed directory $OUTDIR for output"
fi

mkdir -p "$OUTDIR"

cd "$OUTDIR"
if [ ! -d "$OUTDIR/linux-stable" ]; then
    #Clone only if the repository does not exist.
	echo "CLONING GIT LINUX STABLE VERSION ${KERNEL_VERSION} IN $OUTDIR"
	git clone ${KERNEL_REPO} --depth 1 --single-branch --branch ${KERNEL_VERSION}
fi
if [ ! -e "$OUTDIR"/linux-stable/arch/${ARCH}/boot/Image ]; then
    cd linux-stable
    echo "Checking out version ${KERNEL_VERSION}"
    git checkout ${KERNEL_VERSION}

    # TODO: Add your kernel build steps here
    #BUILD (deep clean): Make specific the architecture and deep clean (e.g., remove .conf, etc.). 
    make ARCH=$ARCH CROSS_COMPILE=$CROSS_COMPILE mrproper
    #Make menuconfig and save.
    sudo cp "$FINDER_APP_DIR"/kconf/.config "$OUTDIR"/linux-stable
    #BUILD (configure virt): Make specific the architecture and configure for virtual arm device board. (QEMU). 
    make ARCH=$ARCH CROSS_COMPILE=$CROSS_COMPILE defconfig
    #BUILD: Kernel image for QEMU. -j4 (4 jobs may run simultaneously)
    #Nice to have, makes building faster, with fix for issue. https://github.com/BPI-SINOVOIP/BPI-M4-bsp/issues/4
    sed -i 's/^YYLTYPE /extern YYLTYPE /g' "$OUTDIR"/linux-stable/scripts/dtc/dtc-lexer.l
    make -j 16 ARCH=$ARCH CROSS_COMPILE=$CROSS_COMPILE
    #BUILD: Kernel image for QEMU. -j4 (& load moduels),with fix for bug. https://github.com/BPI-SINOVOIP/BPI-M4-bsp/issues/4
    #sed -i 's/^YYLTYPE /extern YYLTYPE /g' "$OUTDIR"/linux-stable/scripts/dtc/dtc-lexer.l
    #make -j 8 ARCH=$ARCH CROSS_COMPILE=$CROSS_COMPILE INSTALL_MOD_PATH="$OUTDIR"/rootfs modules_install
    #BUILD (kernel modules): 
    make -j 16 ARCH=$ARCH CROSS_COMPILE=$CROSS_COMPILE modules
    #BUILD (device-tree): 
    make -j 16 ARCH=$ARCH CROSS_COMPILE=$CROSS_COMPILE dtbs
fi

echo "Adding the Image in outdir"
cp "$OUTDIR"/linux-stable/arch/arm64/boot/Image "$OUTDIR"

echo "Creating the staging directory for the root filesystem"
cd "$OUTDIR"
if [ -d ""$OUTDIR"/rootfs" ]
then
	echo "Deleting rootfs directory at $OUTDIR/rootfs and starting over"
    sudo rm -rf "$OUTDIR"/rootfs
fi

# TODO: Create necessary base directories
#Make root filesystem. Using absolute paths and “mkdir -p” for efficiency.
mkdir -p "$OUTDIR"/rootfs/bin
mkdir -p "$OUTDIR"/rootfs/dev
mkdir -p "$OUTDIR"/rootfs/etc
mkdir -p "$OUTDIR"/rootfs/conf
mkdir -p "$OUTDIR"/rootfs/home
mkdir -p "$OUTDIR"/rootfs/lib/modules
mkdir -p "$OUTDIR"/rootfs/lib64
mkdir -p "$OUTDIR"/rootfs/proc
mkdir -p "$OUTDIR"/rootfs/sbin
mkdir -p "$OUTDIR"/rootfs/sys
mkdir -p "$OUTDIR"/rootfs/temp
mkdir -p "$OUTDIR"/rootfs/usr/bin
mkdir -p "$OUTDIR"/rootfs/usr/lib
mkdir -p "$OUTDIR"/rootfs/usr/sbin
mkdir -p "$OUTDIR"/rootfs/var/log

#
#CONFIG_PREFIX=$("$OUTDIR"/rootfs)

cd "$OUTDIR"
if [ ! -d "$OUTDIR/busybox" ]
then
git clone git://busybox.net/busybox.git
    cd busybox
    git checkout ${BUSYBOX_VERSION}
    # TODO:  Configure busybox
    make distclean
    #Make menuconfig and save.
    sudo cp "$FINDER_APP_DIR"/bconf/.config "$OUTDIR"/busybox
    make defconfig
else
    cd busybox
fi

# TODO: Make and install busybox
make ARCH=$ARCH CROSS_COMPILE=$CROSS_COMPILE CONFIG_PREFIX="$OUTDIR"/rootfs install

echo "Library dependencies"
${CROSS_COMPILE}readelf -a "$OUTDIR"/rootfs/bin/busybox | grep "program interpreter"
${CROSS_COMPILE}readelf -a "$OUTDIR"/rootfs/bin/busybox | grep "Shared library"

# TODO: Add library dependencies to rootfs
cp -a "$SYSROOT"/lib/ld-linux-aarch64.so.1 "$OUTDIR"/rootfs/lib
cp -a "$SYSROOT"/lib64/ld-2.31.so "$OUTDIR"/rootfs/lib64
#libc
cp -a "$SYSROOT"/lib64/libc.so.6 "$OUTDIR"/rootfs/lib64
cp -a "$SYSROOT"/lib64/libc-2.31.so "$OUTDIR"/rootfs/lib64
#libm
cp -a "$SYSROOT"/lib64/libm.so.6 "$OUTDIR"/rootfs/lib64
cp -a "$SYSROOT"/lib64/libm-2.31.so "$OUTDIR"/rootfs/lib64
#shared
cp -a "$SYSROOT"/lib64/libresolv.so.2 "$OUTDIR"/rootfs/lib64
cp -a "$SYSROOT"/lib64/libresolv-2.31.so "$OUTDIR"/rootfs/lib64

# TODO: Make device nodes
sudo mknod -m 666 "$OUTDIR"/rootfs/dev/null c 1 3
sudo mknod -m 600 "$OUTDIR"/rootfs/dev/console c 5 1 

# TODO: Clean and build the writer utility
cd "$FINDER_APP_DIR"
make clean
make -j 8 ARCH=$ARCH CROSS_COMPILE=$CROSS_COMPILE
cd "$OUTDIR"
# TODO: Copy the finder related scripts and executables to the /home directory
# on the target rootfs
sudo cp -r "$FINDER_APP_DIR"/* "$OUTDIR"/rootfs/home/
sudo cp -r "$FINDER_APP_DIR"/conf/* "$OUTDIR"/rootfs/conf/

# TODO: Chown the root directory
cd "$OUTDIR"/rootfs/
sudo chown -R root:root *

# TODO: Create initramfs.cpio.gz
cd "$OUTDIR"/rootfs
find . | cpio -H newc -ov --owner root:root > ../initramfs.cpio
cd ..
gzip initramfs.cpio
sudo chown -R root:root initramfs.cpio.gz