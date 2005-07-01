#!/bin/sh
rsvg -w 1024 -h 1024 fitz.svg fitz.png
pngtopnm -alpha fitz.png |pnmsmooth -size 11 11 |pamdice -w 256 -h 256 -out dice

for i in dice_0* 
	do pnmshear 30 $i >shear_$i
done
pnmcat -tb shear_dice_0* |pnmscale -h 192 |pnmpad -w 24 |pnminvert |ppmtoxpm >../src/buttons.xpm
rm shear_dice_* dice_* fitz.png
echo "rember to add the following line to buttons.xpm"
echo "static const char * const buttons_xpm[] = {"
