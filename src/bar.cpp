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
#include <qfont.h>
#include <qbitmap.h>
#include <qpainter.h>
#include <qtooltip.h>
#include <qapplication.h>

//fitz
#include "bar.h"
#include "button.h"

//X11
#include <X11/Xlib.h>

namespace Fitz {

// Constructor
Bar::Bar(KDecoration *parent, const char *name, bool tl)
		: QWidget (parent->widget(), name, tl?(WType_TopLevel | WX11BypassWM):0),
		client(parent), reparented(0), toplevel(tl), corners(6)
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
	slantWidth = (BTN_HEIGHT-FRAMESIZE+4)/2;
	titleSpace = new QSpacerItem(BTN_WIDTH,BTN_HEIGHT);
	titleBar = new QPixmap;
	
	setMouseTracking(true);
	installEventFilter(this);
	
	setMask(QRegion());
}

Bar::~Bar() {
	for (int n=0; n<BtnType::COUNT; n++) {
		if (button[n]) delete button[n];
	}
	//delete titleBar;
}

// Add buttons to title layout
void Bar::addButtons(const QString& s) {
	kdDebug()<<"Bar::addButtons()"<<endl;
	btnsWidth=0;
	box->addSpacing(slantWidth);
	for (unsigned i=0; i < s.length(); i++) {
		switch (s[i]) {

	// Buttons
		  case 'M': // Menu button
			addButton(BtnType::MENU, "Menu",
					SIGNAL(pressed()), this, SLOT(menuButtonPressed()));
			break;

		  case 'H': // Help button
			if(client->providesContextHelp())
				addButton(BtnType::HELP, "Help",
					SIGNAL(clicked()), client, SLOT(showContextHelp()));
			break;

		  case 'I': // Minimize button
			addButton(BtnType::MIN, "Minimize",
					SIGNAL(clicked()), client, SLOT(minimize()));
			break;

		  case 'A': // Maximize button
			addButton(BtnType::MAX, "Maximize",
					SIGNAL(clicked()), this, SLOT(maxButtonPressed()));
			//set the pixmap to the correct mode
			button[BtnType::MAX]->setPixmap(client->maximizeMode());
			break;

		  case 'X': // Close button
			addButton(BtnType::CLOSE, "Close",
					SIGNAL(clicked()), client, SLOT(closeWindow()));
			break;

		  case 'R': // Resize button
			addButton(BtnType::RESIZE, "Resize",
					SIGNAL(clicked()), this, SLOT(resizeButtonPressed()));
			break;

	// Buttons that can be toggled
		  case 'S': // Sticky button
			addButton(BtnType::STICKY, "Sticky", client->isOnAllDesktops(),
					SLOT(toggleOnAllDesktops())); 
			break;

		  case 'F': // Keep Above button
			addButton(BtnType::ABOVE, "Above", client->keepAbove(),
					SLOT(setKeepAbove(bool)));
			connect(client,SIGNAL(keepAboveChanged(bool)),
					button[BtnType::ABOVE],SLOT(setPixmap(bool)));
			break;

		  case 'B': // Keep Below button
			addButton(BtnType::BELOW, "Below", client->keepBelow(),
					SLOT(setKeepBelow(bool)));
			connect(client,SIGNAL(keepBelowChanged(bool)),
					button[BtnType::BELOW],SLOT(setPixmap(bool)));
			break;

		  case 'L': // Shade button
			addButton(BtnType::SHADE, "Shade", client->isSetShade(),
					SLOT(setShade(bool)));
			break;

	// Things that aren't buttons
		  case '_': // Spacer item
			box->addSpacing(SPACERSIZE);
			btnsWidth+=SPACERSIZE;
			break;

		  case ' ': // Title bar
			box->addItem(titleSpace);
			captionChange(client->caption());
			break;

		  default:
			kdDebug()<<"Unknown Button: "<<char(s[i])<<endl;
			break;
		}
	}
	resize();
}

// Add a generic button to title layout (called by addButtons() )
void Bar::addButton(BtnType::Type b, const char *name,
		const char* signal, QObject *recv, const char* slot)
{
	unless(button[b]) {
		button[b] = new Button(this, name, client, b);
		connect(button[b], signal, recv, slot);
		box->addWidget (button[b],0,Qt::AlignTop);
	}
	btnsWidth+=BTN_WIDTH;
}

// This adds a button that can be toggled ( it is also called by addButtons() )
void Bar::addButton(BtnType::Type b, const char *name, bool on,
		const char* slot)
{
	addButton(b,name,SIGNAL(toggled(bool)),client,slot);
	button[b]->setPixmap(on);
	connect(button[b],SIGNAL(clicked()),button[b],SLOT(toggle()));
}

void Bar::resize() {
	setFixedSize(
			btnsWidth+slantWidth,
			BTN_HEIGHT+2+slantWidth*2
	);
	
	int frameX = width()-FRAMESIZE+2;
	int barY = BTN_HEIGHT+2;
	
	corners.putPoints(
		0, 6,
		0, 0,
		0, FRAMESIZE-1,
		slantWidth, barY,
		frameX-slantWidth-1, barY,
		frameX, barY+slantWidth*2+2,
		frameX,0
	);

	reposition();

	if(reparented)
		setMask(QRegion(corners));
}

//moves the bar to the correct location iff this is a toplevel widget
void Bar::reposition() {
	//kdDebug()<<"Bar::reposition("<<client->geometry()<<") : "<<client->caption()<<endl;

	int x=client->width()-width()-1;
	int y=2;
	if(FRAMESIZE>2)
		y=1;
	move(x,y);
}

//this function finds the parent window of the window decoration widget and
//makes the bar a child of that window
void Bar::reparent() {
	Display* disp = x11Display();
	Window barWin = winId();
	Window deco = client->widget()->winId();
	Window root;
	Window parent;
	Window *children;
	unsigned int num_children;
	
	XQueryTree(disp, deco, &root, &parent, &children, &num_children);
	if (children)
		XFree(children);
	
	XReparentWindow(disp, barWin, parent, 0, 0);
	reparented = true;
	resize();
}

// Event filter
bool Bar::eventFilter(QObject *obj, QEvent *e) {
	if (obj != this) return false;
	//kdDebug()<<"Bar::eventFilter("<<e->type()<<") : "<<client->caption()<<endl;
	QMouseEvent *me;
	QWheelEvent *we;
	QEvent *event;
	
	switch (e->type()) {
	  case QEvent::MouseButtonPress:
	  case QEvent::MouseButtonRelease:
	  case QEvent::Leave:
	  case QEvent::Enter:
	  case QEvent::MouseMove:
	  case QEvent::MouseButtonDblClick:
		me = static_cast<QMouseEvent *>(e);
		
		//move the posiiton of the event so that it is relative to
		//client->widget()
		event = new QMouseEvent(
			me->type(), me->pos() + pos(),
			me->globalPos(), me->button(), me->state()
		);
		break;
		
	  case QEvent::Wheel:
		we = static_cast< QWheelEvent* >( e );
		
		//move the posiiton of the event so that it is relative to
		//client->widget()
		event = new QWheelEvent(
			we->pos() + pos(), we->globalPos(), we->delta(), we->state(),
			we->orientation()
		);
		//QWheelEvent ( const QPoint & pos, const QPoint & globalPos, int delta, int state, Orientation orient = Vertical )
		break;
			
	  default:
		return false;
	}
	QApplication::postEvent(client->widget(),event);
	return true;
}


// window active state has changed
void Bar::activeChange(bool /*active*/) {
}

// Called when desktop/sticky changes
void Bar::desktopChange(bool onAllDesktops) {
	if (button[BtnType::STICKY]) {
		button[BtnType::STICKY]->setPixmap(onAllDesktops);
		QToolTip::remove(button[BtnType::STICKY]);
		QToolTip::add(button[BtnType::STICKY], onAllDesktops ? i18n("Un-Sticky") : i18n("Sticky"));
	}
}

// Maximized state has changed
void Bar::maximizeChange(bool maximizeMode) {
	if (button[BtnType::MAX]) {
		//set the image
		button[BtnType::MAX]->setPixmap(client->maximizeMode());
		
		QToolTip::remove(button[BtnType::MAX]);
		QToolTip::add(button[BtnType::MAX], maximizeMode ? i18n("Restore") : i18n("Maximize"));
	}
}

// window active state has changed
void Bar::captionChange(const QString& caption) {
	//make the string shorter - remove everything after " - "
	QString file(caption);
	file.truncate(file.find(" - "));
	kdDebug()<<"Bar::caption("<<file<<")"<<endl;

	//change the font
	QFont font = client->options()->font();
	font.setPixelSize(BTN_HEIGHT-1);
	font.setItalic(false);
	font.setWeight(QFont::DemiBold);
	font.setStretch(130);
	
	//figure out the width
	QFontMetrics fm(font);
	int width = fm.width(file) + BTN_WIDTH/2;
	int oldWidth = titleBar->width();
	btnsWidth+=width-oldWidth;
	titleBar->resize(width,BTN_HEIGHT);
	
	//read in colors
	QColor fg=KDecoration::options()->color(KDecoration::ColorFont);
	QColor bg=KDecoration::options()->color(KDecoration::ColorTitleBar);
	
	
	//make the bitmap for the caption
	QPainter p;
	p.begin(titleBar);
	p.setPen(fg);
	p.setFont(font);
	p.fillRect(0,0,width,BTN_HEIGHT,bg);
	p.shear(0,.5);
	p.drawText(0,0,width,BTN_HEIGHT,AlignLeft|AlignVCenter,file);
	p.end();
	
	if(oldWidth) {
		resize();
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

void Bar::paintEvent(QPaintEvent*) {
	kdDebug()<<"Bar::paintEvent()"<<endl;
	unless(fitzFactoryInitialized()) return;

	QPointArray line;
	line.putPoints(0,4,corners,1);
	line.translate(0,-1);

	QPointArray fill(6);
	fill.putPoints(1,4,corners,1);
	fill.translate(0,-2);
	fill.setPoint(0,0,0);
	fill.setPoint(5,width()-FRAMESIZE+2,0);
	
	QColorGroup group;
	group = client->options()->colorGroup(KDecoration::ColorTitleBar, true);
	
	QPainter painter(this);
	painter.setPen(group.background());
	painter.setBrush(group.background());
	painter.drawPolygon(fill);
	painter.setPen(group.dark());
	painter.drawPolyline(line);
	
	QPoint origin = titleSpace->geometry().topLeft();
	painter.drawPixmap(origin,*titleBar);
}

}

#include "bar.moc"
