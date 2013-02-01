#!/usr/bin/python
# -*- coding: utf-8 -*-
#
# Licensed under the GNU General Public License, version 2.
# See the file http://www.gnu.org/copyleft/gpl.txt.

from pisi.actionsapi import autotools
from pisi.actionsapi import pisitools
from pisi.actionsapi import get

def build():
    autotools.make("-j1")

def install():
    pisitools.insinto("/usr/bin", "pommed/")
    pisitools.insinto("/etc", "pommed.conf.mactel")
    pisitools.insinto("/etc", "pommed.conf.pmac")
    pisitools.insinto("/etc/dbus-1/system.d", "dbus-policy.conf", "pommed.conf")
    # Man page
    pisitools.insinto("/usr/share/man/man1", "pommed.1")
    # Sounds
    pisitools.insinto("/usr/share/pommed", "pommed/data/*")
    #gpomme
    pisitools.insinto("/usr/bin", "gpomme")
    pisitools.insinto("/usr/share/gpomme/themes", "gpomme/themes/*")
    pisitools.insinto("/usr/share/applications", "gpomme/gpomme*.desktop")
    pisitools.insinto("/usr/share/icons/hicolor", "icons/gpomme.svg", "gpomme.svg")
    #wmpomme
    pisitools.insinto("/usr/bin", "wmpomme")
    pisitools.insinto("/usr/share/pixmaps", "icons/gpomme_32x32.xpm", "wmpomme.xpm")
    

    pisitools.dodoc("ChangeLog", "COPYING", "README")
