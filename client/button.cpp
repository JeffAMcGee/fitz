//////////////////////////////////////////////////////////////////////////////
// button.cpp
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
#include <kdecoration.h>
#include <kdebug.h>

//qt
#include <qbitmap.h>
#include <qlabel.h>
#include <qpainter.h>
#include <qtooltip.h>
//Added by qt3to4:
#include <QEvent>
#include <QMouseEvent>

//fitz
#include "fitz.h"
#include "factory.h"
#include "button.h"

namespace Fitz {


// Constructor
Button::Button(QWidget *parent, const char *name,
		KDecoration* c, BtnType::Type t, bool act)
	: QAbstractButton(parent, name), type(t),
		deco(0), lastmouse(0), client(c), state(0)
{
	setBackgroundMode(Qt::NoBackground);
	setFixedSize(BTN_WIDTH, BTN_HEIGHT);
	setCursor(Qt::arrowCursor);
	setPixmap(0,act);
	QToolTip::add(this, i18n(name));
}

Button::~Button() {
}

void Button::toggle() {
	state=!state;
	setPixmap(state);
	emit toggled(state);
}

void Button::setPixmap(bool b) {
	setPixmap(int(b),active);
}

void Button::setPixmap(int i) {
	setPixmap(i,active);
}

void Button::setActive(bool act) {
	setPixmap(state,act);
}

// Set the button decoration
void Button::setPixmap(int i, bool act) {
	Factory *f=static_cast<Factory*>(client->factory());
	state=i;
	active=act;
	deco=f->getPixmap(type,i,act);
	repaint(false);
}

// Return size hint
QSize Button::sizeHint() const {
	return QSize(BTN_WIDTH, BTN_HEIGHT);
}

// Mouse has entered the button
void Button::enterEvent(QEvent *e) {
	// if we wanted to do mouseovers, we would keep track of it here
	QAbstractButton::enterEvent(e);
}

// Mouse has left the button
void Button::leaveEvent(QEvent *e) {
	// if we wanted to do mouseovers, we would keep track of it here
	QAbstractButton::leaveEvent(e);
}

// Button has been pressed
void Button::mousePressEvent(QMouseEvent* e) {
	lastmouse = e->button();

	// translate and pass on mouse event
	int button = Qt::LeftButton;
	if ((type != BtnType::MAX) && (e->button() != Qt::LeftButton)) {
		button = Qt::NoButton; // middle & right buttons inappropriate
	}
	QMouseEvent me(e->type(), e->pos(), e->globalPos(),
				   button, e->state());
	QAbstractButton::mousePressEvent(&me);
}

// Button has been released
void Button::mouseReleaseEvent(QMouseEvent* e) {
	lastmouse = e->button();

	// translate and pass on mouse event
	int button = Qt::LeftButton;
	if ((type != BtnType::MAX) && (e->button() != Qt::LeftButton)) {
		button = Qt::NoButton; // middle & right buttons inappropriate
	}
	QMouseEvent me(e->type(), e->pos(), e->globalPos(), button, e->state());
	QAbstractButton::mouseReleaseEvent(&me);
}

void Button::paintEvent(QPaintEvent *) {
	QPainter painter(this);
	drawButton(&painter);
}

// Draw the button
void Button::drawButton(QPainter *painter) {
	unless(fitzFactoryInitialized()) return;
	painter->drawPixmap(0,0, *deco);
}

}

#include "button.moc"
