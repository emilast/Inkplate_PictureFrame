# Fonts

The very nice font [Acari Sans](https://github.com/StefanPeev/acari-sans) by Stefan Peev is used in the project. The font files are converted to a format that the Adafruit GFX library can use.

To convert the font files, you need to have the `fontconvert` tool from the [Adafruit GFX library](https://github.com/adafruit/Adafruit-GFX-Library) (a precompiled version is in the `fontconvert` directory).

Depending your system, you may need to adapt the below script to your needs. This works on my Windows machine with Windows Subsystem for Linux (WSL) with Ubuntu 20.04 installed.

```shell
cd /mnt/c/code/Adafruit-GFX-Library/fontconvert

./fontconvert /mnt/c/code/Inkplate_PictureFrame/src/Fonts/AcariSans/AcariSans-Bold.ttf 12 44 253 > /mnt/c/code/Inkplate_PictureFrame/src/Fonts/AcariSansbd12.h
./fontconvert /mnt/c/code/Inkplate_PictureFrame/src/Fonts/AcariSans/AcariSans-Regular.ttf 12 44 253 > /mnt/c/code/Inkplate_PictureFrame/src/Fonts/AcariSans12.h
```
