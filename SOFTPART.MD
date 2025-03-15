# Software Part

So you just assembled device?

What about assembling a project?
In this "Chapter" 
I will tell you how to upload existing Firmware




## Easy way

First of all we need zip file from releases

We will find there 

- bin files (Firmware itself)
- esptool
- script (bat or sh)
- FIRMWARE folder (for sdcard)
Unpack it and launch script

Don't forget `chmod +x FLASH.sh` for linux users

If you'll open script it should automatically flash device.

## MicroSD card
MicroSD card should be formatted into FAT (FAT16 or FAT32)

folder "`FIRMWARE`" with "`IMAGES.SG`" should be copied to root directory of MicroSD card.

## Alternative way

See [DEVINFO.MD](./DEVINFO.MD)