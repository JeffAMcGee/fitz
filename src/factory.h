//////////////////////////////////////////////////////////////////////////////
// factory.h
// -------------------
// Fitz window decoration for KDE
// -------------------
// Copyright (c) 2003-2005 by Jeffrey McGee <jeffreym@cs.tamu.edu>
// Portions Copyright (c) 2003 by David Johnson <david@usermode.org>
// 
// You can Freely distribute this program under the GNU General Public
// License. See the file "COPYING" for the exact licensing terms.
//////////////////////////////////////////////////////////////////////////////


#ifndef FITZFACTORY_H
#define FITZFACTORY_H

#include <kdecorationfactory.h>
#include <qvaluelist.h>
#include <qobject.h>

class QTimer;

namespace Fitz {

class Bar;
class ButtonCache;

class Factory: public QObject, public KDecorationFactory {
	Q_OBJECT
  public:
	Factory();
	virtual ~Factory();
	virtual KDecoration *createDecoration(KDecorationBridge *b);
	virtual bool reset(unsigned long changed);

	static bool autoMax();
	
	const QPixmap* getPixmap(BtnImg::Img i);
	
	void updateRegion(const QRect &r);
	void addBar(Bar* bar);
	void delBar(Bar* bar);
  private slots:
	void cleanRegion();
  private:
	QRect dirty;
	QTimer *delay;
	
	bool readConfig();
	friend bool fitzFactoryInitialized();

	static bool initialized_;
	static bool autoMax_;
	QPtrList<Bar> list;
	ButtonCache *cache;
};

inline bool Factory::autoMax()
	{ return autoMax_; }


} // namespace Fitz

#endif // FITZFACTORY_H
