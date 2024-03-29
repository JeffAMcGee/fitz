//////////////////////////////////////////////////////////////////////////////
// buttoncache.h
// -------------------
// Fitz window decoration for KDE
// -------------------
// Copyright (c) 2003-2005 by Jeffrey McGee <JeffAMcGee@gmail.com>
// Portions Copyright (c) 2003 by David Johnson <david@usermode.org>
// 
// You can Freely distribute this program under the GNU General Public
// License. See the file "COPYING" for the exact licensing terms.
//////////////////////////////////////////////////////////////////////////////

#ifndef FITZBUTTONCACHE_H
#define FITZBUTTONCACHE_H

#include <qpixmap.h>
#include "fitz.h"

class QColor;

namespace Fitz {

class ButtonCache{
  public:
	ButtonCache();
	~ButtonCache();
	
	void makePixmaps();
	const QPixmap* getPixmap(BtnType::Type type, int state, bool active);
  private:
	enum {IMG_COUNT = 16};
	QPixmap btns[2][IMG_COUNT];
	bool pixmapsMade;
};

};

#endif
