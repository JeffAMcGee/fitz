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
#include "bar.h"
#include "client.h"
#include "factory.h"

#include "buttoncache.h"

namespace Fitz {

const int DELAY_LENGTH=10;

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
	delay=new QTimer(this,"Bar refresh delay");
	connect(delay,SIGNAL(timeout()),SLOT(cleanRegion()));
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

void Factory::addBar(Bar* bar) {
	list.append(bar);
}

void Factory::delBar(Bar* bar) {
	list.removeRef(bar);
}

void Factory::updateRegion(const QRect &r) {
	//kdDebug()<<"Factory::updateRegion("<<r<<")"<<endl;
	dirty = dirty.unite(r);
	unless(delay->isActive())
		delay->start(DELAY_LENGTH,true);
}

void Factory::cleanRegion() {
	//kdDebug()<<"Factory::cleanRegion("<<dirty<<")"<<endl;
	Bar *bar;
	for(bar=list.first();bar;bar =list.next()) {
		unless(dirty.intersect(bar->geometry()).isNull())
			bar->doMask(false);
	}
	dirty=QRect();
}

const QPixmap* Factory::getPixmap(BtnImg::Img i) {
	return cache->getPixmap(i);
}

bool fitzFactoryInitialized() {
	return Factory::initialized_;
}

}
#include "factory.moc"
