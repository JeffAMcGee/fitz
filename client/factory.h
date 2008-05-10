//////////////////////////////////////////////////////////////////////////////
// factory.h
// -------------------
// Fitz window decoration for KDE
// -------------------
// Copyright (c) 2003-2005 by Jeffrey McGee <JeffAMcGee@gmail.com>
// Portions Copyright (c) 2003 by David Johnson <david@usermode.org>
// 
// You can Freely distribute this program under the GNU General Public
// License. See the file "COPYING" for the exact licensing terms.
//////////////////////////////////////////////////////////////////////////////


#ifndef FITZFACTORY_H
#define FITZFACTORY_H

#include <kdecorationfactory.h>
#include <q3valuelist.h>
#include <qobject.h>
//Added by qt3to4:
#include <QPixmap>

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
	virtual bool supports( Ability ability ) const;
	virtual QList< KDecoration::BorderSize > borderSizes() const;

	static bool autoMax();
	
	const QPixmap* getPixmap(BtnType::Type t, int i, bool active);
	
  private:
	bool readConfig();
	friend bool fitzFactoryInitialized();

	static bool initialized_;
	static bool autoMax_;
	ButtonCache *cache;
};

inline bool Factory::autoMax()
	{ return autoMax_; }


} // namespace Fitz

#endif // FITZFACTORY_H
