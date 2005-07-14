//////////////////////////////////////////////////////////////////////////////
// fakemouse.h
// -------------------
// Fitz window decoration for KDE
// -------------------
// Copyright (c) 2003-2005 by Jeffrey McGee <JeffAMcGee@gmail.com>
// Portions Copyright (c) 2003 by David Johnson <david@usermode.org>
// 
// You can Freely distribute this program under the GNU General Public
// License. See the file "COPYING" for the exact licensing terms.
//////////////////////////////////////////////////////////////////////////////

#ifndef FITZFAKEMOUSE_H
#define FITZFAKEMOUSE_H

//qt
//#include <qpoint.h>

namespace Fitz {
namespace FakeMouse{

enum Btn {LEFT=1, MID, RIGHT};

//push the button down or let go
void push(Btn b=LEFT, bool down=true);

//generate a click (an up and a down)
void click(Btn b=LEFT, bool doubleclick=false);

//move the mouse by (dx, dy)
void move(int dx, int dy);
//void move(const QPoint &d) {move(d.x(),d.y())};

};
};

#endif
