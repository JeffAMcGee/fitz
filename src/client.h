//////////////////////////////////////////////////////////////////////////////
// client.h
// -------------------
// Fitz window decoration for KDE
// -------------------
// Copyright (c) 2003-2005 by Jeffrey McGee <JeffAMcGee@gmail.com>
// Portions Copyright (c) 2003 by David Johnson <david@usermode.org>
// 
// You can Freely distribute this program under the GNU General Public
// License. See the file "COPYING" for the exact licensing terms.
//////////////////////////////////////////////////////////////////////////////


#ifndef FITZCLIENT_H
#define FITZCLIENT_H

#include <kdecoration.h>
#include <qpointarray.h>
#include "fitz.h"

class QBoxLayout;
class QSpacerItem;
class QPixmap;
class QWidget;

namespace Fitz {

class Button;
class Factory;

class Client : public KDecoration {
	Q_OBJECT
  public:
	Client(KDecorationBridge *b, KDecorationFactory *f);
	virtual ~Client();

	virtual void init();

	virtual void activeChange();
	virtual void desktopChange();
	virtual void captionChange();
	virtual void iconChange();
	virtual void maximizeChange();
	virtual void shadeChange();

	virtual void borders(int &l, int &r, int &t, int &b) const;
	void reposition();
	virtual void resize(const QSize &size);
	virtual void resizeBar();
	virtual void resizeTitleBar();
	virtual QSize minimumSize() const;
	virtual Position mousePosition(const QPoint &point) const;
	
	void addButtons(const QString& buttons);
	static void setBorderSize(BorderSize b);
	
  signals:
	void activeChanged(bool act);

  private slots:
	virtual void maximizeFull();
	virtual void reparent();
	virtual void maxButtonPressed();
	virtual void menuButtonPressed();
	virtual void resizeButtonPressed();

  private:
	void barInit();
	bool eventFilter(QObject *obj, QEvent *e);
	bool barEventFilter(QObject *obj, QEvent *e);
	int redrawTitle();
	
	void addButton(BtnType::Type b, const char *name,
			const char* signal, const char* slot);
	void addButton(BtnType::Type b, const char *name, bool on,
			const char* slot);

	void mousePressEvent(QMouseEvent *);
	void mouseReleaseEvent(QMouseEvent *);
	void mouseLeaveEvent(QMouseEvent *);
	void mouseDoubleClickEvent(QMouseEvent *e);
	void wheelEvent(QWheelEvent *e);

	void moveEvent(QMoveEvent *);
	void paintEvent(QPaintEvent *e);
	void barPaintEvent(QPaintEvent *e);
	void showEvent(QShowEvent *);
	//void hideEvent(QHideEvent *);
	
	int headHeight() const;
	int headWidth() const;
	int tailHeight() const {return 16;}
	int tailWidth() const {return 8;}
	QRect frameGeom() const;
		
	QMouseEvent *event;
	QWidget *bar;
	QRect oldGeom;
	
	//from bar
	Button *button[BtnType::COUNT];
	QBoxLayout *box;
	bool reparented;
	bool dialog;
	bool dialogType;
	int btnsWidth;
	int hiddenTitleWidth;
	QPointArray corners;
	QRect head;
	QRect tail;
	QSpacerItem *headSpace;
	QSpacerItem *titleSpace;
	QPixmap *titleBar;
	QColor bgc;
	QColor fgc;
	static int framesize_;
	
	
	//for gravity
	bool do_accel;
	bool do_thresh;
	bool slow;
	int accel_num;
	int accel_denom;
	int thresh;
};

} // namespace Fitz

#endif // FITZCLIENT_H
