//////////////////////////////////////////////////////////////////////////////
// fakemouse.cpp
// -------------------
// Fitz window decoration for KDE
// -------------------
// Copyright (c) 2003-2005 by Jeffrey McGee <JeffAMcGee@gmail.com>
// Portions Copyright (c) 2003 by David Johnson <david@usermode.org>
// 
// You can Freely distribute this program under the GNU General Public
// License. See the file "COPYING" for the exact licensing terms.
//////////////////////////////////////////////////////////////////////////////

//qt
#include <qpaintdevice.h>

//kde
#include <kdebug.h>

//X11
#include <X11/Xlib.h>
#include <X11/extensions/XTest.h>

//X defines this, which causes problems for fitz.h
#undef COUNT

//fitz
#include "fitz.h"
#include "fakemouse.h"


namespace Fitz {
namespace FakeMouse {


//push the button down or let go
void push(Btn b, bool down) {
	Display *display = QPaintDevice::x11AppDisplay();
	XTestFakeButtonEvent(display, b, down, 0L);
}

//generate a click
void click(Btn b, bool doubleclick) {
	Display *display = QPaintDevice::x11AppDisplay();
	XTestFakeButtonEvent(display, b, true, 0L);
	XTestFakeButtonEvent(display, b, false, 100L);
	unless(doubleclick) return;
	XTestFakeButtonEvent(display, b, true, 200L);
	XTestFakeButtonEvent(display, b, false, 300L);
}

//Does not work
//move the mouse by (dx, dy)
void move(int dx, int dy) {
	Display *display = QPaintDevice::x11AppDisplay();
	XWarpPointer(
			display,None,None,
			0,0,0,0,
			dx,dy
	);
}

};
};
