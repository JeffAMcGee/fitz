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
#include <qtooltip.h>
#include <qcursor.h>
#include <qapplication.h>

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

kdbgstream& operator<<( kdbgstream &kd, const QPointArray &pa ) {
    kd << "[";
	for(unsigned int i=0;i<pa.size();i++) {
		kd << pa.point(i);
		if(i<pa.size())
			kd << ", ";
	}
	kd << "]";
    return kd;
}

namespace Fitz {

///////////////////////////////////////////////////////////
// initialization

// Constructor
Client::Client(KDecorationBridge *b, KDecorationFactory *f) 
	: KDecoration(b, f), event(0) { ; }

Client::~Client() {
	//kdDebug()<<"Client::dtor()"<<endl;
	for (int n=0; n<BtnType::COUNT; n++) {
		if (button[n]) delete button[n];
	}
	if(slow) {
		Display* disp = bar->x11Display();
		XChangePointerControl(disp, do_accel, do_thresh, accel_num, accel_denom, thresh);
	}
	//???
	//delete titleBar;
}

//constants for init()
const char *const  fitzLabel =
"<b><center>Fitz preview</center></b><br />"

"<p><center>Warning: Fitz may work poorly with some programs.  See BUGS for "
"details.</center></p><br />";

int Client::framesize_ = 3;

// Actual initializer for class
void Client::init() {
	createMainWidget(WResizeNoErase | WRepaintNoErase);
	widget()->installEventFilter(this);
	widget()->setBackgroundMode(NoBackground);

	NET::WindowType type = windowType(
			NET::NormalMask | NET::DesktopMask | NET::DockMask |
			NET::ToolbarMask | NET::MenuMask | NET::DialogMask |
			NET::OverrideMask | NET::TopMenuMask |
			NET::UtilityMask | NET::SplashMask
	);
	kdDebug()<<"Client::init() "<<caption()<<" - "<<int(type)<<endl;
	
	if (isPreview()) {
		dialogType = dialog = !isActive();
		barInit();
		
		// setup layout
		QBoxLayout *mainlayout = new QBoxLayout(
			widget(), QBoxLayout::TopToBottom, framesize_, 0
		);
		mainlayout->setResizeMode(QLayout::FreeResize);
		mainlayout->addSpacing(bar->height()-framesize_+4);
		mainlayout->addWidget(
				new QLabel(i18n(fitzLabel),	widget())
		);
		//widget()->addWidget(bar);
	} else {
		dialogType = dialog = (type != NET::Normal);
		barInit();

		//we don't want to reparent until the window exists and qt has started
		//processing the event loop
		QTimer::singleShot(0,this,SLOT(reparent()));
	}
	
	slow = false;

	// setup titlebar buttons
	addButtons(
		options()->titleButtonsLeft() + " " +
		options()->titleButtonsRight()
	);
		
	if(isPreview())
		resizeBar();

	//maximize the window if appropriate
	if(Factory::autoMax() && type == NET::Normal)
		QTimer::singleShot(0,this,SLOT(maximizeFull()));
}

void Client::barInit() {
	bar = new QWidget(widget(), "button bar", 0);
	hiddenTitleWidth = 0;

	bar->setBackgroundMode(NoBackground);
	bar->setSizePolicy(QSizePolicy(QSizePolicy::Fixed,QSizePolicy::Fixed));
	
	box = new QBoxLayout(bar, QBoxLayout::LeftToRight, 2, 0, "Fitz::Bar Layout");

	// setup titlebar buttons
	for (int n=0; n<BtnType::COUNT; n++) {
		button[n] = 0;
	}
	titleSpace = new QSpacerItem(0,BTN_HEIGHT);
	titleBar = new QPixmap;
	
	headSpace = new QSpacerItem(
			headWidth()-2,0,
			QSizePolicy::Fixed,
			QSizePolicy::Minimum
	);
	box->addItem(headSpace);
	
	bar->setMouseTracking(true);
	bar->installEventFilter(this);

	if(isPreview())
		bar->clearMask();
}

//this function finds the parent window of the window decoration widget and
//makes the bar a child of that window
void Client::reparent() {
	Display* disp = bar->x11Display();
	Window barWin = bar->winId();
	Window deco = widget()->winId();
	Window root;
	Window parent;
	Window *children;
	unsigned int num_children;
	
	XQueryTree(disp, deco, &root, &parent, &children, &num_children);
	if (children)
		XFree(children);
	
	XReparentWindow(disp, deco, parent, 0, 0);
	kdDebug()<<"reparent()"<<endl;
	resizeBar();
}

///////////////////////////////////////////////////////////
// button creation

// Add buttons to title layout
void Client::addButtons(const QString& s) {
	//kdDebug()<<"Client::addButtons()"<<endl;
	btnsWidth=0;
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
			btnsWidth+=SPACERSIZE;
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
		btnsWidth+=BTN_WIDTH;
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
	widget()->repaint(false);
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
	btnsWidth+=width-oldWidth;
	
	//force repaint even if resize was a nop
	bar->update();
	
	if(oldWidth) {
		kdDebug()<<"captionChange()"<<endl;
		resizeBar();
	}
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


///////////////////////////////////////////////////////////
// size stuff

// Get the size of the borders
void Client::borders(int &l, int &r, int &t, int &b) const {
	l = r = t = b = framesize_;
	if(isShade())
		t=BTN_HEIGHT+3;
	else if(dialog)
		t=BTN_HEIGHT+4;
	else if(isPreview())
		t=BTN_HEIGHT+8;
}

// Called to resize or move the window
void Client::resize(const QSize &size) {
	//kdDebug()<<"resize()"<<endl;
	widget()->resize(size);
	resizeBar();
}

void Client::resizeBar() {
	//kdDebug()<<"Client::resizeBar() : "<<caption()<<geometry()
	//	<<bar->geometry()<<hiddenTitleWidth<<endl;

	btnsWidth += hiddenTitleWidth;
	int newWidth = width() -barWidth();
	
	if(
		(newWidth<300 && !dialog && !isPreview() ) ||
		(newWidth>400 && dialog && !dialogType && !isPreview() )
	) {
		toggleDialog();
		return;
	}

	if(newWidth <0) {
		//explain this
		hiddenTitleWidth = -newWidth;
		btnsWidth -= hiddenTitleWidth;
	} else if(hiddenTitleWidth) {
		hiddenTitleWidth = 0;
	}

	makeCorners();
	bar->setMask(corners);

	int x = width() -barWidth();
	bar->move(x,0);
	
	QRegion outside(frameGeom());
	QRegion mask(corners);
	mask.translate(x,0);

	QRect inside = frameGeom();
	inside.addCoords(framesize_,framesize_,-framesize_,-framesize_);
	
	if(dialog)
		setMask(mask+outside);
	if(!isPreview())
		widget()->setMask(outside-QRegion(inside)+mask);
	
}

void Client::toggleDialog() {
	dialog=!dialog;
	
	if(!dialog)	clearMask();
	headSpace->changeSize(
		headWidth()-2,0,
		QSizePolicy::Fixed,
		QSizePolicy::Minimum
	);
	box->invalidate();
	
	//tell kwin about our change in borders()
	if( !isSetShade() ) {
		setShade(1);
		setShade(0);
		return;
	}
	return;
}

void Client::makeCorners() {
	//Welcome to the land of magic constants.  I hope you enjoy your stay.
	bar->setFixedSize(
		barWidth(),
		BTN_HEIGHT+tailHeight()+3
	);
	head = QRect(
		0,
		dialog ? 0 : framesize_,
		headWidth(),
		headHeight()
	);
	tail = QRect(
		bar->width() - framesize_ - tailWidth(),
		BTN_HEIGHT+4,
		tailWidth(),
		tailHeight()
	);
	
	if(dialog) {
		corners.putPoints(
			0, 7,
			0,BTN_HEIGHT+3,
			tail.left(), tail.top(),
			tail.right(), tail.bottom(),
			bar->width(), tail.bottom(),
			bar->width(), 0,
			head.right(), head.top(),
			head.left(), head.bottom()
		);
	} else {
		corners.putPoints(
			0, 7,
			0, 1,
			head.left(), head.top(),
			head.right(), head.bottom(),
			tail.left(), tail.top(),
			tail.right(), tail.bottom(),
			bar->width()-1, tail.bottom(),
			bar->width()-1, 1
		);
	}
}

int Client::headHeight() const {
	return BTN_HEIGHT+5-framesize_;
}

int Client::headWidth() const {
	int h = BTN_HEIGHT+5-framesize_;
	return dialog ? (h*2-1) : (h+1)/2 ; 
}

int Client::barWidth() const {
	return btnsWidth +headWidth() -2;
}

QRect Client::frameGeom() const {
	QRect frame = widget()->geometry();
	if(isPreview()) {
		frame.moveTop(0);
		frame.moveLeft(0);
	}
	if(dialog) {
		frame.setTop(headHeight()-1);
	}
	return frame;
}

// Return the minimum allowable size for this decoration
QSize Client::minimumSize() const {
	return QSize(bar->width()-titleBar->width()+hiddenTitleWidth, bar->height());
}

void Client::setBorderSize(BorderSize b) {
	switch(b) {
	  case BorderTiny:
		framesize_ = 1;
		break;
	  case BorderNormal:
		framesize_ = 3;
		break;
	  case BorderLarge:
		framesize_ = 6;
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
}

///////////////////////////////////////////////////////////
// the great event filter

// Event filter
bool Client::eventFilter(QObject *obj, QEvent *e) {
	if (obj == bar)
		return barEventFilter(obj, e);
	if (obj != widget())
		return false;
	//kdDebug()<<"Client::eventFilter("<<e->type()<<") : "<<caption()<<endl;
 
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


// Event filter
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
		//QWheelEvent ( const QPoint & pos, const QPoint & globalPos, int delta, int state, Orientation orient = Vertical )
		break;
			
	  case QEvent::Paint:
		barPaintEvent(static_cast<QPaintEvent *>(e));
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
	
	//kdDebug()<<"Client::mousePosition("<<point<<") : "<<caption()<<endl;

	int x = point.x();
	int y = point.y();
	
	if((bar->geometry().contains(point) && y>1) || isSetShade())
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
	//kdDebug()<<"Client::mousePressEvent("<<e->button()<<","<<e->globalX()<<","<<e->globalY()<<")"<<endl;
	if(
		e->button() == (Qt::MouseButtonMask&Qt::LeftButton) &&
		! (bar->geometry().contains(e->pos()) && e->globalY()>1) &&
		! isSetShade()
	) {
		//this is a left mouse button press that is not inside the button bar
		assert(event == 0);
		event=new QMouseEvent(
				e->type(), e->pos(),
				e->globalPos(),e->button(),e->state()
		);
	} else {
		processMousePressEvent(e);
	}
}

void Client::mouseReleaseEvent(QMouseEvent *e) {
	//kdDebug()<<"Client::mouseReleaseEvent("<<e->button()<<","<<e->globalX()<<","<<e->globalY()<<")"<<endl;

	if(e->button()==(Qt::MouseButtonMask&Qt::LeftButton) && event !=0) {
		delete event;
		event=0;
		
		int x,y,w,h;
		
		if(isPreview()){
			QWidget *d=widget();
			x=e->x();
			y=e->y();
			w=d->width();
			h=d->height();
		} else {
			QWidget *d = QApplication::desktop();
			x=e->globalX();
			y=e->globalY();
			w=d->width();
			h=d->height();
		}
		bool willMove=false;

		if(x<framesize_) { //left
			 x=framesize_+6;
			 willMove=true;
		}  else if((w-x)<=framesize_) {//right
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
	
		if(isPreview()) {
			QPoint p = widget()->mapToGlobal(QPoint(x,y));
			x=p.x();
			y=p.y();
		}
		
		if(willMove) {
			QCursor::setPos(x,y);
			Fitz::FakeMouse::click();
		}
	}
}

void Client::mouseLeaveEvent(QMouseEvent * /*e*/) {
	//kdDebug()<<"Client::mouseLeaveEvent("<<e->globalX()<<","<<e->globalY()<<")"<<endl;
	
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
	//kdDebug()<<"Client::paintEvent() : "<<caption()<<endl;

	//check for color change
	QColor fg=KDecoration::options()->color(KDecoration::ColorFont,isActive());
	QColor bg=KDecoration::options()->color(KDecoration::ColorTitleBar,isActive());
	if(bg!=bgc || fg!=fgc) {
		fgc=fg;
		bgc=bg;
		redrawTitle();
	}
	
	QPainter painter(widget());
	QRect frame = frameGeom();

	// draw frame
	for(int i=0; i<framesize_; i++) {
		/*if(i==0)
			painter.setPen(bgc.dark(150));
		else if(i==framesize_-1)
			painter.setPen(bgc.dark(130));
		else if(i==1)*/
			painter.setPen(bgc);

		painter.drawRect(frame);
		frame.addCoords(1,1,-1,-1);
	}
	
	//fill in empty space
	if(isSetShade()) {
		frame.setRect(framesize_, framesize_, width()-framesize_*2, height()-framesize_*2);
		painter.fillRect(frame, bgc.light(120));
	}

	if(isPreview()) {
		frame.setBottom(bar->height());
		painter.fillRect(frame, widget()->paletteBackgroundColor());
	}
	
	//avoid flicker by drawing bar now instead of waiting for event loop to send
	//paintEvent to bar. this results in two calls to barPaint for every paintEvent. Ohh well.
	barPaintEvent(e);
}


void Client::barPaintEvent(QPaintEvent*) {
	//kdDebug()<<"Client::barPaintEvent() : "
	//	<<caption()<<bar->geometry()<<corners<<endl;
	if( !fitzFactoryInitialized()) return;
	if(corners.isNull()) return;
	
	QPainter painter(bar);
	
	//fill in the empty space
	painter.setPen(bgc);
	painter.setBrush(bgc);
	painter.drawPolygon(corners);

	QPointArray line;

	if(dialog) {
		line.putPoints(
			0, 3,
			0, tail.top()-1,
			tail.left(), tail.top()-1,
			tail.right(), tail.bottom()-1
		);
		
		if(bar->x() < framesize_-1) {
			line.setPoint(0,framesize_-bar->x()-1,tail.top()-1);
		}
		
		painter.setPen(bgc.dark(130));
		//painter.drawPolyline(line);
		
		line.putPoints(
			0, 4,
			bar->width()-1, tail.bottom(),
			bar->width()-1, 0,
			head.right()+1, head.top(),
			head.left(), head.bottom()
		);
		
		painter.setPen(bgc.dark(150));
		//painter.drawPolyline(line);
		
		if(bar->x() == 0) {
			painter.drawLine(0,head.bottom(),0,tail.top()-1);
		}
	} else {
		line.putPoints(0,4,corners,1);
		line.translate(0,-1);
		painter.setPen(bgc.dark(130));
		//painter.drawPolyline(line);
	}
	
	QPoint origin = titleSpace->geometry().topLeft();
	painter.drawPixmap(
		origin.x(), origin.y(), *titleBar, 0, 0,
		titleBar->width()-hiddenTitleWidth, -1
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
