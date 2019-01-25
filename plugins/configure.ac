AC_INIT(neutrinohd2-plugins,4.0)
AM_INIT_AUTOMAKE([nostdinc])

m4_ifdef([AM_SILENT_RULES], [AM_SILENT_RULES([yes])])

TUXBOX_APPS
TUXBOX_APPS_DIRECTORY
TUXBOX_APPS_DVB
TUXBOX_BOXTYPE

AC_PROG_CC
AC_PROG_CXX
AC_DISABLE_STATIC

AM_PROG_LIBTOOL

PKG_CHECK_MODULES(CURL,libcurl)
PKG_CHECK_MODULES(FREETYPE,freetype2)
PKG_CHECK_MODULES(PNG,libpng)

# nglcd
AC_ARG_ENABLE(nglcd,
	AS_HELP_STRING(--enable-nglcd,include nglcd support),
	,[enable_nglcd=no])
AM_CONDITIONAL(ENABLE_NGLCD,test "$enable_nglcd" = "yes")

# tuxcalender
AC_ARG_ENABLE(tuxcal,
	AS_HELP_STRING(--enable-tuxcal,include tuxcalender support),
	,[enable_tuxcal=no])
AM_CONDITIONAL(ENABLE_TUXCAL,test "$enable_tuxcal" = "yes")

# testing
AC_ARG_ENABLE(testing,
	AS_HELP_STRING(--enable-testing,include testing plugins support),
	,[enable_testing=no])
AM_CONDITIONAL(ENABLE_TESTING,test "$enable_testing" = "yes")

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
satipclient/Makefile
icecast/Makefile
internetradio/Makefile
tsbrowser/Makefile
moviebrowser/Makefile
picviewer/Makefile
audioplayer/Makefile
opkg_manager/Makefile
nfilm/Makefile
ntvshows/Makefile
upnp/Makefile
ard/Makefile
python/Makefile
lua/Makefile
])

