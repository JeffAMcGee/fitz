//////////////////////////////////////////////////////////////////////////////
// button.cpp
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
#include <kdecoration.h>
#include <kdebug.h>

//qt
#include <qbitmap.h>
#include <qlabel.h>
#include <qpainter.h>
#include <qtooltip.h>

//fitz
#include "fitz.h"
#include "factory.h"
#include "button.h"

namespace Fitz {


// Constructor
Button::Button(QWidget *parent, const char *name,
		const QString& tip, KDecoration* c, BtnType::Type t,
		BtnImg::Img bitmap)
	: QButton(parent, name), type(t),
		deco(0), lastmouse(0), client(c)
{
	setBackgroundMode(NoBackground);
	setFixedSize(BTN_WIDTH, BTN_HEIGHT);
	setCursor(arrowCursor);
	setPixmap(bitmap);
	QToolTip::add(this, tip);
}

Button::~Button() {
}

// Set the button decoration
void Button::setPixmap(BtnImg::Img i) {
	Factory *f=static_cast<Factory*>(client->factory());
	deco=f->getPixmap(i);
	repaint(false);
}

// Return size hint
QSize Button::sizeHint() const {
	return QSize(BTN_WIDTH, BTN_HEIGHT);
}

// Mouse has entered the button
void Button::enterEvent(QEvent *e) {
	// if we wanted to do mouseovers, we would keep track of it here
	QButton::enterEvent(e);
}

// Mouse has left the button
void Button::leaveEvent(QEvent *e) {
	// if we wanted to do mouseovers, we would keep track of it here
	QButton::leaveEvent(e);
}

// Button has been pressed
void Button::mousePressEvent(QMouseEvent* e) {
	lastmouse = e->button();

	// translate and pass on mouse event
	int button = LeftButton;
	if ((type != BtnType::MAX) && (e->button() != LeftButton)) {
		button = NoButton; // middle & right buttons inappropriate
	}
	QMouseEvent me(e->type(), e->pos(), e->globalPos(),
				   button, e->state());
	QButton::mousePressEvent(&me);
}

// Button has been released
void Button::mouseReleaseEvent(QMouseEvent* e) {
	lastmouse = e->button();

	// translate and pass on mouse event
	int button = LeftButton;
	if ((type != BtnType::MAX) && (e->button() != LeftButton)) {
		button = NoButton; // middle & right buttons inappropriate
	}
	QMouseEvent me(e->type(), e->pos(), e->globalPos(), button, e->state());
	QButton::mouseReleaseEvent(&me);
}

// Draw the button
void Button::drawButton(QPainter *painter) {
	unless(fitzFactoryInitialized()) return;

	QColorGroup group;
	
	// paint a plain box with border
	group = KDecoration::options()->
		colorGroup(KDecoration::ColorButtonBg, client->isActive());
	painter->fillRect(rect(), group.button());
	/*painter->setPen(group.dark());
	painter->drawRect(rect());*/

	painter->drawPixmap(0,0, *deco);
}

}

//#include "button.moc"
