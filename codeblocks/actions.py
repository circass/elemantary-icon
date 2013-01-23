#!/usr/bin/python
# -*- coding: utf-8 -*-
#
# Licensed under the GNU General Public License, version 2.
# See the file http://www.gnu.org/licenses/old-licenses/gpl-2.0.txt

from pisi.actionsapi import autotools
from pisi.actionsapi import pisitools
from pisi.actionsapi import shelltools
from pisi.actionsapi import get

shelltools.export("HOME", get.workDIR())

def setup():
    shelltools.system("./bootstrap")
    autotools.autoreconf("-vif")
    #plugins = "AutoVersioning,BrowseTracker,byogames,Cccc,CppCheck,cbkoders,codesnippets,codestat,copystrings,dragscroll,envvars,headerfixup,help,hexeditor,incsearch,keybinder,MouseSap,profiler,regex,exporter,symtab,Valgrind"
    autotools.rawConfigure("--prefix=/usr \
                            --with-contrib-plugins=all")

    # Disable rpath
     #pisitools.dosed("libtool", "^hardcode_libdir_flag_spec=.*", "hardcode_libdir_flag_spec=\"\"")
     #pisitools.dosed("libtool", "^runpath_var=LD_RUN_PATH", "runpath_var=DIE_RPATH_DIE")


def build():
    autotools.make()

def install():
    autotools.rawInstall("DESTDIR=%s" % get.installDIR())
    #autotools.install("mandir=%s/%s" % (get.installDIR(), get.manDIR()))

    pisitools.dodoc("AUTHORS", "ChangeLog", "COPYING", "README")
