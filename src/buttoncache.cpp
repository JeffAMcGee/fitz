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

//kde
#include <kimageeffect.h>
#include <kdebug.h>
#include <kdecoration.h>

//fitz
#include "buttoncache.h"
#include "fitz.h"

namespace Fitz {

/*The order of this enum must match BtnType.  The numbers come from buttons.xpm*/
int btnsLookup[][4] = {
	{4}, //HELP
	{1,3,7,2}, //MAX
	{0}, //MIN
	{9}, //CLOSE
	{8}, //MENU
	{5,6}, //STICKY
	{10,1}, //SHADE
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

	QColor fg=KDecoration::options()->color(KDecoration::ColorFont);
	QColor bg=KDecoration::options()->color(KDecoration::ColorTitleBar);
	
	kdDebug() <<"ButtonCache::makePixmaps(...)"<<endl;
	
	#include "buttons.xpm"
	QImage greyBtns(buttons_xpm);//load the image from #include
	KImageEffect::flatten(greyBtns,fg,bg);//change the colors
	btnsPic.convertFromImage(greyBtns);//convert to pixmap

	//split to pixmaps
	for(int i=0;i<IMG_COUNT;i++) {
		btns[i].resize(BTN_WIDTH,BTN_HEIGHT);
		bitBlt(
			&(btns[i]), 0,0,
			&btnsPic, 0,i*BTN_HEIGHT, BTN_WIDTH,BTN_HEIGHT
		      );
	}
	pixmapsMade=true;
}

const QPixmap* ButtonCache::getPixmap(BtnType::Type type, int state) {
	unless(pixmapsMade) makePixmaps();
	int i = btnsLookup[type][state];
	return &(btns[i]);
}

};
