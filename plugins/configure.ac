AC_INIT(neutrinohd2-plugins,0.0.1)
AM_INIT_AUTOMAKE([0.0.1 nostdinc])

TUXBOX_APPS
TUXBOX_APPS_DIRECTORY
TUXBOX_APPS_DVB
TUXBOX_BOXTYPE

AC_PROG_CC
AC_PROG_CXX
AC_DISABLE_STATIC
AM_PROG_LIBTOOL

TUXBOX_APPS_LIB_CONFIG(CURL,curl-config)
PKG_CHECK_MODULES(FREETYPE,freetype2)
PKG_CHECK_MODULES(PNG,libpng)

AC_OUTPUT([
Makefile
test/Makefile
netzkino/Makefile
fbcallmonitor/Makefile
youtube/Makefile
systeminfo/Makefile
tuxdns/Makefile
restart_enigma/Makefile
tuxcom/Makefile
dvdplayer/Makefile
mediaplayer/Makefile
tuxcal/Makefile
tuxcal/daemon/Makefile
nglcd/Makefile
fileexplorer/Makefile
mediaportal/Makefile
])

