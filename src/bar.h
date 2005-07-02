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

	void addButtons(const QString& buttons);

	void reposition();
	void paintEvent(QPaintEvent *e);
  /*public slots:
	virtual void show ();
	virtual void hide ();*/
	
  private slots:
	void maxButtonPressed();
	void menuButtonPressed();

  private:
	Factory* factory();

	void addButton(BtnType::Type b, const char *name,
			const char* signal, QObject *recv, const char* slot);
	void addButton(BtnType::Type b, const char *name, bool on,
			const char* slot);
	void doMask();
	
	Button *button[BtnType::COUNT];
	KDecoration *client;
	QBoxLayout *box;
	QRegion oldParent;
	bool toplevel;
	int btnswidth;
};

} // namespace Fitz

#endif // FITZBAR_H

