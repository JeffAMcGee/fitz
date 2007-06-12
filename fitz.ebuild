# Copyright 1999-2005 Gentoo Foundation
# Distributed under the terms of the GNU General Public License v2
# $Header: /cvsroot/fitz/fitz/fitz.ebuild,v 1.1 2005/12/21 00:34:35 nerd4christ Exp $
inherit kde

need-kde 3.2

DESCRIPTION="A window decoration for KDE that allows you to work (or play) faster."
HOMEPAGE="http://fitz.sourceforge.net"
SRC_URI="mirror://sourceforge/fitz/fitz-${PV}.tar.bz2"

LICENSE="GPL-2"
SLOT="$KDEMAJORVER.$KDEMINORVER"
KEYWORDS="x86 ~amd64 ~sparc ~ppc"
IUSE=""

DEPEND=">=kde-base/kwin-3.2"
RDEPEND=">=kde-base/kwin-3.2"
