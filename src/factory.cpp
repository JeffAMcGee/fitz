//////////////////////////////////////////////////////////////////////////////
// factory.cpp
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
#include <kconfig.h>
#include <kdebug.h>

//qt
#include <qtimer.h>

#include "fitz.h"
#include "client.h"
#include "factory.h"

#include "buttoncache.h"

namespace Fitz {


bool Factory::initialized_ = false;
bool Factory::autoMax_ = false;

extern "C" KDecorationFactory* create_factory()
{
	return new Fitz::Factory();
}


// Constructor
Factory::Factory() {
	kdDebug()<<"Factory::Factory()"<<endl;
	readConfig();
	initialized_ = true;
	cache= new ButtonCache();
	cache->makePixmaps();
}

// Destructor
Factory::~Factory() {
	initialized_ = false;
	delete cache;
}

// Create the decoration
KDecoration* Factory::createDecoration(KDecorationBridge* b) {
	return new Client(b, this);
}

// Reset the handler. Returns true if decorations need to be remade, false if
// only a repaint is necessary
bool Factory::reset(unsigned long changed) {
	// read in the configuration
	initialized_ = false;
	bool confchange = readConfig();
	initialized_ = true;

	if(changed & SettingColors)
		  cache->makePixmaps();
	
	if (confchange ||
		(changed & (SettingDecoration | SettingButtons | SettingBorder))
	) {
		return true;
	} else {
		resetDecorations(changed);
		return false;
	}
}

// Read in the configuration file
bool Factory::readConfig() {
	// create a config object
	KConfig config("kwinfitzrc");
	config.setGroup("General");

	autoMax_ = config.readBoolEntry("autoMax", false);

	return false;

}

const QPixmap* Factory::getPixmap(BtnType::Type t, int i) {
	return cache->getPixmap(t,i);
}

bool fitzFactoryInitialized() {
	return Factory::initialized_;
}

}
#include "factory.moc"
