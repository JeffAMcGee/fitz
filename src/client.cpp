//////////////////////////////////////////////////////////////////////////////
// client.cpp
// -------------------
// Fitz window decoration for KDE
// -------------------
// Copyright (c) 2003-2005 by Jeffrey McGee <JeffAMcGee@gmail.com>
// Portions Copyright (c) 2003 by David Johnson <david@usermode.org>
// 
// You can Freely distribute this program under the GNU General Public
// License. See the file "COPYING" for the exact licensing terms.
//////////////////////////////////////////////////////////////////////////////

//kde
#include <klocale.h>
#include <kdebug.h>
#include <kdecorationfactory.h>

//qt
#include <qlabel.h>
#include <qlayout.h>
#include <qtimer.h>
#include <qfont.h>
#include <qbitmap.h>
#include <qpainter.h>
#include <qmemarray.h>
#include <qtooltip.h>
#include <qcursor.h>
#include <qapplication.h>
#include <qvariant.h> //needed for SuSE rpm

//fitz
#include "fitz.h"
#include "client.h"
#include "factory.h"
#include "fakemouse.h"
#include "button.h"

//C
#include <assert.h>

//X11
#include <X11/Xlib.h>

/*kdbgstream& operator<<( kdbgstream &kd, const QPointArray &pa ) {
    kd << "[";
	for(unsigned int i=0;i<pa.size();i++) {
		kd << pa.point(i);
		if(i<pa.size())
			kd << ", ";
	}
	kd << "]";
    return kd;
}*/

namespace Fitz {

///////////////////////////////////////////////////////////
// initialization

// Constructor
Client::Client(KDecorationBridge *b, KDecorationFactory *f) 
	: KDecoration(b, f), event(0) { ; }

Client::~Client() {
	for (int n=0; n<BtnType::COUNT; n++) {
		if (button[n]) delete button[n];
	}
	if(slow) {
		Display* disp = bar->x11Display();
		XChangePointerControl(disp, do_accel, do_thresh, accel_num, accel_denom, thresh);
	}
	titleBar->resize(0,0);
	//Which is better: A crash or a leak?
	//delete titleBar;
}

//constants for init()
int Client::framesize_ = 3;
QRegion Client::headWinMask;
QRegion Client::headDiaMask;
QRegion Client::tailMask;
QRegion Client::cornerMask;

// Actual initializer for class
void Client::init() {
	createMainWidget(WResizeNoErase | WRepaintNoErase);
	widget()->installEventFilter(this);
	widget()->setBackgroundMode(Qt::NoBackground);
	
	kdDebug()<<"init() "<<caption()<<endl;

	mainLayout = new QGridLayout(widget(), 4, 3);
	/*The mainLayout looks like this:

	  *         *
	***************
	  *     the bar   
	***************
	  *         *
	  *  label  *
	  *         *
	***************
	  *         *    */

	mainLayout->setResizeMode(QLayout::FreeResize);
	mainLayout->addRowSpacing(0, 2);
	mainLayout->addRowSpacing(3, framesize_);
	mainLayout->addColSpacing(0, framesize_);
	mainLayout->addColSpacing(2, framesize_);

	// the window should stretch
	mainLayout->setRowStretch(2, 10);
	mainLayout->setColStretch(1, 10);
	
	if (isPreview()) {
		dialogType = dialog = !isActive();
		barInit();
		
		label = new QLabel(i18n("<b><center>Fitz preview</center></b>"), widget());
		mainLayout->addWidget(label,2,1);
	} else {
		NET::WindowType type = windowType(
				NET::NormalMask | NET::DesktopMask | NET::DockMask |
				NET::ToolbarMask | NET::MenuMask | NET::DialogMask |
				NET::OverrideMask | NET::TopMenuMask |
				NET::UtilityMask | NET::SplashMask
		);
		dialogType = dialog = (type != NET::Normal);
		barInit();

		//we don't want to reparent until the window exists and qt has started
		//processing the event loop
		QTimer::singleShot(0,this,SLOT(reparent()));
		
		//maximize the window if appropriate
		if(Factory::autoMax() && type == NET::Normal)
			QTimer::singleShot(0,this,SLOT(maximizeFull()));
	}
	
	mainLayout->addMultiCellWidget(bar,1,1,1,2,AlignRight);

	// setup titlebar buttons
	addButtons(
		options()->titleButtonsLeft() + " " +
		options()->titleButtonsRight()
	);
	updateColors();
}

void Client::barInit() {
	bar = new QWidget(widget(), "button bar", 0);
	slow = false;
	togglingDialog = false;

	bar->setBackgroundMode(Qt::NoBackground);
	bar->setSizePolicy(QSizePolicy(QSizePolicy::Maximum,QSizePolicy::Fixed));
	
	box = new QBoxLayout(bar, QBoxLayout::LeftToRight, 0, 0, "Fitz::Bar Layout");

	// setup titlebar buttons
	for (int n=0; n<BtnType::COUNT; n++) {
		button[n] = 0;
	}
	
	titleSpace = new QSpacerItem(0,BTN_HEIGHT);
	titleBar = new QPixmap;
		
	bar->setMouseTracking(true);
	bar->installEventFilter(this);
}

//this function finds the parent window of the window decoration widget and
//makes the bar a child of that window
void Client::reparent() {
	Display* disp = bar->x11Display();
	Window deco = widget()->winId();
	Window root;
	Window parent;
	Window *children;
	unsigned int num_children;
	
	XQueryTree(disp, deco, &root, &parent, &children, &num_children);
	if (children)
		XFree(children);
	
	XReparentWindow(disp, deco, parent, 0, 0);
	kdDebug()<<"reparent() "<<caption()<<geometry()<<deco<<endl;
	resizeBar();
}

///////////////////////////////////////////////////////////
// button creation

// Add buttons to title layout
void Client::addButtons(const QString& s) {
	for (unsigned i=0; i < s.length(); i++) {
		switch (s[i]) {

	// Buttons
		  case 'M': // Menu button
			addButton(BtnType::MENU, "Menu",
					SIGNAL(pressed()), SLOT(menuButtonPressed()));
			break;

		  case 'H': // Help button
			if(providesContextHelp())
				addButton(BtnType::HELP, "Help",
					SIGNAL(clicked()), SLOT(showContextHelp()));
			break;

		  case 'I': // Minimize button
			addButton(BtnType::MIN, "Minimize",
					SIGNAL(clicked()), SLOT(minimize()));
			break;

		  case 'A': // Maximize button
			addButton(BtnType::MAX, "Maximize",
					SIGNAL(clicked()), SLOT(maxButtonPressed()));
			//set the pixmap to the correct mode
			button[BtnType::MAX]->setPixmap(maximizeMode());
			break;

		  case 'X': // Close button
			addButton(BtnType::CLOSE, "Close",
					SIGNAL(clicked()), SLOT(closeWindow()));
			break;

		  case 'R': // Resize button
			addButton(BtnType::RESIZE, "Resize",
					SIGNAL(clicked()), SLOT(resizeButtonPressed()));
			break;

	// Buttons that can be toggled
		  case 'S': // Sticky button
			addButton(BtnType::STICKY, "Sticky", isOnAllDesktops(),
					SLOT(toggleOnAllDesktops())); 
			break;

		  case 'F': // Keep Above button
			addButton(BtnType::ABOVE, "Above", keepAbove(),
					SLOT(setKeepAbove(bool)));
			connect(this,SIGNAL(keepAboveChanged(bool)),
					button[BtnType::ABOVE],SLOT(setPixmap(bool)));
			break;

		  case 'B': // Keep Below button
			addButton(BtnType::BELOW, "Below", keepBelow(),
					SLOT(setKeepBelow(bool)));
			connect(this,SIGNAL(keepBelowChanged(bool)),
					button[BtnType::BELOW],SLOT(setPixmap(bool)));
			break;

		  case 'L': // Shade button
			addButton(BtnType::SHADE, "Shade", isSetShade(),
					SLOT(setShade(bool)));
			break;

	// Things that aren't buttons
		  case '_': // Spacer item
			box->addSpacing(SPACERSIZE);
			break;

		  case ' ': // Title bar
			box->addItem(titleSpace);
			captionChange();
			break;

		  default:
			kdDebug()<<"Unknown Button: "<<char(s[i])<<endl;
			break;
		}
	}
}

// Add a generic button to title layout (called by addButtons() )
void Client::addButton(BtnType::Type b, const char *name,
		const char* signal, const char* slot)
{
	unless(button[b]) {
		button[b] = new Button(bar, name, this, b, isActive());
		connect(button[b], signal, this, slot);
		connect(this, SIGNAL(activeChanged(bool)), button[b], SLOT(setActive(bool)));
		box->addWidget (button[b],0,Qt::AlignTop);
	}
}

// This adds a button that can be toggled ( it is also called by addButtons() )
void Client::addButton(BtnType::Type b, const char *name, bool on,
		const char* slot)
{
	addButton(b,name,SIGNAL(toggled(bool)),slot);
	button[b]->setPixmap(on);
	connect(button[b],SIGNAL(clicked()),button[b],SLOT(toggle()));
}


///////////////////////////////////////////////////////////
// actions

void Client::maximizeFull() {
	QWidget *w=widget();
	if(w->width()>500 && w->height()>400)
		maximize(KDecoration::MaximizeFull);
}

// window active state has changed
void Client::activeChange() {
	emit activeChanged(isActive());
	updateColors();
}

// Called when desktop/sticky changes
void Client::desktopChange() {
	if (button[BtnType::STICKY]) {
		button[BtnType::STICKY]->setPixmap(isOnAllDesktops());
		QToolTip::remove(button[BtnType::STICKY]);
		QToolTip::add(button[BtnType::STICKY], isOnAllDesktops() ? i18n("Un-Sticky") : i18n("Sticky"));
	}
}

// The icon has changed, but I don't care.
void Client::iconChange() { ; }

// The title has changed
void Client::captionChange() {
	int oldWidth = titleBar->width();
	int width = redrawTitle();
	
	titleSpace->changeSize(
		width, BTN_HEIGHT,
		QSizePolicy::Maximum, QSizePolicy::Fixed
	);
	box->activate();
	mainLayout->activate();
	
	//force repaint even if resize was a nop
	bar->update();
	
	if(oldWidth) {
		kdDebug()<<"captionChange()"<<endl;
		resizeBar();
	}
}

void Client::updateColors() {
	fgc = KDecoration::options()->color(KDecoration::ColorFont,isActive());
	bgc = KDecoration::options()->color(KDecoration::ColorTitleBar,isActive());
	//widget()->setPaletteBackgroundColor(bgc);
	//bar->setPaletteBackgroundColor(bgc);
	redrawTitle();
	widget()->repaint(false);
}

int Client::redrawTitle() {
	//make the string shorter - remove everything after " - "
	QString file(caption());
	file.truncate(file.find(" - "));
	if(file.length() > 50) {
		file.truncate(50);
		file+= "...";
	}
	
	//change the font
	QFont font = options()->font();
	font.setPixelSize(BTN_HEIGHT-1);
	font.setItalic(false);
	font.setWeight(QFont::DemiBold);
	font.setStretch(130);
	
	QFontMetrics fm(font);
	int width = fm.width(file) + BTN_WIDTH/2;
	titleBar->resize(width,BTN_HEIGHT);
	
	//make the bitmap for the caption
	QPainter p;
	p.begin(titleBar);
	p.setPen(fgc);
	p.setFont(font);
	p.fillRect(0,0,width,BTN_HEIGHT,bgc);
	p.drawText(0,0,width,BTN_HEIGHT,AlignLeft|AlignVCenter,file);
	p.end();
	
	return width;
}

// Maximized state has changed
void Client::maximizeChange() {
	if (button[BtnType::MAX]) {
		//set the image
		button[BtnType::MAX]->setPixmap(maximizeMode());
		
		QToolTip::remove(button[BtnType::MAX]);
		QToolTip::add(button[BtnType::MAX], maximizeMode() ? i18n("Restore") : i18n("Maximize"));
	}
}

// The window has been shaded or unshaded
void Client::shadeChange() { ; }


// Max button was pressed
void Client::maxButtonPressed() {
	if (button[BtnType::MAX]) {
		switch (button[BtnType::MAX]->lastMousePress()) {
		  case MidButton:
			maximize(maximizeMode() ^ KDecoration::MaximizeVertical);
			break;
		  case RightButton:
			maximize(maximizeMode() ^ KDecoration::MaximizeHorizontal);
			break;
		  default:
			if (maximizeMode() == KDecoration::MaximizeFull) {
				maximize(KDecoration::MaximizeRestore);
			} else {
				maximize(KDecoration::MaximizeFull);
			}
		}
	}
}

// Menu button was pressed (popup the menu)
void Client::menuButtonPressed() {
	if (button[BtnType::MENU]) {
		QPoint p(button[BtnType::MENU]->rect().bottomLeft().x(),
				 button[BtnType::MENU]->rect().bottomLeft().y());
		KDecorationFactory* f = factory();
		showWindowMenu(button[BtnType::MENU]->mapToGlobal(p));
		unless(f->exists(this)) return; // decoration was destroyed
		button[BtnType::MENU]->setDown(false);
	}
}

void Client::resizeButtonPressed() {
	performWindowOperation(KDecoration::ResizeOp);
}

void Client::reset(unsigned long changed) {
	kdDebug()<<"reset()"<<endl;
	if(changed & SettingColors)
		updateColors();
}

///////////////////////////////////////////////////////////
// size stuff

// Get the size of the borders
void Client::borders(int &l, int &r, int &t, int &b) const {
	l = r = t = b = framesize_;
	kdDebug()<<"borders()"<<endl;
	if(isShade()) {
		t=BTN_HEIGHT+5;
		b=0;
	} else if(dialog)
		t=BTN_HEIGHT+4;
	else if(isPreview())
		t=BTN_HEIGHT+8;
}

// Called to resize or move the window
void Client::resize(const QSize &size) {
	kdDebug()<<"resize() "<<size<<togglingDialog<<heightBeforeToggle<<endl;
	if(togglingDialog && heightBeforeToggle > size.height())
		return;
	widget()->resize(size);
	resizeBar();
}

void Client::resizeBar() {
	kdDebug()<<"resizeBar() : "<<caption()<<geometry()
		<<bar->geometry()<<dialog<<frameGeom()<<endl;
	
	int newWidth = width() -barWidth();
	
	if( 
		(newWidth<400 && !dialog && !isPreview() ) ||
		(newWidth>500 && dialog && !dialogType && !isPreview() )
	) {
		toggleDialog();
	}
	
	doMask();
}

//just a helper for doMask()
void setParentMask(QWidget *w, QRegion r) {
	QPoint p = w->mapFromParent(QPoint(0,0));
	r.translate(p.x(),p.y());
	w->setMask(r);
}

void Client::doMask() {
	//Welcome to the land of magic constants.  I hope you enjoy your stay.
	
	QRegion outside(frameGeom());
	QRegion mask;
	QRegion tmp;
	
	//bar
	QRect barGeom = bar->geometry();
	barGeom.addCoords(dialog?2:5,-2,1,2);
	mask+=QRegion(barGeom);
	
	//head
	tmp = (dialog?headDiaMask:headWinMask);
	tmp.translate(barGeom.left()-headWidth()+1,dialog ? 0 : framesize_);
	mask+=tmp;
	
	//tail
	tmp = tailMask;
	tmp.translate(width() - framesize_ - tailWidth()-1, BTN_HEIGHT+4);
	mask+=tmp;
	

	//inside corners
	mask+=QRegion(framesize_, dialog?BTN_HEIGHT+4:framesize_, 1, 1);
	mask+=QRegion(framesize_, height()-framesize_-1, 1, 1);
	mask+=QRegion(width()-framesize_-1, height()-framesize_-1, 1, 1);

	//outside corners
	enum{LEFT=1,RIGHT=2,TOP=4,BOTTOM=8};
	int edges = 0;
	if(!isPreview()) {
		QWidget *d = QApplication::desktop();
		QRect r = widget()->geometry();
		QPoint tl = widget()->mapToGlobal(r.topLeft());
		QPoint br = widget()->mapToGlobal(r.bottomRight());
		
		if(tl.x()==0)
			edges+=LEFT;
		if(br.x()==d->width()-1)
			edges+=RIGHT;
		if(tl.y()==0)
			edges+=TOP;
		if(br.y()==d->height()-1)
			edges+=BOTTOM;
	}

	tmp=cornerMask&QRegion(0,0,2,2);
	if(dialog && headWidth() + bar->width() < width() ) {
		tmp.translate(0,BTN_HEIGHT-framesize_+4);
		outside-=tmp;
	} else if(!((edges&LEFT) || (edges&TOP))) {
		outside-=tmp;
	}
	
	tmp=cornerMask&QRegion(0,2,2,2);
	tmp.translate(0,height()-4);
	if(!((edges&LEFT) || (edges&BOTTOM)) )
		outside-=tmp;
	
	tmp=cornerMask&QRegion(2,2,2,2);
	tmp.translate(width()-4,height()-4);
	if(!((edges&RIGHT) || (edges&BOTTOM)) )
		outside-=tmp;
	
	tmp=cornerMask&QRegion(2,0,2,2);
	tmp.translate(width()-4,0);
	if(!((edges&RIGHT) || (edges&TOP)) ) {
		outside-=tmp;
		mask-=tmp;
	}
	
	QRect insideRect = frameGeom();
	insideRect.addCoords(framesize_,framesize_,-framesize_,-framesize_);
	insideMask = QRegion(insideRect)-mask;
	
	setMask(mask+outside);
	if(isShade() && !dialog) {
		widget()->setMask(outside);
		setParentMask(bar,mask);
	} else if(!isPreview()) {
		widget()->setMask(outside-insideMask+mask);
	} else {
		widget()->setMask(mask+outside);
		QPoint p = label->mapFromParent(QPoint(0,0));
		setParentMask(label,insideMask);
		if(!dialog)
			setParentMask(bar,mask);
	}
	kdDebug()<<"doMask() : "<<caption()
		<<" frame:"<<frameGeom()
		<<" out:"<<outside.boundingRect()
		<<" in:"<<insideMask.boundingRect()
		<<" mask:"<<mask.boundingRect()<<endl;
}

void Client::toggleDialog() {
	dialog=!dialog;
	
	box->invalidate();
	
	kdDebug()<<"toggleDialog() : "<<caption()<<endl;
	
	//tell kwin about our change in borders()
	if( !isShade() ) {
		heightBeforeToggle = widget()->height();
		//int change = (dialog?-1:1)*(framesize_-(BTN_HEIGHT+4);
		//QSize s = widget()->size() + QSize(0,change);
		//kdDebug()<<" "<<s<<" "<<change<<endl;

		togglingDialog = 1;
		setShade(1);
		setShade(0);
		togglingDialog = 0;
		return;
	}
	return;
}

int Client::headHeight(bool /*dia*/) {
	return BTN_HEIGHT+4-framesize_;
}

int Client::headWidth(bool dia) {
	int h = BTN_HEIGHT+4-framesize_;
	return dia ? (h*2+1) : (h/2+2) ; 
}

int Client::barWidth() const {
	return bar->width() +headWidth();
}

QRect Client::frameGeom() const {
	//kdDebug()<<"frameGeom"<<geometry()<<widget()->geometry()<<endl;
	QRect frame = widget()->geometry();
	if(isPreview()) {
		frame.moveTop(0);
		frame.moveLeft(0);
	}
	if(dialog) {
		frame.setTop(headHeight());
	}
	return frame;
}

// Return the minimum allowable size for this decoration
QSize Client::minimumSize() const {
	QSize s = box->minimumSize();
	return QSize(s.width()-framesize_*2, 40);
}

void Client::setBorderSize(BorderSize b) {
	kdDebug()<<"setBorderSize() : "<<endl;
	switch(b) {
	  case BorderTiny:
		framesize_ = 1;
		break;
	  case BorderNormal:
		framesize_ = 3;
		break;
	  case BorderLarge:
		framesize_ = 5;
		break;
	  case BorderVeryLarge:
		framesize_ = 9;
		break;
	  case BorderHuge:
	  case BorderVeryHuge:
	  case BorderOversized:
		framesize_ = 9;
		break;
	  default:
		framesize_ = 3;
		break;
	}
	makeStaticMasks();
}

void Client::makeStaticMasks() {
#include "corner.xbm"	
#include "head_dia.xbm"	
#include "head_win.xbm"	
#include "tail.xbm"	
	
	cornerMask = QRegion(QBitmap(corner_width,corner_height,corner_bits,true));
	tailMask = QRegion(QBitmap(tail_width,tail_height,tail_bits,true));
	
	int w = headWidth(false)/2;
	int h = headHeight(false);
	
	QRegion headWin(QBitmap(head_win_width,head_win_height,head_win_bits,true));
	QRegion left = headWin & QRegion(0,0,w,h);
	QRegion right = headWin ;//& QRegion(head_win_width-w-1,head_win_height-h-1,w,h);
	right.translate(headWidth(false)-head_win_width-1,h-head_win_height);
	headWinMask = left + right;
	
	w = headWidth(true)/2;
	h = headHeight(true);
	
	QRegion headDia(QBitmap(head_dia_width,head_dia_height,head_dia_bits,true));
	left = headDia & QRegion(0,head_dia_height-h,w,h);
	right = headDia & QRegion(head_dia_width-w,0,w,h);
	left.translate(0,h-head_dia_height);
	right.translate(2*w-head_dia_width,0);
	headDiaMask = left + right;
}

///////////////////////////////////////////////////////////
// the great event filter

// Event filter
bool Client::eventFilter(QObject *obj, QEvent *e) {
	if (obj == bar)
		return barEventFilter(obj, e);
	if (obj != widget())
		return false;
	//kdDebug()<<"eventFilter("<<e->type()<<") : "<<caption()<<endl;
 
	switch (e->type()) {
	  case QEvent::MouseButtonPress:
		mousePressEvent(static_cast<QMouseEvent *>(e));
		return true;
	  case QEvent::MouseButtonRelease:
		mouseReleaseEvent(static_cast<QMouseEvent *>(e));
		return true;
	  case QEvent::MouseButtonDblClick:
		mouseDoubleClickEvent( static_cast< QMouseEvent* >( e ));
		return true;
	  case QEvent::Wheel:
		wheelEvent( static_cast< QWheelEvent* >( e ));
		return true;
		
	  case QEvent::Leave:
		mouseLeaveEvent(static_cast<QMouseEvent *>(e));
		return true;
	  case QEvent::Paint:
		paintEvent(static_cast<QPaintEvent *>(e));
		return true;
	  case QEvent::Resize:
		if(isPreview()) {
			kdDebug()<<"eventFilter"<<endl;
			resizeBar();
			return true;
		}
		return false;
	  case QEvent::Show:
		showEvent(static_cast<QShowEvent *>(e));
		return true;
		
	  default:
		return false;
	}

	return false;
}


// the bar event filter just handles mouse junk
bool Client::barEventFilter(QObject *obj, QEvent *e) {
	if (obj != bar) return false;
	//kdDebug()<<"Bar::barEventFilter("<<e->type()<<") : "<<caption()<<endl;
	QMouseEvent *me;
	QWheelEvent *we;
	QEvent *event;
	Display* disp = bar->x11Display();
	
	switch (e->type()) {
	  case QEvent::Leave:
		unless(slow) goto forward;
		
		XChangePointerControl(disp, do_accel, do_thresh, accel_num, accel_denom, thresh);
		slow = false;
		goto forward;
		
	  case QEvent::Enter:
		if(slow || isPreview()) goto forward;
		
		accel_denom = 0; thresh = -1;
		XGetPointerControl(disp, &accel_num, &accel_denom, &thresh);
		do_accel = (accel_denom != 0);
		do_thresh = (thresh != -1);
		slow = true;
		
		XChangePointerControl(disp, 1, 0, 1, 2, 0);
		goto forward;
		  
	  forward:
	  case QEvent::MouseButtonPress:
	  case QEvent::MouseButtonRelease:
	  case QEvent::MouseMove:
	  case QEvent::MouseButtonDblClick:
		me = static_cast<QMouseEvent *>(e);
		
		//move the posiiton of the event so that it is relative to
		//client->widget()
		event = new QMouseEvent(
			me->type(), me->pos() + bar->pos(),
			me->globalPos(), me->button(), me->state()
		);
		break;
		
	  case QEvent::Wheel:
		we = static_cast< QWheelEvent* >( e );
		
		//move the posiiton of the event so that it is relative to
		//client->widget()
		event = new QWheelEvent(
			we->pos() + bar->pos(), we->globalPos(), we->delta(), we->state(),
			we->orientation()
		);
		break;
			
	  case QEvent::Paint:
		//widget's paintEvent took care of it for us
		return true;

	  default:
		return false;
	}
	QApplication::postEvent(widget(),event);
	return true;
}


///////////////////////////////////////////////////////////
// mouse stuff

// Return logical mouse position
KDecoration::Position Client::mousePosition(const QPoint &point) const {
	const int corner = 32;
	Position pos = PositionCenter;
	
	//kdDebug()<<"mousePosition("<<point<<") : "<<caption()<<endl;

	int x = point.x();
	int y = point.y();
	
	if((bar->geometry().contains(point) && y>1) || isShade())
		return PositionCenter;
	
	if (x <= corner)
		pos = PositionLeft;
	else if (x >= (width()-corner))
		pos = PositionRight;

	if (y <= corner)
		 pos = static_cast<Position>(pos | PositionTop);
	else if (y >= (height()-corner))
		pos = static_cast<Position>(pos | PositionBottom);

	return pos;
}


// Deal with mouse click
void Client::mousePressEvent(QMouseEvent *e) {
	//kdDebug()<<"mousePressEvent("<<e->button()<<","<<e->globalX()<<","<<e->globalY()<<")"<<endl;
	if(
		e->button() == (Qt::MouseButtonMask&Qt::LeftButton) &&
		! (bar->geometry().contains(e->pos()) && e->globalY()>1) &&
		! isShade() &&
		! isPreview()
	) {
		//this is a left mouse button press that is not inside the button bar
		assert(event == 0);
		event=new QMouseEvent(
			e->type(), e->pos(),
			e->globalPos(), e->button(), e->state()
		);
	} else {
		processMousePressEvent(e);
	}
}

void Client::mouseReleaseEvent(QMouseEvent *e) {
	//kdDebug()<<"mouseReleaseEvent("<<e->button()<<","<<e->globalX()<<","<<e->globalY()<<")"<<endl;

	if(e->button()==(Qt::MouseButtonMask&Qt::LeftButton) && event !=0) {
		delete event;
		event=0;
		
		QWidget *d = QApplication::desktop();
		int x = e->globalX();
		int y = e->globalY();
		int w = d->width();
		int h = d->height();
		bool willMove = false;

		if(x<framesize_) { //left
			 x=framesize_+6;
			 willMove=true;
		} else if((w-x)<=framesize_) {//right
			x=w-framesize_-6;
			willMove=true;
		}
		if(y<framesize_) { //top
			if( x > w - bar->width() )
				y=6;
			else
				y=framesize_+6;
			willMove=true;
		} else if((h-y)<=framesize_) {//bottom
			y=h-framesize_-6;
			willMove=true;
		}

		if(willMove) {
			QCursor::setPos(x,y);
			Fitz::FakeMouse::click();
		}
	}
}

void Client::mouseLeaveEvent(QMouseEvent * /*e*/) {
	//kdDebug()<<"mouseLeaveEvent("<<e->globalX()<<","<<e->globalY()<<")"<<endl;
	
	if(event!=0) {
		processMousePressEvent(event);
		delete event;
		event=0;
	}
}

#if KDE_IS_VERSION( 3, 4, 89 )
void Client::wheelEvent(QWheelEvent *e) {
	titlebarMouseWheelOperation( e->delta());
}
#else	
void Client::wheelEvent(QWheelEvent* /*e*/) { ; }
#endif	

void Client::mouseDoubleClickEvent(QMouseEvent *e) {
	if(e->button() != Qt::LeftButton)
		return;
	if(e->pos().y() < bar->height())
		titlebarDblClickOperation();
}


///////////////////////////////////////////////////////////
// painting

// Repaint the window
void Client::paintEvent(QPaintEvent* e) {
	unless(fitzFactoryInitialized()) return;
	//kdDebug()<<"paintEvent() : "<<caption()<<e->rect()<<endl;

	QPainter painter(widget());
	painter.setPen(bgc);
	
	// draw frame
	painter.fillRect(e->rect(),bgc);
	
	//fill in empty space
	if((isPreview() || isShade()) && !dialog) {
		QColor widgetBg = widget()->colorGroup().background();
		QMemArray<QRect> ar = insideMask.rects();
		QMemArray<QRect>::Iterator it;

		//Why isn't there a painter.fillRegion()?
		for(it = ar.begin(); it != ar.end(); ++it) {
			painter.fillRect(*it, widgetBg);
		}
	}

	//avoid flicker by drawing bar now instead of waiting for event loop to send
	//paintEvent to bar.
	barPaintEvent(e);
}


void Client::barPaintEvent(QPaintEvent*) {
	if( !fitzFactoryInitialized()) return;
	
	QPainter painter(bar);
	painter.fillRect(0,0,bar->width(),bar->height(),bgc);
	
	QPoint origin = titleSpace->geometry().topLeft();
	painter.drawPixmap(
		origin.x(), origin.y(), *titleBar, 0, 0,
		titleSpace->geometry().width(), -1
	);
}

// Window is being shown
void Client::showEvent(QShowEvent *)  {
	widget()->update();
	bar->show();
	if(isPreview())
		resizeBar();
}

}

#include "client.moc"
