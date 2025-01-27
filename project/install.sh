INS_DEV=$1
ROOT_PART=$2
ROOT_FS=$3
GRUB_PART=$4
GRUB_TARGET=$5
BOOT_TYPE=${GRUB_TARGET##*\-}
echo $@ $BOOT_TYPE
sudo mkfs.$ROOT_FS  "$INS_DEV"p"$ROOT_PART"
sudo mkfs.vfat -F 32 "$INS_DEV"p"$GRUB_PART"
if [[ $BOOT_TYPE == "efi" ]]
then
	sudo mkfs.vfat -F 32 "$INS_DEV"p1
fi
sudo mount "$INS_DEV"p"$GRUB_PART" mnt
if [[ $BOOT_TYPE == "efi" ]]
then
	sudo mkdir mnt/efi
	sudo mount "$INS_DEV"p1 mnt/efi
	sudo grub-install --target=$GRUB_TARGET --boot-directory=./mnt  --modules="part_gpt all_video"   --efi-directory=./mnt/efi  --bootloader-id=GeekToyOS
else
	sudo grub-install --target=$GRUB_TARGET --boot-directory=./mnt  --modules="part_msdos all_video" $INS_DEV
fi
sudo cp project/grub_$BOOT_TYPE.cfg mnt/grub/grub.cfg
if [[ $BOOT_TYPE == "efi" ]]
then
	sudo umount mnt/efi
    sudo rmdir mnt/efi
fi
sudo umount mnt
sudo mount "$INS_DEV"p"$ROOT_PART" mnt 