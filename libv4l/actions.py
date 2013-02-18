#!/usr/bin/python
# -*- coding: utf-8 -*-
#
# Copyleft 2012 Pardus ANKA Community
# Copyright 2005-2011 TUBITAK/UEAKE
# Licensed under the GNU General Public License, version 2.
# See the file http://www.gnu.org/copyleft/gpl.txt.

from pisi.actionsapi import autotools
from pisi.actionsapi import pisitools
from pisi.actionsapi import shelltools
from pisi.actionsapi import get
    

def setup():
    autotools.configure()


    if get.buildTYPE() == "emul32":
        options = " --libdir=/usr/lib32 \
                    --prefix=/usr/lib32"
        shelltools.export("CFLAGS", "%s -m32" % get.CFLAGS())
        autotools.configure(options)

def build():
    #pisitools.dosed("Make.rules", "^CFLAGS\s:=.*")
    autotools.make()

def install():
    autotools.rawInstall("DESTDIR=%s" % get.installDIR())

    pisitools.dodoc("ChangeLog", "COPYING*", "README*", "TODO")
    pisitools.insinto("/%s/%s/" % (get.docDIR(), get.srcNAME()), "contrib")

