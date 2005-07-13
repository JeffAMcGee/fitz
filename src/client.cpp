//////////////////////////////////////////////////////////////////////////////
// client.cpp
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
#include <kdebug.h>

//qt
#include <qlabel.h>
#include <qlayout.h>
#include <qtimer.h>
#include <qpainter.h>
#include <qtooltip.h>
#include <qcursor.h>
#include <qapplication.h>

//fitz
#include "fitz.h"
#include "client.h"
#include "bar.h"
#include "factory.h"
#include "fakemouse.h"

//C
#include <assert.h>

namespace Fitz {

const int DELAY_LENGTH=200;
const char *const  fitzLabel =
"<b><center>Fitz preview</center></b><br />"

"<p><center>Warning: This is an alpha release of Fitz.  It may work poorly "
"with some programs.  See BUGS for details.</center></p><br />"

"<p><center>If you click on the frame of a maximized window, the mouse will "
"jump.  You can click this frame for a demonstration.<center></p>";

// Constructor
Client::Client(KDecorationBridge *b, KDecorationFactory *f) 
	: KDecoration(b, f), event(0) { ; }

Client::~Client()
{ ; }

// Actual initializer for class
void Client::init() {
	createMainWidget(WResizeNoErase | WRepaintNoErase);
	widget()->installEventFilter(this);

	// for flicker-free redraws
	widget()->setBackgroundMode(NoBackground);

	// setup layout
	QGridLayout *mainlayout = new QGridLayout(widget(), 4, 3); // 4x3 grid

	mainlayout->setResizeMode(QLayout::FreeResize);
	mainlayout->addRowSpacing(0, FRAMESIZE);
	mainlayout->addRowSpacing(3, FRAMESIZE);
	mainlayout->addColSpacing(0, FRAMESIZE);
	mainlayout->addColSpacing(2, FRAMESIZE);

	// the window should stretch
	mainlayout->setRowStretch(2, 10);
	mainlayout->setColStretch(1, 10);
	
	NET::WindowType type = windowType(
			NET::NormalMask | NET::DesktopMask | NET::DockMask |
			NET::ToolbarMask | NET::MenuMask | NET::DialogMask |
			NET::OverrideMask | NET::TopMenuMask |
			NET::UtilityMask | NET::SplashMask
	);
	kdDebug()<<"Client::init() "<<caption()<<" - "<<int(type)<<endl;
	
	if (isPreview()) {
		//preview window
		mainlayout->addWidget(
				new QLabel(i18n(fitzLabel),
				widget()), 2,1
		);
		mainlayout->addRowSpacing(1,0);
		bar=new Bar(this, "preview button bar", false);
	/*} else if(type != NET::Normal) {{
		//dialog box
		mainlayout->addItem(new QSpacerItem(0, 0), 2, 1);
		mainlayout->addRowSpacing(1,0);
		bar=new Bar(this, "dialog button bar", false);*/
	} else {
		//normal window
		mainlayout->addRowSpacing(1,0);
		bar=new Bar(this, "button bar", true);
		QTimer::singleShot(0,bar,SLOT(reparent()));
	}

	// setup titlebar buttons
	bar->addButtons(options()->titleButtonsLeft()+" "+options()->titleButtonsRight());

	//maximize the window if appropriate
	if(Factory::autoMax() && type == NET::Normal)
		QTimer::singleShot(0,this,SLOT(maximizeFull()));
}

void Client::maximizeFull() {
	QWidget *w=widget();
	if(w->width()>500 && w->height()>400)
		maximize(KDecoration::MaximizeFull);
}

// window active state has changed
void Client::activeChange() {
	bar->activeChange(isActive());
	widget()->repaint(false);
}

// Called when desktop/sticky changes
void Client::desktopChange() {
	bar->desktopChange(isOnAllDesktops());
}

// The icon has changed, but I don't care.
void Client::iconChange()
{ ; }

// The title has changed, but there is no title
void Client::captionChange() {
	bar->captionChange(caption());
}

// Maximized state has changed
void Client::maximizeChange() {
	bar->maximizeChange(maximizeMode());
}

// The window has been shaded or unshaded
void Client::shadeChange() {
	kdDebug()<<" Client::shadeChange() : window shading does not work yet"<<endl;
	//if(isShade()) setShade(false);
}

// Get the size of the borders
void Client::borders(int &l, int &r, int &t, int &b) const {
	l = r = t = b = FRAMESIZE;
}

// Called to resize or move the window
void Client::resize(const QSize &size) {
	widget()->resize(size);
}

// Return the minimum allowable size for this decoration
QSize Client::minimumSize() const {
	return QSize(bar->width()+FRAMESIZE*2+20, FRAMESIZE*2+20);
}

// Return logical mouse position
KDecoration::Position Client::mousePosition(const QPoint &point) const {
	const int corner = 32;
	Position pos = PositionCenter;
	
	kdDebug()<<"Client::mousePosition("<<point<<") : "<<caption()<<endl;

	int x = point.x();
	int y = point.y();
	
	if(bar->geometry().contains(point))
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

// Event filter
bool Client::eventFilter(QObject *obj, QEvent *e) {
	if (obj != widget()) return false;
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
		resizeEvent(static_cast<QResizeEvent *>(e));
		return true;
	  case QEvent::Show:
		showEvent(static_cast<QShowEvent *>(e));
		return true;
		
	  default:
		return false;
	}

	return false;
}

// Deal with mouse click
void Client::mousePressEvent(QMouseEvent *e) {
	//kdDebug()<<"Client::mousePressEvent("<<e->button()<<","<<e->globalX()<<","<<e->globalY()<<")"<<endl;
	if(
		e->button() == (Qt::MouseButtonMask&Qt::LeftButton) &&
		! bar->geometry().contains(e->pos())
	) {
		//this is a left mous button press that is not inside the button bar
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

	if(e->button()==(Qt::MouseButtonMask&Qt::LeftButton)  && event !=0) {
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

		if(x<FRAMESIZE) { //left
			 x=FRAMESIZE+6;
			 willMove=true;
		}  else if((w-x)<=FRAMESIZE) {//right
			x=w-FRAMESIZE-6;
			willMove=true;
		}
		if(y<FRAMESIZE) { //top
			y=FRAMESIZE+6;
			willMove=true;
		} else if((h-y)<=FRAMESIZE) {//bottom
			y=h-FRAMESIZE-6;
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

void Client::mouseLeaveEvent(QMouseEvent *) {
	if(event!=0) {
		processMousePressEvent(event);
		delete event;
		event=0;
	}
}

void Client::wheelEvent(QWheelEvent *e) {
#if KDE_IS_VERSION( 3, 4, 89 )
	titlebarMouseWheelOperation( e->delta());
#endif	
}

void Client::mouseDoubleClickEvent(QMouseEvent *e) {
	if(e->button() != Qt::LeftButton)
		return;
	if(e->pos().y() < bar->height())
		titlebarDblClickOperation();
}

// Repaint the window
void Client::paintEvent(QPaintEvent* e) {
	unless(fitzFactoryInitialized()) return;

	QRect rect = e->rect(); //rect is relative to the widget
	QRect cli = geometry(); //cli is r t screen
	rect.moveBy(cli.left(),cli.top()); //geom is now r t screen

	QColorGroup group;
	QPainter painter(widget());

	// draw frame
	group = options()->colorGroup(KDecoration::ColorTitleBar, true);

	QRect frame(0, 0, width(), FRAMESIZE);
	painter.fillRect(frame, group.background());
	frame.setRect(0, 0, FRAMESIZE, height());
	painter.fillRect(frame, group.background());
	frame.setRect(0, height() - FRAMESIZE, width(), FRAMESIZE);
	painter.fillRect(frame, group.background());
	frame.setRect(width()-FRAMESIZE, 0, FRAMESIZE, height());
	painter.fillRect(frame, group.background());

	// outline the frame
	painter.setPen(group.dark());
	frame = widget()->rect();

	painter.drawRect(frame);
	frame.setRect(frame.x() + FRAMESIZE-1, frame.y() + FRAMESIZE-1,
			width() - FRAMESIZE*2 +2,
			frame.height() - FRAMESIZE*2 +2);
	painter.drawRect(frame);
}

// Window is being resized
void Client::resizeEvent(QResizeEvent *)  {
	if (widget()->isShown()) {
		widget()->erase(widget()->rect());
	}
	bar->reposition();
}

// Window is being shown
void Client::showEvent(QShowEvent *)  {
	//widget()->repaint();
	widget()->update();
	bar->show();
	bar->reposition();
}

/*void Client::hideEvent(QHideEvent *)  {
	//BAR::DO_MASK kdDebug()<<"Client::HideEvent() : "<<caption()<<endl;
	bar->hide();
}*/

}

#include "client.moc"
