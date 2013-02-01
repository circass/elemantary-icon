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
    pisitools.insinto("/usr/sbin", "/var/pisi/pommed-*/work/pommed-*/pommed/")
    pisitools.insinto("/etc", "/var/pisi/pommed-*/work/pommed-*/pommed.conf.mactel")
    pisitools.insinto("/etc", "/var/pisi/pommed-*/work/pommed-*/pommed.conf.pmac")
    pisitools.insinto("/etc/dbus-1/system.d", "/var/pisi/pommed-*/work/pommed-*/dbus-policy.conf", "pommed.conf")
    # Man page
    pisitools.insinto("/usr/share/man/man1", "/var/pisi/pommed-*/work/pommed-*/pommed.1")
    # Sounds
    pisitools.insinto("/usr/share/pommed", "/var/pisi/pommed-*/work/pommed-*/pommed/data/*")

    pisitools.dodoc("ChangeLog", "COPYING", "README")
