#!/bin/sh

BTN_HEIGHT=15
LARGE_WIDTH=256
ROWS=4
COLS=4
H=$[$ROWS*$COLS*$BTN_HEIGHT]

rsvg -w $[$LARGE_WIDTH*$COLS] -h $[$LARGE_WIDTH*$ROWS]  fitz.svg fitz.png
pngtopnm -alpha fitz.png |pnmsmooth -size 11 11 |pamdice -w $LARGE_WIDTH -h $LARGE_WIDTH -out dice

for i in dice_0* 
	do pnmshear 30 $i >shear_$i
done

echo -e "/* XPM */\nstatic const char * const buttons_xpm[] = {" > ../src/buttons.xpm
pnmcat -tb shear_dice_0* |pnmscale -h $H |pnmpad -w $[2*$BTN_HEIGHT] |pnminvert |ppmtoxpm |grep -v 'static char '>>../src/buttons.xpm
rm shear_dice_* dice_* fitz.png
