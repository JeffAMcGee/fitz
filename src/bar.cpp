//////////////////////////////////////////////////////////////////////////////
// Bar.cpp
// -------------------
// Fitz window decoration for KDE
// -------------------
// Copyright (c) 2003-2005 by Jeffrey McGee <jeffreym@cs.tamu.edu>
// Portions Copyright (c) 2003 by David Johnson <david@usermode.org>
// 
// You can Freely distribute this program under the GNU General Public
// License. See the file "COPYING" for the exact licensing terms.
//////////////////////////////////////////////////////////////////////////////

//kde
#include <klocale.h>
#include <kdecoration.h>
#include <kdecorationfactory.h>
#include <kdebug.h>

//qt
#include <qlabel.h>
#include <qlayout.h>
#include <qbitmap.h>
#include <qpainter.h>
#include <qtooltip.h>

//fitz
#include "bar.h"
#include "button.h"

namespace Fitz {

// Constructor
Bar::Bar(KDecoration *parent, const char *name, bool tl)
		: QWidget (parent->widget(), name, tl?(WType_TopLevel | WX11BypassWM):0),
		client(parent), oldParent(), toplevel(tl)
{
	// for flicker-free redraws
	if(toplevel)
		setBackgroundMode(NoBackground);

	setSizePolicy(QSizePolicy(QSizePolicy::Fixed,QSizePolicy::Fixed));
	
	box = new QBoxLayout ( this, QBoxLayout::LeftToRight, 0, 0, "Fitz::Bar Layout");

	// setup titlebar buttons
	for (int n=0; n<BtnType::COUNT; n++) {
		button[n] = 0;
	}
}

Bar::~Bar() {
	for (int n=0; n<BtnType::COUNT; n++) {
		if (button[n]) delete button[n];
	}
}

// Add buttons to title layout
void Bar::addButtons(const QString& s) {
	btnswidth=0;
	box->addSpacing(SLANT_WIDTH);
	for (unsigned i=0; i < s.length(); i++) {
		switch (s[i]) {
		  case 'M': // Menu button
			addButton(BtnType::MENU, "Menu", BtnImg::MENU,
					SIGNAL(pressed()), this, SLOT(menuButtonPressed()));
			break;

		  case 'S': // Sticky button
			  addButton(BtnType::STICKY,"Sticky",
				    client->isOnAllDesktops() ? BtnImg::STICK:BtnImg::UNSTICK,
					SIGNAL(clicked()), client, SLOT(toggleOnAllDesktops()));
			break;

		  case 'H': // Help button
			if(client->providesContextHelp())
				addButton(BtnType::HELP, "Help", BtnImg::HELP,
						SIGNAL(clicked()), client, SLOT(showContextHelp()));
			break;

		  case 'I': // Minimize button
			addButton(BtnType::MIN, "Minimize", BtnImg::MIN,
					SIGNAL(clicked()), client, SLOT(minimize()));
			break;

		  case 'A': // Maximize button
			BtnImg::Img i;
			if(client->maximizeMode()==KDecorationDefines::MaximizeFull)
				i=BtnImg::RESTORE;
			else
				i=BtnImg::MAX;

			addButton(BtnType::MAX, "Maximize", i,
					SIGNAL(clicked()), this, SLOT(maxButtonPressed()));
			break;

		  case 'X': // Close button
			addButton(BtnType::CLOSE, "Close", BtnImg::CLOSE,
					SIGNAL(clicked()), client, SLOT(closeWindow()));
			break;

		  case 'F': // Keep Above button
			addButton(BtnType::ABOVE, "Above", BtnImg::ABOVE,
					SIGNAL(clicked()), this, SLOT(aboveButtonPressed()));
			break;

		  case 'B': // Keep Below button
			addButton(BtnType::BELOW, "Below", BtnImg::BELOW,
					SIGNAL(clicked()), this, SLOT(belowButtonPressed()));
			break;

		  case 'L': // Shade button
			addButton(BtnType::SHADE, "Shade", BtnImg::SHADE,
					SIGNAL(clicked()), this, SLOT(shadeButtonPressed()));
			break;

		  case 'R': // Resize button
			addButton(BtnType::RESIZE, "Resize", BtnImg::RESIZE,
					SIGNAL(clicked()), this, SLOT(resizeButtonPressed()));
			break;

		case '_': // Spacer item
			box->addSpacing(SPACERSIZE);
			btnswidth+=SPACERSIZE;
			break;

		  default:
			kdDebug()<<"Unknown Button: "<<char(s[i])<<endl;
			break;
		}
	}
	setFixedSize(
			btnswidth+SLANT_WIDTH,
			BTN_HEIGHT*2+2
	);
	prepMask();
}

// Add a button to title layout (called by addButtons() )
void Bar::addButton(BtnType::Type b, const char *name,
		BtnImg::Img img, const char* signal, QObject *recv,
		const char* slot)
{
	unless(button[b]) {
		button[b] = new Button(this, name, i18n(name), client, b, img);
		connect(button[b], signal, recv, slot);
		box->addWidget (button[b],0,Qt::AlignTop);
	}
	btnswidth+=BTN_WIDTH;
}

void Bar::prepMask() {
	mask=QRegion();//clear the mask

	//add the boxes
	mask+=QRegion(0, 0, FRAMESIZE+1, FRAMESIZE);
	mask+=QRegion(FRAMESIZE+1, 0, btnswidth, BTN_HEIGHT+2);
	mask+=QRegion(width()-FRAMESIZE, BTN_HEIGHT+2, FRAMESIZE, BTN_HEIGHT-2);

	//add the corners
	#include "barcorner.xbm"
	QBitmap corner(
			bar_corner_xbm_width,bar_corner_xbm_height,
			bar_corner_xbm, true
	);
	QRegion r(corner);
	r.translate(1,FRAMESIZE);
	mask+=r;
	r.translate(btnswidth-FRAMESIZE+1,BTN_HEIGHT+2-FRAMESIZE);
	mask+=r;
}

//show bar and coverup other bars
void Bar::show() {
	QWidget::show();
	QRegion geom(client->geometry());
	doMask();
}

//hide bar and show other bars
void Bar::hide() {
	QWidget::hide();
	QRegion geom(client->geometry());
	doMask();
}

//moves the bar to the correct location iff this is a toprlevel widget
void Bar::reposition() {
	//kdDebug()<<"Bar::reposition("<<client->geometry()<<") : "<<client->caption()<<endl;

	int x=client->width()-width()-1;
	int y=2;
	if(toplevel) {
		move(x-FRAMESIZE,y-FRAMESIZE);
	} else {
		move(x,y);
	}
}

//recalculates transparent reigon
void Bar::doMask(bool /*local*/) {
	//kdDebug()<<"        Bar::doMask() : "<<client->caption()<<endl;
	unless(toplevel) return;

	//"r t" stands for relative to as in foo is relative to the top-left corner of baz

	QRect geom = geometry(); //geom is relative to screen
	QRect cli=client->geometry(); //cli is r t screen
	geom.moveBy(-cli.left(),-cli.top()); //geom is now r t client->widget()

	setMask(mask);
}

// window active state has changed
void Bar::activeChange(bool /*active*/) {
}

// Called when desktop/sticky changes
void Bar::desktopChange(bool onAllDesktops) {
	if (button[BtnType::STICKY]) {
		button[BtnType::STICKY]->setPixmap(onAllDesktops ? BtnImg::STICK : BtnImg::UNSTICK);
		QToolTip::remove(button[BtnType::STICKY]);
		QToolTip::add(button[BtnType::STICKY], onAllDesktops ? i18n("Un-Sticky") : i18n("Sticky"));
	}
}

// Maximized state has changed
void Bar::maximizeChange(bool maximizeMode) {
	if (button[BtnType::MAX]) {
		//set the image
		BtnImg::Img i;
		if(client->maximizeMode()==KDecorationDefines::MaximizeFull)
			i=BtnImg::RESTORE;
		else
			i=BtnImg::MAX;
		button[BtnType::MAX]->setPixmap(i);
		
		QToolTip::remove(button[BtnType::MAX]);
		QToolTip::add(button[BtnType::MAX], maximizeMode ? i18n("Restore") : i18n("Maximize"));
	}
}

// Max button was pressed
void Bar::maxButtonPressed() {
	if (button[BtnType::MAX]) {
		switch (button[BtnType::MAX]->lastMousePress()) {
		  case MidButton:
			client->maximize(client->maximizeMode() ^ KDecoration::MaximizeVertical);
			break;
		  case RightButton:
			client->maximize(client->maximizeMode() ^ KDecoration::MaximizeHorizontal);
			break;
		  default:
			if (client->maximizeMode() == KDecoration::MaximizeFull) {
				client->maximize(KDecoration::MaximizeRestore);
			} else {
				client->maximize(KDecoration::MaximizeFull);
			}
		}
	}
}

// Menu button was pressed (popup the menu)
void Bar::menuButtonPressed() {
	if (button[BtnType::MENU]) {
		QPoint p(button[BtnType::MENU]->rect().bottomLeft().x(),
				 button[BtnType::MENU]->rect().bottomLeft().y());
		KDecorationFactory* f = client->factory();
		client->showWindowMenu(button[BtnType::MENU]->mapToGlobal(p));
		unless(f->exists(client)) return; // decoration was destroyed
		button[BtnType::MENU]->setDown(false);
	}
}

void Bar::aboveButtonPressed() {
	client->setKeepAbove( ! client->keepAbove());
}

void Bar::belowButtonPressed() {
	client->setKeepBelow( ! client->keepBelow());
}

void Bar::shadeButtonPressed() {
	/*shade just doesn't make sense for this decoration; so I lower the window instead */
	//client->setShade( ! client->isShade());
	client->performWindowOperation(KDecoration::LowerOp);
}

void Bar::resizeButtonPressed() {
	client->performWindowOperation(KDecoration::ResizeOp);
}

void Bar::paintEvent(QPaintEvent*) {
	unless(fitzFactoryInitialized()) return;

	QColorGroup group;
	QPainter painter(this);
	
	group = client->options()->colorGroup(KDecoration::ColorTitleBar, true);

	
	QPoint a(0,FRAMESIZE-2);
	QPoint b(btnswidth-2,BTN_HEIGHT+2);
	
	//draw the two triangles
	for(int y=0;y<(BTN_HEIGHT-FRAMESIZE+4);y++) {
		int x=y/2;
		painter.setPen(group.dark());
		painter.drawPoint(a+QPoint(x, y));
		painter.drawPoint(b+QPoint(x, y));
		painter.setPen(group.background());
		painter.drawLine(a+QPoint(x+1,y) , a+QPoint(SLANT_WIDTH,y));
		painter.drawLine(b+QPoint(x+1,y) , b+QPoint(SLANT_WIDTH+1,y));
	}

	//draw the part of the frame below the buttons
	painter.setPen(group.background());
	painter.drawLine(
			SLANT_WIDTH,BTN_HEIGHT,
			width(),BTN_HEIGHT
			);
	painter.drawLine(
			btnswidth-2,BTN_HEIGHT+1,
			width(),BTN_HEIGHT+1
			);
	painter.setPen(group.dark());
	painter.drawLine(
			SLANT_WIDTH,BTN_HEIGHT+1,
			btnswidth-3,BTN_HEIGHT+1
	);/* I have no idea why that is (btnswidth-3) as
	opposed to btnswidth, but that magic constant works.*/
	painter.fillRect(
			0,0,
			SLANT_WIDTH,FRAMESIZE-2,
			group.background()
	);
	
}

}

#include "bar.moc"
