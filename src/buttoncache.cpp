//////////////////////////////////////////////////////////////////////////////
// buttoncache.cpp
// -------------------
// Fitz window decoration for KDE
// -------------------
// Copyright (c) 2003-2005 by Jeffrey McGee <jeffreym@cs.tamu.edu>
// Portions Copyright (c) 2003 by David Johnson <david@usermode.org>
// 
// You can Freely distribute this program under the GNU General Public
// License. See the file "COPYING" for the exact licensing terms.
//////////////////////////////////////////////////////////////////////////////

//qt
#include <qimage.h>

//kde
#include <kimageeffect.h>
#include <kdebug.h>
#include <kdecoration.h>

//fitz
#include "buttoncache.h"
#include "fitz.h"

namespace Fitz {

ButtonCache::ButtonCache() : pixmapsMade(false) {
}

ButtonCache::~ButtonCache() {
}

void ButtonCache::makePixmaps(){
	QPixmap btnsPic;

	QColor fg=KDecoration::options()->color(KDecoration::ColorFont);
	QColor bg=KDecoration::options()->color(KDecoration::ColorTitleBar);
	
	kdDebug() <<"ButtonCache::makePixmaps(...)"<<endl;
	
	#include "buttons.xpm"
	QImage greyBtns(buttons_xpm);//load the image from #include
	KImageEffect::flatten(greyBtns,fg,bg);//change the colors
	btnsPic.convertFromImage(greyBtns);//convert to pixmap

	//split to pixmaps
	for(int i=0;i<BtnImg::COUNT;i++) {
		btns[i].resize(BTN_WIDTH,BTN_HEIGHT);
		bitBlt(
			&(btns[i]), 0,0,
			&btnsPic, 0,i*BTN_HEIGHT, BTN_WIDTH,BTN_HEIGHT
		      );
	}
	pixmapsMade=true;
}

const QPixmap* ButtonCache::getPixmap(BtnImg::Img i) {
	unless(pixmapsMade) makePixmaps();
	return &(btns[i]);
}

};
