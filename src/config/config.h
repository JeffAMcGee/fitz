//////////////////////////////////////////////////////////////////////////////
// config.h
// -------------------
// Fitz window decoration for KDE
// -------------------
// Copyright (c) 2003-2005 by Jeffrey McGee <jeffreym@cs.tamu.edu>
// Portions Copyright (c) 2003 by David Johnson <david@usermode.org>
// 
// You can Freely distribute this program under the GNU General Public
// License. See the file "COPYING" for the exact licensing terms.
//////////////////////////////////////////////////////////////////////////////


#ifndef FITZCONFIG_H
#define FITZCONFIG_H

#include <qobject.h>

class KConfig;
class ConfigDialog;

class Config : public QObject
{
	Q_OBJECT
public:
	Config(KConfig* config, QWidget* parent);
	~Config();
	
signals:
	void changed();

public slots:
	void load(KConfig*);
	void save(KConfig*);
	void defaults();

private:
	KConfig *config;
	ConfigDialog *dialog;
};

#endif // FITZCONFIG_H
