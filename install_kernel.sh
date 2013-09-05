#!/bin/bash

modprobe -r loop

sleep 1

modprobe loop max_part=15

sleep 1

losetup -f /home/maisvendoo/PhantomEx/hdd/hdd.img

sleep 1

mount /dev/loop0p1 /mnt

sleep 1

rm /mnt/boot/kernel
rm /mnt/boot/initrd.img

#/home/maisvendoo/PhantomEx/bin/make_initrd /home/maisvendoo/PhantomEx/bin/hello hello 
#/home/maisvendoo/PhantomEx/bin/intro intro

cp /home/maisvendoo/PhantomEx/bin/kernel /mnt/boot/kernel
cp /home/maisvendoo/PhantomEx/bin/initrd.img /mnt/boot/initrd.img

umount /mnt

sleep 1

losetup -d /dev/loop0


