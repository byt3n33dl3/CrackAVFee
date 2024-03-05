    Check Connected Devices:
        Use lsusb to list USB devices connected to your system.
        Use dmesg | tail right after connecting the device to see kernel messages, which might indicate the device recognition and mounting process.

Step 2: Mount the Device (If Not Automatically Mounted)

If the device doesn't mount automatically, you'll need to manually mount it.

    Find the Device:
        Use lsblk or fdisk -l to list all block devices, including USB drives.

    Mount the Device:
        Create a mount point: mkdir /mnt/usb
        Mount the device: sudo mount /dev/sdX1 /mnt/usb (Replace /dev/sdX1 with the correct device identifier found in the previous step.)

Step 3: Transfer Files

Once the device is mounted, you can transfer files using file operation commands.

    Copy the File:
        Use cp to copy the file from the mounted directory to your desired location: cp /mnt/usb/path/to/file /your/destination/path

Step 4: Unmount the Device

After transferring the files, it's important to safely unmount the USB device.

    Unmount the Device:
        Use sudo umount /mnt/usb