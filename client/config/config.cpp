//////////////////////////////////////////////////////////////////////////////
// config.cpp
// -------------------
// Fitz window decoration for KDE
// -------------------
// Copyright (c) 2003-2005 by Jeffrey McGee <JeffAMcGee@gmail.com>
// Portions Copyright (c) 2003 by David Johnson <david@usermode.org>
// 
// You can Freely distribute this program under the GNU General Public
// License. See the file "COPYING" for the exact licensing terms.
//////////////////////////////////////////////////////////////////////////////

//kde
#include <kconfig.h>
#include <klocale.h>
#include <kglobal.h>

//qt
#include <qcheckbox.h>

//fitz
#include "config.h"
#include "ui_configdialog.h"

class ConfigDialogWidget : public QWidget, public Ui::ConfigDialog {
public:
	ConfigDialogWidget( QWidget *parent ) : QWidget( parent ) {
		setupUi( this );
	}
};

// Constructor
Config::Config(KConfig* /*config*/, QWidget* parent)
	: QObject(parent), config(0), dialog(0)
{
	// create the configuration object
	config = new KConfig("kwinfitzrc");
	KGlobal::locale()->insertCatalog("kwin_fitz_config");

	// create and show the configuration dialog
	dialog = new ConfigDialogWidget(parent);
	dialog->show();


	// load the configuration
	load(config);

	// setup the connections
	connect(dialog->autoMaxCheckbox, SIGNAL(clicked()),
			this, SIGNAL(changed()));

}

// Destructor
Config::~Config() {
	if (dialog) delete dialog;
	if (config) delete config;
}

// Load configuration data
void Config::load(KConfig*) {
	KConfigGroup cg(config, "General");
	
	bool max = cg.readEntry("autoMax", false);
	dialog->autoMaxCheckbox->setChecked(max);
}

// Save configuration data
void Config::save(KConfig*) {
	KConfigGroup cg(config, "General");
	
	bool max = dialog->autoMaxCheckbox->isChecked();
	cg.writeEntry("autoMax", max);
	
	config->sync();
}


// Set configuration defaults
void Config::defaults()  {
	dialog->autoMaxCheckbox->setChecked(false);
}


// Plugin Stuff
extern "C" {
	KDE_EXPORT QObject* allocate_config(KConfig* config, QWidget* parent) {
		return (new Config(config, parent));
	}
}

#include "config.moc"
