//////////////////////////////////////////////////////////////////////////////
// client.h
// -------------------
// Fitz window decoration for KDE
// -------------------
// Copyright (c) 2003-2005 by Jeffrey McGee <jeffreym@cs.tamu.edu>
// Portions Copyright (c) 2003 by David Johnson <david@usermode.org>
// 
// You can Freely distribute this program under the GNU General Public
// License. See the file "COPYING" for the exact licensing terms.
//////////////////////////////////////////////////////////////////////////////


#ifndef FITZCLIENT_H
#define FITZCLIENT_H

#include <kdecoration.h>

namespace Fitz {

class Bar;

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
	virtual void resize(const QSize &size);
	virtual QSize minimumSize() const;
	virtual Position mousePosition(const QPoint &point) const;

	
  private slots:
	virtual void maximizeFull();
	virtual void reparentBar();

  private:
	bool eventFilter(QObject *obj, QEvent *e);

	void mousePressEvent(QMouseEvent *);
	void mouseReleaseEvent(QMouseEvent *);
	void mouseLeaveEvent(QMouseEvent *);
	void mouseDoubleClickEvent(QMouseEvent *e);
	void wheelEvent(QWheelEvent *e);

	void moveEvent(QMoveEvent *);
	void paintEvent(QPaintEvent *e);
	void resizeEvent(QResizeEvent *);
	void showEvent(QShowEvent *);
	//void hideEvent(QHideEvent *);
	
	QMouseEvent *event;
	Bar *bar;
	QRect oldGeom;
};

} // namespace Fitz

#endif // FITZCLIENT_H
