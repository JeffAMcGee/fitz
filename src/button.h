//////////////////////////////////////////////////////////////////////////////
// button.h
// -------------------
// Fitz window decoration for KDE
// -------------------
// Copyright (c) 2003-2005 by Jeffrey McGee <jeffreym@cs.tamu.edu>
// Portions Copyright (c) 2003 by David Johnson <david@usermode.org>
// 
// You can Freely distribute this program under the GNU General Public
// License. See the file "COPYING" for the exact licensing terms.
//////////////////////////////////////////////////////////////////////////////


#ifndef FITZBUTTON_H
#define FITZBUTTON_H

#include <qbutton.h>
#include "fitz.h"

class KDecoration;

namespace Fitz {

class Button : public QButton {
public:
	Button(QWidget *parent, const char *name, const QString& tip,
			KDecoration* c, BtnType::Type type, BtnImg::Img bitmap);
	~Button();

	void setPixmap(BtnImg::Img i);
	QSize sizeHint() const;
	int lastMousePress() const;
	void reset();

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
};

inline int Button::lastMousePress() const
	{ return lastmouse; }

inline void Button::reset()
	{ repaint(false); }
} // namespace Fitz

#endif // FITZBUTTON_H
