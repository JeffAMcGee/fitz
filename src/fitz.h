//////////////////////////////////////////////////////////////////////////////
// fitz.h
// -------------------
// Fitz window decoration for KDE
// -------------------
// Copyright (c) 2003-2005 by Jeffrey McGee <jeffreym@cs.tamu.edu>
// Portions Copyright (c) 2003 by David Johnson <david@usermode.org>
// 
// You can Freely distribute this program under the GNU General Public
// License. See the file "COPYING" for the exact licensing terms.
//////////////////////////////////////////////////////////////////////////////

#ifndef FITZ_H
#define FITZ_H

namespace Fitz {

class FitzClient;
class FItzBar;

enum {
	BTN_HEIGHT=12,
	BTN_WIDTH=24,
	FRAMESIZE = 4,
	SPACERSIZE = 4,
	SLANT_WIDTH = (BTN_HEIGHT+FRAMESIZE-4)/2
};

bool fitzFactoryInitialized();

namespace BtnType {
	enum Type {
		HELP=0,
		MAX,
		MIN,
		CLOSE,
		MENU,
		STICKY,
		SHADE,
		ABOVE,
		BELOW,
		RESIZE,
		COUNT
	};
}


} // namespace Fitz

//a spoonful of syntactic sugar makes the Perl hacker happy
#define unless(x) if(!(x))

#endif // FITZ_H
