# Information for Developers

Oh, so you want to compile that from source.
Or even do some changes and see what will it do?
Thats all here.

Actually nope, documentation will be at Wiki

## Setting up workspace
1. First of all, you need [PlatfromIO](https://platformio.org/platformio-ide) and IDE that supports it.
2. Clone this repository
```
git clone https://github.com/Nergon123/Okabe_Phone
```
3. Open folder of this repository with your IDE or go to PIO Home and press "Open Project" then choose this folder.
4. PlatformIO should automatically set up everything else.

## Compile Source
If you did everything right `Build` and `Upload` buttons should appear somewhere in workspace.
Build should only compile project.
Upload will build and upload firmware to device.

## IMAGES.SG

Basically IMAGES.SG is just one big RGB565 array.
Adresses of every image there located in [ADRESSES.MD](./ADRESSES.MD)

Editor located in [TOOLS](./Tools/README.MD)