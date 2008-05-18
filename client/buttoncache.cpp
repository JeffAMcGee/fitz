//////////////////////////////////////////////////////////////////////////////
// buttoncache.cpp
// -------------------
// Fitz window decoration for KDE
// -------------------
// Copyright (c) 2003-2005 by Jeffrey McGee <JeffAMcGee@gmail.com>
// Portions Copyright (c) 2003 by David Johnson <david@usermode.org>
// 
// You can Freely distribute this program under the GNU General Public
// License. See the file "COPYING" for the exact licensing terms.
//////////////////////////////////////////////////////////////////////////////

//qt
#include <qimage.h>
//Added by qt3to4:
#include <QPixmap>

//kde
#include <kdebug.h>
#include <kdecoration.h>

//fitz
#include "buttoncache.h"
#include "fitz.h"


namespace Fitz {

QImage& flatten(QImage &img, const QColor &ca, const QColor &cb);

/*The order of this enum must match BtnType.  The numbers come from buttons.xpm*/
int btnsLookup[][4] = {
	{4}, //HELP
	{1,3,7,2}, //MAX
	{0}, //MIN
	{9}, //CLOSE
	{8}, //MENU
	{5,6}, //STICKY
	{10,10}, //SHADE
	{14,11}, //ABOVE
	{13,15}, //BELOW
	{12}, //RESIZE
};

//this is not used for anything
char btnsLabel[][9] = {
	"Help", "Maximize", "Minimize", "Close", "Menu", "Sticky", "Shade",
	"Above", "Below", "Resize"
};

ButtonCache::ButtonCache() : pixmapsMade(false) {
}

ButtonCache::~ButtonCache() {
}

void ButtonCache::makePixmaps(){
	QPixmap btnsPic;
	kDebug() <<endl;

	for(int active=0;active<2;active++) {
		QColor fg=KDecoration::options()->color(KDecoration::ColorFont, active);
		QColor bg=KDecoration::options()->color(KDecoration::ColorTitleBar, active);
		
		#include "buttons.xpm"
		QImage greyBtns(buttons_xpm);//load the image from #include
		flatten(greyBtns,fg,bg);//change the colors
		btnsPic.convertFromImage(greyBtns);//convert to pixmap

		//split to pixmaps
		for(int i=0;i<IMG_COUNT;i++) {
			btns[active][i].resize(BTN_WIDTH,BTN_HEIGHT);
			bitBlt(
				&(btns[active][i]), 0,0,
				&btnsPic, 0,i*BTN_HEIGHT, BTN_WIDTH,BTN_HEIGHT
			);
		}
	}
	pixmapsMade=true;
}

const QPixmap* ButtonCache::getPixmap(BtnType::Type type, int state, bool active) {
	unless(pixmapsMade) makePixmaps();
	int i = btnsLookup[type][state];
	return &(btns[active][i]);
}

/* This file is part of the KDE libraries
    Copyright (C) 1998, 1999, 2001, 2002 Daniel M. Duley <mosfet@kde.org>
    (C) 1998, 1999 Christian Tibirna <ctibirna@total.net>
    (C) 1998, 1999 Dirk Mueller <mueller@kde.org>
    (C) 1999 Geert Jansen <g.t.jansen@stud.tue.nl>
    (C) 2000 Josef Weidendorfer <weidendo@in.tum.de>
    (C) 2004 Zack Rusin <zack@kde.org>

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions
are met:

1. Redistributions of source code must retain the above copyright
   notice, this list of conditions and the following disclaimer.
2. Redistributions in binary form must reproduce the above copyright
   notice, this list of conditions and the following disclaimer in the
   documentation and/or other materials provided with the distribution.

THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR
IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT,
INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT
NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
(INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

*/


QImage& flatten(QImage &img, const QColor &ca,
							const QColor &cb)
{
	int r1 = ca.red(); int r2 = cb.red();
	int g1 = ca.green(); int g2 = cb.green();
	int b1 = ca.blue(); int b2 = cb.blue();
	int min = 0, max = 255;

	QRgb col;

	// Get minimum and maximum greylevel.
	if (img.numColors()) {
		// pseudocolor
		for (int i = 0; i < img.numColors(); i++) {
			col = img.color(i);
			int mean = (qRed(col) + qGreen(col) + qBlue(col)) / 3;
			min = QMIN(min, mean);
			max = QMAX(max, mean);
		}
	} else {
		// truecolor
		for (int y=0; y < img.height(); y++)
			for (int x=0; x < img.width(); x++) {
				col = img.pixel(x, y);
				int mean = (qRed(col) + qGreen(col) + qBlue(col)) / 3;
				min = QMIN(min, mean);
				max = QMAX(max, mean);
			}
	}

	// Conversion factors
	float sr = ((float) r2 - r1) / (max - min);
	float sg = ((float) g2 - g1) / (max - min);
	float sb = ((float) b2 - b1) / (max - min);


	// Repaint the image
	if (img.numColors()) {
		for (int i=0; i < img.numColors(); i++) {
			col = img.color(i);
			int mean = (qRed(col) + qGreen(col) + qBlue(col)) / 3;
			int r = (int) (sr * (mean - min) + r1 + 0.5);
			int g = (int) (sg * (mean - min) + g1 + 0.5);
			int b = (int) (sb * (mean - min) + b1 + 0.5);
			img.setColor(i, qRgba(r, g, b, qAlpha(col)));
		}
	} else {
		for (int y=0; y < img.height(); y++)
			for (int x=0; x < img.width(); x++) {
				col = img.pixel(x, y);
				int mean = (qRed(col) + qGreen(col) + qBlue(col)) / 3;
				int r = (int) (sr * (mean - min) + r1 + 0.5);
				int g = (int) (sg * (mean - min) + g1 + 0.5);
				int b = (int) (sb * (mean - min) + b1 + 0.5);
				img.setPixel(x, y, qRgba(r, g, b, qAlpha(col)));
			}
	}

	return img;
}

};
