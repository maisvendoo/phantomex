rm ~/PhantomEx/hdd/hdd.vmdk
rm ~/PhantomEx/hdd/hdd.vdi
qemu-img convert -f raw -O vmdk ~/PhantomEx/hdd/hdd.img ~/PhantomEx/hdd/hdd.vmdk
qemu-img convert -f raw -O vdi ~/PhantomEx/hdd/hdd.img ~/PhantomEx/hdd/hdd.vdi


