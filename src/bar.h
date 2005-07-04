//////////////////////////////////////////////////////////////////////////////
// bar.h
// -------------------
// Fitz window decoration for KDE
// -------------------
// Copyright (c) 2003-2005 by Jeffrey McGee <jeffreym@cs.tamu.edu>
// Portions Copyright (c) 2003 by David Johnson <david@usermode.org>
// 
// You can Freely distribute this program under the GNU General Public
// License. See the file "COPYING" for the exact licensing terms.
//////////////////////////////////////////////////////////////////////////////


#ifndef FITZBAR_H
#define FITZBAR_H

#include <qwidget.h>
#include "fitz.h"

class KDecoration;
class QBoxLayout;
class QSpacerItem;
class QPixmap;

namespace Fitz {

class Button;
class Factory;

class Bar : public QWidget {
	Q_OBJECT
  public:
	Bar(KDecoration *parent=0, const char *name=0, bool topLevel=true);
	virtual ~Bar();

	void activeChange(bool active);
	void desktopChange(bool onAllDesktops);
	void maximizeChange(bool maximizeMode);
	void captionChange(const QString& caption);

	void addButtons(const QString& buttons);

	void reposition();
	void paintEvent(QPaintEvent *e);
	
  private slots:
	virtual void maxButtonPressed();
	virtual void menuButtonPressed();

  private:
	Factory* factory();
	bool eventFilter(QObject *obj, QEvent *e);

	void addButton(BtnType::Type b, const char *name,
			const char* signal, QObject *recv, const char* slot);
	void addButton(BtnType::Type b, const char *name, bool on,
			const char* slot);
	void calcSize();
	
	Button *button[BtnType::COUNT];
	KDecoration *client;
	QBoxLayout *box;
	bool toplevel;
	int btnsWidth;
	int slantWidth;
	QPointArray corners;
	QSpacerItem *titleSpace;
	QPixmap *titleBar;
};

} // namespace Fitz

#endif // FITZBAR_H

