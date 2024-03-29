//////////////////////////////////////////////////////////////////////////////
// button.h
// -------------------
// Fitz window decoration for KDE
// -------------------
// Copyright (c) 2003-2005 by Jeffrey McGee <JeffAMcGee@gmail.com>
// Portions Copyright (c) 2003 by David Johnson <david@usermode.org>
// 
// You can Freely distribute this program under the GNU General Public
// License. See the file "COPYING" for the exact licensing terms.
//////////////////////////////////////////////////////////////////////////////


#ifndef FITZBUTTON_H
#define FITZBUTTON_H

#include <QMouseEvent>
#include <QAbstractButton>
#include <QEvent>
#include <QPixmap>
#include "fitz.h"

class KDecoration;

namespace Fitz {

class Button : public QAbstractButton {
	Q_OBJECT
public:
	Button(QWidget *parent, const char *name,
			KDecoration* c, BtnType::Type type, bool act);
	~Button();

	QSize sizeHint() const;
	int lastMousePress() const;
	void reset();
	void paintEvent(QPaintEvent *e);
public slots:
	void setPixmap(int i);
	void setPixmap(bool b);
	void setPixmap(int i, bool act);
	void setActive(bool act);
	void toggle();
signals:
	void toggled(bool on);
private:
	void enterEvent(QEvent *e);
	void leaveEvent(QEvent *e);
	void mousePressEvent(QMouseEvent *e);
	void mouseReleaseEvent(QMouseEvent *e);
	void drawButton(QPainter *painter);

private:
	BtnType::Type type;
	const QPixmap *deco;
	int lastmouse;
	KDecoration *client;
	int state;
	bool active;
};

inline int Button::lastMousePress() const
	{ return lastmouse; }

inline void Button::reset()
	{ repaint(false); }
} // namespace Fitz

#endif // FITZBUTTON_H
