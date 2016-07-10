AC_DEFUN([TUXBOX_APPS],[
AM_CONFIG_HEADER(config.h)
AM_MAINTAINER_MODE

AC_GNU_SOURCE
AC_SYS_LARGEFILE

AC_ARG_WITH(target,
	[  --with-target=TARGET    target for compilation [[native,cdk]]],
	[TARGET="$withval"],[TARGET="native"])

AC_ARG_WITH(targetprefix,
	[  --with-targetprefix=PATH  prefix relative to target root (only applicable in cdk mode)],
	[targetprefix="$withval"],[targetprefix="NONE"])

AC_ARG_WITH(debug,
	[  --without-debug         disable debugging code],
	[DEBUG="$withval"],[DEBUG="yes"])

if test "$DEBUG" = "yes"; then
	DEBUG_CFLAGS="-g3 -ggdb"
	AC_DEFINE(DEBUG,1,[Enable debug messages])
fi

AC_MSG_CHECKING(target)

if test "$TARGET" = "native"; then
	AC_MSG_RESULT(native)

	if test "$CFLAGS" = "" -a "$CXXFLAGS" = ""; then
		CFLAGS="-Wall -O2 -pipe $DEBUG_CFLAGS"
		CXXFLAGS="-Wall -O2 -pipe $DEBUG_CFLAGS"
	fi
	if test "$prefix" = "NONE"; then
		prefix=/usr/local
	fi
	targetprefix=$prefix
elif test "$TARGET" = "cdk"; then
	AC_MSG_RESULT(cdk)

	if test "$CC" = "" -a "$CXX" = ""; then
		CC=sh4-linux-gcc CXX=sh4-linux-g++
	fi
	if test "$CFLAGS" = "" -a "$CXXFLAGS" = ""; then
		CFLAGS="-Wall -O2 -pipe $DEBUG_CFLAGS"
		CXXFLAGS="-Wall -O2 -pipe $DEBUG_CFLAGS"
	fi
	if test "$prefix" = "NONE"; then
		AC_MSG_ERROR(invalid prefix, you need to specify one in cdk mode)
	fi
	if test "$targetprefix" = "NONE"; then
		targetprefix=""
	fi
	if test "$host_alias" = ""; then
		cross_compiling=yes
		host_alias=sh4-linux
	fi
else
	AC_MSG_RESULT(none)
	AC_MSG_ERROR([invalid target $TARGET, choose on from native,cdk]);
fi

AC_CANONICAL_BUILD
AC_CANONICAL_HOST

check_path () {
	return $(perl -e "if(\"$1\"=~m#^/usr/(local/)?bin#){print \"0\"}else{print \"1\";}")
}
])

AC_DEFUN([TUXBOX_APPS_DIRECTORY_ONE],[
AC_ARG_WITH($1,[  $6$7 [[PREFIX$4$5]]],[
	_$2=$withval
	if test "$TARGET" = "cdk"; then
		$2=`eval echo "${targetprefix}$withval"`
	else
		$2=$withval
	fi
	TARGET_$2=${$2}
],[
	$2="\${$3}$5"
	if test "$TARGET" = "cdk"; then
		_$2=`eval echo "${target$3}$5"`
	else
		_$2=`eval echo "${$3}$5"`
	fi
	TARGET_$2=$_$2
])

dnl automake <= 1.6 don't support this
dnl AC_SUBST($2)
AC_DEFINE_UNQUOTED($2,"$_$2",$7)
AC_SUBST(TARGET_$2)
])

AC_DEFUN([TUXBOX_APPS_DIRECTORY],[
AC_REQUIRE([TUXBOX_APPS])

if test "$TARGET" = "cdk"; then
	datadir="\${prefix}/share"
	sysconfdir="\${prefix}/etc"
	localstatedir="\${prefix}/var"
	libdir="\${prefix}/lib"
	targetdatadir="\${targetprefix}/share"
	targetsysconfdir="\${targetprefix}/etc"
	targetlocalstatedir="\${targetprefix}/var"
	targetlibdir="\${targetprefix}/lib"
fi

TUXBOX_APPS_DIRECTORY_ONE(configdir,CONFIGDIR,localstatedir,/var,/tuxbox/config,
	[--with-configdir=PATH   ],[where to find the config files])

TUXBOX_APPS_DIRECTORY_ONE(datadir,DATADIR,datadir,/share,/tuxbox,
	[--with-datadir=PATH     ],[where to find data])

TUXBOX_APPS_DIRECTORY_ONE(fontdir,FONTDIR,datadir,/share,/fonts,
	[--with-fontdir=PATH     ],[where to find the fonts])

TUXBOX_APPS_DIRECTORY_ONE(gamesdir,GAMESDIR,localstatedir,/var,/tuxbox/games,
	[--with-gamesdir=PATH    ],[where games data is stored])

TUXBOX_APPS_DIRECTORY_ONE(plugindir,PLUGINDIR,localstatedir,/var,/tuxbox/plugins,
	[--with-plugindir=PATH   ],[where to find the plugins])

TUXBOX_APPS_DIRECTORY_ONE(themesdir,THEMESDIR,datadir,/share,/tuxbox/neutrino/themes,
	[--with-themesdir=PATH     ],[where to find the themes (don't change)])
])

dnl automake <= 1.6 needs this specifications
AC_SUBST(CONFIGDIR)
AC_SUBST(DATADIR)
AC_SUBST(FONTDIR)
AC_SUBST(GAMESDIR)
AC_SUBST(PLUGINDIR)
AC_SUBST(THEMESDIR)
dnl end workaround

AC_DEFUN([TUXBOX_APPS_ENDIAN],[
AC_CHECK_HEADERS(endian.h)
AC_C_BIGENDIAN
])

AC_DEFUN([TUXBOX_APPS_DRIVER],[
AC_ARG_WITH(driver,
	[  --with-driver=PATH      path for driver sources [[NONE]]],
	[DRIVER="$withval"],[DRIVER=""])

if test "$DRIVER"; then
	AC_SUBST(DRIVER)
	CPPFLAGS="$CPPFLAGS -I$DRIVER/include"
fi
])

AC_DEFUN([TUXBOX_APPS_DVB],[
AC_ARG_WITH(dvbincludes,
	[  --with-dvbincludes=PATH  path for dvb includes [[NONE]]],
	[DVBINCLUDES="$withval"],[DVBINCLUDES=""])

if test "$DVBINCLUDES"; then
	CPPFLAGS="$CPPFLAGS -I$DVBINCLUDES"
fi

AC_CHECK_HEADERS(ost/dmx.h,[
	DVB_API_VERSION=1
	AC_MSG_NOTICE([found dvb version 1])
])

if test -z "$DVB_API_VERSION"; then
AC_CHECK_HEADERS(linux/dvb/version.h,[
	AC_LANG_PREPROC_REQUIRE()
	AC_REQUIRE([AC_PROG_EGREP])
	AC_LANG_CONFTEST([AC_LANG_SOURCE([[
#include <linux/dvb/version.h>
version DVB_API_VERSION
	]])])
	DVB_API_VERSION=`(eval "$ac_cpp conftest.$ac_ext") 2>&AS_MESSAGE_LOG_FD | $EGREP "^version" | sed "s,version\ ,,"`
	rm -f conftest*

	AC_MSG_NOTICE([found dvb version $DVB_API_VERSION])
])
fi

if test "$DVB_API_VERSION"; then
	AC_DEFINE(HAVE_DVB,1,[Define to 1 if you have the dvb includes])
	AC_DEFINE_UNQUOTED(HAVE_DVB_API_VERSION,$DVB_API_VERSION,[Define to the version of the dvb api])
else
	AC_MSG_ERROR([can't find dvb headers])
fi
])

AC_DEFUN([_TUXBOX_APPS_LIB_CONFIG],[
AC_PATH_PROG($1_CONFIG,$2,no)
if test "$$1_CONFIG" != "no"; then
	if test "$TARGET" = "cdk" && check_path "$$1_CONFIG"; then
		AC_MSG_$3([could not find a suitable version of $2]);
	else
		$1_CFLAGS=$($$1_CONFIG --cflags)
		$1_LIBS=$($$1_CONFIG --libs)
	fi
fi

AC_SUBST($1_CFLAGS)
AC_SUBST($1_LIBS)
])

AC_DEFUN([TUXBOX_APPS_LIB_CONFIG],[
_TUXBOX_APPS_LIB_CONFIG($1,$2,ERROR)
if test "$$1_CONFIG" = "no"; then
	AC_MSG_ERROR([could not find $2]);
fi
])

AC_DEFUN([TUXBOX_APPS_LIB_CONFIG_CHECK],[
_TUXBOX_APPS_LIB_CONFIG($1,$2,WARN)
])

AC_DEFUN([TUXBOX_APPS_PKGCONFIG],[
AC_PATH_PROG(PKG_CONFIG, pkg-config,no)
if test "$PKG_CONFIG" = "no" ; then
	AC_MSG_ERROR([could not find pkg-config]);
fi
])

AC_DEFUN([_TUXBOX_APPS_LIB_PKGCONFIG],[
AC_REQUIRE([TUXBOX_APPS_PKGCONFIG])
AC_MSG_CHECKING(for package $2)
if PKG_CONFIG_PATH="$PKG_CONFIG_PATH" $PKG_CONFIG --exists "$2" ; then
	AC_MSG_RESULT(yes)
	$1_CFLAGS=$(PKG_CONFIG_PATH="$PKG_CONFIG_PATH" $PKG_CONFIG --cflags "$2")
	$1_LIBS=$(PKG_CONFIG_PATH="$PKG_CONFIG_PATH" $PKG_CONFIG --libs "$2")
else
	AC_MSG_RESULT(no)
fi

AC_SUBST($1_CFLAGS)
AC_SUBST($1_LIBS)
])

AC_DEFUN([TUXBOX_APPS_LIB_PKGCONFIG],[
_TUXBOX_APPS_LIB_PKGCONFIG($1,$2)
if test -z "$$1_CFLAGS" ; then
	AC_MSG_ERROR([could not find package $2]);
fi
])

AC_DEFUN([TUXBOX_APPS_LIB_PKGCONFIG_CHECK],[
_TUXBOX_APPS_LIB_PKGCONFIG($1,$2)
])

AC_DEFUN([_TUXBOX_APPS_LIB_SYMBOL],[
AC_CHECK_LIB($2,$3,HAVE_$1="yes",HAVE_$1="no")
if test "$HAVE_$1" = "yes"; then
	$1_LIBS=-l$2
fi

AC_SUBST($1_LIBS)
])

AC_DEFUN([TUXBOX_APPS_LIB_SYMBOL],[
_TUXBOX_APPS_LIB_SYMBOL($1,$2,$3,ERROR)
if test "$HAVE_$1" = "no"; then
	AC_MSG_ERROR([could not find $2]);
fi
])

AC_DEFUN([TUXBOX_APPS_LIB_CONFIG_SYMBOL],[
_TUXBOX_APPS_LIB_SYMBOL($1,$2,$3,WARN)
])

AC_DEFUN([TUXBOX_APPS_GETTEXT],[
AC_PATH_PROG(MSGFMT, msgfmt, no)
AC_PATH_PROG(GMSGFMT, gmsgfmt, $MSGFMT)
AC_PATH_PROG(XGETTEXT, xgettext, no)
AC_PATH_PROG(MSGMERGE, msgmerge, no)

AC_MSG_CHECKING([whether NLS is requested])
AC_ARG_ENABLE(nls,
	[  --disable-nls           do not use Native Language Support],
	USE_NLS=$enableval, USE_NLS=yes)
AC_MSG_RESULT($USE_NLS)
AC_SUBST(USE_NLS)

if test "$USE_NLS" = "yes"; then
	AC_CACHE_CHECK([for GNU gettext in libc], gt_cv_func_gnugettext_libc,[
		AC_TRY_LINK([
			#include <libintl.h>
			#ifndef __GNU_GETTEXT_SUPPORTED_REVISION
			#define __GNU_GETTEXT_SUPPORTED_REVISION(major) ((major) == 0 ? 0 : -1)
			#endif
			extern int _nl_msg_cat_cntr;
			extern int *_nl_domain_bindings;
			],[
			bindtextdomain ("", "");
			return (int) gettext ("") + _nl_msg_cat_cntr + *_nl_domain_bindings;
			], gt_cv_func_gnugettext_libc=yes, gt_cv_func_gnugettext_libc=no
		)]
	)

	if test "$gt_cv_func_gnugettext_libc" = "yes"; then
		AC_DEFINE(ENABLE_NLS, 1, [Define to 1 if translation of program messages to the user's native language is requested.])
		gt_use_preinstalled_gnugettext=yes
	else
		USE_NLS=no
	fi
fi

if test -f "$srcdir/po/LINGUAS"; then
	ALL_LINGUAS=$(sed -e "/^#/d" "$srcdir/po/LINGUAS")
fi

POFILES=
GMOFILES=
UPDATEPOFILES=
DUMMYPOFILES=
for lang in $ALL_LINGUAS; do
	POFILES="$POFILES $srcdirpre$lang.po"
	GMOFILES="$GMOFILES $srcdirpre$lang.gmo"
	UPDATEPOFILES="$UPDATEPOFILES $lang.po-update"
	DUMMYPOFILES="$DUMMYPOFILES $lang.nop"
done
INST_LINGUAS=
if test -n "$ALL_LINGUAS"; then
	for presentlang in $ALL_LINGUAS; do
		useit=no
		if test -n "$LINGUAS"; then
			desiredlanguages="$LINGUAS"
		else
			desiredlanguages="$ALL_LINGUAS"
		fi
		for desiredlang in $desiredlanguages; do
			case "$desiredlang" in
				"$presentlang"*) useit=yes;;
			esac
		done
		if test $useit = yes; then
			INST_LINGUAS="$INST_LINGUAS $presentlang"
		fi
	done
fi
CATALOGS=
if test -n "$INST_LINGUAS"; then
	for lang in $INST_LINGUAS; do
		CATALOGS="$CATALOGS $lang.gmo"
	done
fi
AC_SUBST(POFILES)
AC_SUBST(GMOFILES)
AC_SUBST(UPDATEPOFILES)
AC_SUBST(DUMMYPOFILES)
AC_SUBST(CATALOGS)
])

dnl backward compatiblity
AC_DEFUN([AC_GNU_SOURCE],
[AH_VERBATIM([_GNU_SOURCE],
[/* Enable GNU extensions on systems that have them.  */
#ifndef _GNU_SOURCE
# undef _GNU_SOURCE
#endif])dnl
AC_BEFORE([$0], [AC_COMPILE_IFELSE])dnl
AC_BEFORE([$0], [AC_RUN_IFELSE])dnl
AC_DEFINE([_GNU_SOURCE])
])

AC_DEFUN([AC_PROG_EGREP],
[AC_CACHE_CHECK([for egrep], [ac_cv_prog_egrep],
   [if echo a | (grep -E '(a|b)') >/dev/null 2>&1
    then ac_cv_prog_egrep='grep -E'
    else ac_cv_prog_egrep='egrep'
    fi])
 EGREP=$ac_cv_prog_egrep
 AC_SUBST([EGREP])
])

AC_DEFUN([TUXBOX_BOXTYPE],[

AC_ARG_WITH(boxtype,
	[  --with-boxtype          valid values: generic,dgs,gigablue,dreambox,xtrend,fulan,kathrein,ipbox,topfield,fortis_hdbox,octagon,atevio,adb_box,whitebox,vip,homecast,vuplus,technomate,coolstream,hypercube],
	[case "${withval}" in
		generic|dgs|gigablue|dreambox|xtrend|fulan|kathrein|ipbox|topfield|fortis_hdbox|octagon|atevio|adb_box|whitebox|vip|homecast|vuplus|technomate|coolstream|hypercube)
			BOXTYPE="$withval"
			;;
		cu*)
			BOXTYPE="dgs"
			BOXMODEL="$withval"
			;;
		gb*)
			BOXTYPE="gigablue"
			BOXMODEL="$withval"
			;;
		dm*)
			BOXTYPE="dreambox"
			BOXMODEL="$withval"
			;;
		et*)
			BOXTYPE="xtrend"
			BOXMODEL="$withval"
			;;
		spa*)
			BOXTYPE="fulan"
			BOXMODEL="$withval"
			;;
		uf*)
			BOXTYPE="kathrein"
			BOXMODEL="$withval"
			;;
		ip*)
			BOXTYPE="ipbox"
			BOXMODEL="$withval"
			;;
		at*)
			BOXTYPE="atevio"
			BOXMODEL="$withval"
			;;
		vu*)
			BOXTYPE="vuplus"
			BOXMODEL="$withval"
			;;
		tm*)
			BOXTYPE="technomate"
			BOXMODEL="$withval"
			;;
		*)
			AC_MSG_ERROR([unsupported value $withval for --with-boxtype])
			;;
	esac], [BOXTYPE="generic"])

AC_ARG_WITH(boxmodel,
	[  --with-boxmodel	valid for dgs: cuberevo,cuberevo_mini,cuberevo_mini2,cuberevo_mini_fta,cuberevo_250hd,cuberevo_2000hd,cuberevo_9500hd
				valid for gigablue: gbsolo,gb800se,gb800ue,gbquad
				valid for dreambox: dm500, dm500plus, dm600pvr, dm56x0, dm7000, dm7020, dm7025, dm500hd, dm7020hd, dm8000, dm800, dm800se
				valid for xtrend: et4x00,et5x00,et6x00,et9x00
				valid for fulan: spark, spark7162
				valid for kathrein: ufs910, ufs922, ufs912, ufs913
				valid for ipbox: ipbox55, ipbox99, ipbox9900
				valid for atevio: atevio700,atevio7000,atevio7500,atevio7600
				valid for vuplus: vusolo,vuduo,vuuno,vuultimo
				valid for technomate: tmtwin,tm2t,tmsingle],
	[case "${withval}" in
		cuberevo|cuberevo_mini|cuberevo_mini2|cuberevo_mini_fta|cuberevo_250hd|cuberevo_2000hd|cuberevo_9500hd)
			if test "$BOXTYPE" = "dgs"; then
				BOXMODEL="$withval"
			else
				AC_MSG_ERROR([unknown model $withval for boxtype $BOXTYPE])
			fi
			;;
		gb800solo|gb800se|gb800ue|gbquad)
			if test "$BOXTYPE" = "gigablue"; then
				BOXMODEL="$withval"
			else
				AC_MSG_ERROR([unknown model $withval for boxtype $BOXTYPE])
			fi
			;;
		dm500|dm500plus|dm600pvr|dm56x0|dm7000|dm7020|dm7025|dm500hd|dm7020hd|dm8000|dm800|dm800se)
			if test "$BOXTYPE" = "dreambox"; then
				BOXMODEL="$withval"
			else
				AC_MSG_ERROR([unknown model $withval for boxtype $BOXTYPE])
			fi
			;;
		et4x00|et5x00|et6x00|et9x00)
			if test "$BOXTYPE" = "xtrend"; then
				BOXMODEL="$withval"
			else
				AC_MSG_ERROR([unknown model $withval for boxtype $BOXTYPE])
			fi
			;;
		spark|spark7162)
			if test "$BOXTYPE" = "fulan"; then
				BOXMODEL="$withval"
			else
				AC_MSG_ERROR([unknown model $withval for boxtype $BOXTYPE])
			fi
			;;
		ufs910|ufs912|ufs913|ufs922|ufc960)
			if test "$BOXTYPE" = "kathrein"; then
				BOXMODEL="$withval"
			else
				AC_MSG_ERROR([unknown model $withval for boxtype $BOXTYPE])
			fi
			;;
		ipbox55|ipbox99|ipbox9900)
			if test "$BOXTYPE" = "ipbox"; then
				BOXMODEL="$withval"
			else
				AC_MSG_ERROR([unknown model $withval for boxtype $BOXTYPE])
			fi
			;;
		atevio700|atevio7000|atevio7500|atevio7600)
			if test "$BOXTYPE" = "atevio"; then
				BOXMODEL="$withval"
			else
				AC_MSG_ERROR([unknown model $withval for boxtype $BOXTYPE])
			fi
			;;
		vusolo|vuduo|vuuno|vuultimo)
			if test "$BOXTYPE" = "vuplus"; then
				BOXMODEL="$withval"
			else
				AC_MSG_ERROR([unknown model $withval for boxtype $BOXTYPE])
			fi
			;;
		tmtwin|tm2t|tmsingle)
			if test "$BOXTYPE" = "technomate"; then
				BOXMODEL="$withval"
			else
				AC_MSG_ERROR([unknown model $withval for boxtype $BOXTYPE])
			fi
			;;
		*)
			AC_MSG_ERROR([unsupported value $withval for --with-boxmodel])
			;;
	esac],
	[if test "$BOXTYPE" = "dgs" -o "$BOXTYPE" = "gigablue" -o "$BOXTYPE" = "dreambox" -o "$BOXTYPE" = "xtrend" -o "$BOXTYPE" = "fulan" -o "$BOXTYPE" = "kathrein" -o "$BOXTYPE" = "ipbox" -o "$BOXTYPE" = "atevio" -o "$BOXTYPE" = "vuplus" -o "$BOXTYPE" = "technomate" && test -z "$BOXMODEL"; then
		AC_MSG_ERROR([this boxtype $BOXTYPE needs --with-boxmodel])
	fi])

AC_SUBST(BOXTYPE)
AC_SUBST(BOXMODEL)

AM_CONDITIONAL(BOXTYPE_GENERIC, test "$BOXTYPE" = "generic")
AM_CONDITIONAL(BOXTYPE_DGS, test "$BOXTYPE" = "dgs")
AM_CONDITIONAL(BOXTYPE_GIGABLUE, test "$BOXTYPE" = "gigablue")
AM_CONDITIONAL(BOXTYPE_DREAMBOX, test "$BOXTYPE" = "dreambox")
AM_CONDITIONAL(BOXTYPE_XTREND, test "$BOXTYPE" = "xtrend")
AM_CONDITIONAL(BOXTYPE_FULAN, test "$BOXTYPE" = "fulan")
AM_CONDITIONAL(BOXTYPE_KATHREIN, test "$BOXTYPE" = "kathrein")
AM_CONDITIONAL(BOXTYPE_IPBOX, test "$BOXTYPE" = "ipbox")
AM_CONDITIONAL(BOXTYPE_TOPFIELD, test "$BOXTYPE" = "toptfield")
AM_CONDITIONAL(BOXTYPE_FORTIS_HDBOX, test "$BOXTYPE" = "fortis_hdbox")
AM_CONDITIONAL(BOXTYPE_OCTAGON, test "$BOXTYPE" = "octagon")
AM_CONDITIONAL(BOXTYPE_ATEVIO, test "$BOXTYPE" = "atevio")
AM_CONDITIONAL(BOXTYPE_ADB_BOX, test "$BOXTYPE" = "adb_box")
AM_CONDITIONAL(BOXTYPE_WHITEBOX, test "$BOXTYPE" = "whitebox")
AM_CONDITIONAL(BOXTYPE_VIP, test "$BOXTYPE" = "vip")
AM_CONDITIONAL(BOXTYPE_HOMECAST, test "$BOXTYPE" = "homecast")
AM_CONDITIONAL(BOXTYPE_VUPLUS, test "$BOXTYPE" = "vuplus")
AM_CONDITIONAL(BOXTYPE_TECHNOMATE, test "$BOXTYPE" = "technomate")
AM_CONDITIONAL(BOXTYPE_IPBOX, test "$BOXTYPE" = "ipbox")
AM_CONDITIONAL(BOXTYPE_KATHREIN, test "$BOXTYPE" = "kathrein")
AM_CONDITIONAL(BOXTYPE_COOLSTREAM, test "$BOXTYPE" = "coolstream")
AM_CONDITIONAL(BOXTYPE_HYPERCUBE, test "$BOXTYPE" = "hypercube")

AM_CONDITIONAL(BOXMODEL_CUBEREVO, test "$BOXMODEL" = "cuberevo")
AM_CONDITIONAL(BOXMODEL_CUBEREVO_MINI, test "$BOXMODEL" = "cuberevo_mini")
AM_CONDITIONAL(BOXMODEL_CUBEREVO_MINI2, test "$BOXMODEL" = "cuberevo_mini2")
AM_CONDITIONAL(BOXMODEL_CUBEREVO_MINI_FTA, test "$BOXMODEL" = "cuberevo_mini_fta")
AM_CONDITIONAL(BOXMODEL_CUBEREVO_250HD, test "$BOXMODEL" = "cuberevo_250hd")
AM_CONDITIONAL(BOXMODEL_CUBEREVO_2000HD, test "$BOXMODEL" = "cuberevo_2000hd")
AM_CONDITIONAL(BOXMODEL_CUBEREVO_9500HD, test "$BOXMODEL" = "cuberevo_9500HD")

AM_CONDITIONAL(BOXMODEL_GB800SOLO,test "$BOXMODEL" = "gb800solo")
AM_CONDITIONAL(BOXMODEL_GB800SE,test "$BOXMODEL" = "gb800se")
AM_CONDITIONAL(BOXMODEL_GB800UE,test "$BOXMODEL" = "gb800ue")
AM_CONDITIONAL(BOXMODEL_GBQUAD,test "$BOXMODEL" = "gbquad")

AM_CONDITIONAL(BOXMODEL_DM500,test "$BOXMODEL" = "dm500")
AM_CONDITIONAL(BOXMODEL_DM500PLUS,test "$BOXMODEL" = "dm500plus")
AM_CONDITIONAL(BOXMODEL_DM600PVR,test "$BOXMODEL" = "dm600pvr")
AM_CONDITIONAL(BOXMODEL_DM56x0,test "$BOXMODEL" = "dm56x0")
AM_CONDITIONAL(BOXMODEL_DM7000,test "$BOXMODEL" = "dm7000" -o "$BOXMODEL" = "dm7020" -o "$BOXMODEL" = "dm7025")

AM_CONDITIONAL(BOXMODEL_DM500HD,test "$BOXMODEL" = "dm500hd")
AM_CONDITIONAL(BOXMODEL_DM800HD,test "$BOXMODEL" = "dm800")
AM_CONDITIONAL(BOXMODEL_DM800SE,test "$BOXMODEL" = "dm800se")
AM_CONDITIONAL(BOXMODEL_DM7000HD,test "$BOXMODEL" = "dm7020hd")
AM_CONDITIONAL(BOXMODEL_DM8000HD,test "$BOXMODEL" = "dm8000")

AM_CONDITIONAL(BOXMODEL_ET4X00,test "$BOXMODEL" = "et4x00")
AM_CONDITIONAL(BOXMODEL_ET5X00,test "$BOXMODEL" = "et5x00")
AM_CONDITIONAL(BOXMODEL_ET6X00,test "$BOXMODEL" = "et6x00")
AM_CONDITIONAL(BOXMODEL_ET9X00,test "$BOXMODEL" = "et9x00")

AM_CONDITIONAL(BOXMODEL_SPARK,test "$BOXMODEL" = "spark")
AM_CONDITIONAL(BOXMODEL_SPARK7162,test "$BOXMODEL" = "spark7162")

AM_CONDITIONAL(BOXMODEL_UFS910, test "$BOXMODEL" = "ufs910")
AM_CONDITIONAL(BOXMODEL_UFS912,test "$BOXMODEL" = "ufs912")
AM_CONDITIONAL(BOXMODEL_UFS913,test "$BOXMODEL" = "ufs913")
AM_CONDITIONAL(BOXMODEL_UFS922,test "$BOXMODEL" = "ufs922")
AM_CONDITIONAL(BOXMODEL_UFC960,test "$BOXMODEL" = "ufc960")

AM_CONDITIONAL(BOXMODEL_IPBOX55, test "$BOXMODEL" = "ipbox55")
AM_CONDITIONAL(BOXMODEL_IPBOX99, test "$BOXMODEL" = "ipbox99")
AM_CONDITIONAL(BOXMODEL_IPBOX9900, test "$BOXMODEL" = "ipbox9900")

AM_CONDITIONAL(BOXMODEL_ATEVIO700, test "$BOXMODEL" = "atevio700")
AM_CONDITIONAL(BOXMODEL_ATEVIO7000, test "$BOXMODEL" = "atevio7000")
AM_CONDITIONAL(BOXMODEL_ATEVIO7500, test "$BOXMODEL" = "atevio7500")
AM_CONDITIONAL(BOXMODEL_ATEVIO7600, test "$BOXMODEL" = "atevio7600")

AM_CONDITIONAL(BOXMODEL_VUSOLO,test "$BOXMODEL" = "vusolo")
AM_CONDITIONAL(BOXMODEL_VUDUO,test "$BOXMODEL" = "vuduo")
AM_CONDITIONAL(BOXMODEL_VUUNO,test "$BOXMODEL" = "vuuno")
AM_CONDITIONAL(BOXMODEL_VUULTIMO,test "$BOXMODEL" = "vuultimo")

AM_CONDITIONAL(BOXMODEL_TMTWIN,test "$BOXMODEL" = "tmtwin")
AM_CONDITIONAL(BOXMODEL_TM2T,test "$BOXMODEL" = "tm2t")
AM_CONDITIONAL(BOXMODEL_TMSINGLE,test "$BOXMODEL" = "tmsingle")

if test "$BOXTYPE" = "generic"; then
	AC_DEFINE(PLATFORM_GENERIC, 1, [building for generic])
elif test "$BOXTYPE" = "dgs"; then
	AC_DEFINE(PLATFORM_DGS, 1, [building for dgs])
elif test "$BOXTYPE" = "gigablue"; then
	AC_DEFINE(PLATFORM_GIGABLUE, 1, [building for gigablue])
elif test "$BOXTYPE" = "dreambox"; then
	AC_DEFINE(PLATFORM_DREAMBOX, 1, [building for dreambox])
elif test "$BOXTYPE" = "xtrend"; then
	AC_DEFINE(PLATFORM_XTREND, 1, [building for xtrend])
elif test "$BOXTYPE" = "fulan"; then
	AC_DEFINE(PLATFORM_FULAN, 1, [building for fulan])
elif test "$BOXTYPE" = "kathrein"; then
	AC_DEFINE(PLATFORM_KATHREIN, 1, [building for kathrein])
elif test "$BOXTYPE" = "ipbox"; then
	AC_DEFINE(PLATFORM_IPBOX, 1, [building for ipbox])
elif test "$BOXTYPE" = "topfield"; then
	AC_DEFINE(PLATFORM_TF7700, 1, [building for a topfield])
elif test "$BOXTYPE" = "fortis_hdbox"; then
	AC_DEFINE(PLATFORM_FORTIS_HDBOX, 1, [building for a fortis_hdbox])
elif test "$BOXTYPE" = "octagon"; then
	AC_DEFINE(PLATFORM_OCTAGON, 1, [building for a octagon])
elif test "$BOXTYPE" = "atevio"; then
	AC_DEFINE(PLATFORM_ATEVIO, 1, [building for a atevio])
elif test "$BOXTYPE" = "adb_box"; then
	AC_DEFINE(PLATFORM_ADB_BOX, 1, [building for a adb_box])
elif test "$BOXTYPE" = "whitebox"; then
	AC_DEFINE(PLATFORM_WHITEBOX, 1, [building for a whitebox])
elif test "$BOXTYPE" = "vip"; then
	AC_DEFINE(PLATFORM_VIP, 1, [building for an vip])
elif test "$BOXTYPE" = "homecast"; then
	AC_DEFINE(PLATFORM_HOMECAST, 1, [building for a homecast])
elif test "$BOXTYPE" = "vuplus"; then
	AC_DEFINE(PLATFORM_VUPLUS, 1, [building for a vuplus])
elif test "$BOXTYPE" = "technomate"; then
	AC_DEFINE(PLATFORM_TECHNOMATE, 1, [building for a technomate])
elif test "$BOXTYPE" = "coolstream"; then
	AC_DEFINE(PLATFORM_COOLSTREAM, 1, [building for a coolstream])
elif test "$BOXTYPE" = "hypercube"; then
	AC_DEFINE(PLATFORM_HYPERCUBE, 1, [building for a hypercube])
fi

if test "$BOXMODEL" = "cuberevo"; then
	AC_DEFINE(PLATFORM_CUBEREVO, 1, [building for cuberevo])
elif test "$BOXMODEL" = "cuberevo_mini"; then
	AC_DEFINE(PLATFORM_CUBEREVO_MINI, 1, [building for cuberevo_mini])
elif test "$BOXMODEL" = "cuberevo_mini2"; then
	AC_DEFINE(PLATFORM_CUBEREVO_MINI2, 1, [building for cuberevo_mini2])
elif test "$BOXMODEL" = "cuberevo_mini_fta"; then
	AC_DEFINE(PLATFORM_CUBEREVO_MINI_FTA, 1, [building for cuberevo_mini_fta])
elif test "$BOXMODEL" = "cuberevo_250hd"; then
	AC_DEFINE(PLATFORM_CUBEREVO_250HD, 1, [building for cuberevo_250hd])
elif test "$BOXMODEL" = "cuberevo_2000hd"; then
	AC_DEFINE(PLATFORM_CUBEREVO_2000HD, 1, [building for cuberevo_2000hd])
elif test "$BOXMODEL" = "cuberevo_9500hd"; then
	AC_DEFINE(PLATFORM_CUBEREVO_9500HD, 1, [building for cuberevo_9500hd])

elif test "$BOXMODEL" = "gb800solo"; then
	AC_DEFINE(BOXMODEL_GB800SOLO, 1, [building for gigablue 800solo])
elif test "$BOXMODEL" = "gb800se"; then
	AC_DEFINE(BOXMODEL_GB800SE, 1, [building for gigablue 800se])
elif test "$BOXMODEL" = "gb800ue"; then
	AC_DEFINE(BOXMODEL_GB800UE, 1, [building for gigablue 800ue])
elif test "$BOXMODEL" = "gbquad"; then
	AC_DEFINE(BOXMODEL_GBQUAD, 1, [building for gigablue quad])

elif test "$BOXMODEL" = "dm500"; then
	AC_DEFINE(BOXMODEL_DM500, 1, [building for dreambox 500])
elif test "$BOXMODEL" = "dm500plus"; then
	AC_DEFINE(BOXMODEL_DM500PLUS, 1, [building for dreambox 500plus])
elif test "$BOXMODEL" = "dm600pvr"; then
	AC_DEFINE(BOXMODEL_DM600PVR, 1, [building for dreambox 600pvr])
elif test "$BOXMODEL" = "dm56x0"; then
	AC_DEFINE(BOXMODEL_DM56x0, 1, [building for dreambox 56x0])
elif test "$BOXMODEL" = "dm7000" -o "$BOXMODEL" = "dm7020" -o "$BOXMODEL" = "dm7025"; then
	AC_DEFINE(BOXMODEL_DM7000, 1, [building for dreambox 70xx])
elif test "$BOXMODEL" = "dm500hd"; then
	AC_DEFINE(BOXMODEL_DM500HD, 1, [building for dreambox 500hd])
elif test "$BOXMODEL" = "dm7020hd"; then
	AC_DEFINE(BOXMODEL_DM7000HD, 1, [building for dreambox 7020hd])
elif test "$BOXMODEL" = "dm8000"; then
	AC_DEFINE(BOXMODEL_DM8000HD, 1, [building for dreambox 8000])
elif test "$BOXMODEL" = "dm800"; then
	AC_DEFINE(BOXMODEL_DM800HD, 1, [building for dreambox 800])
elif test "$BOXMODEL" = "dm800se"; then
	AC_DEFINE(BOXMODEL_DM800SE, 1, [building for dreambox 800se])

elif test "$BOXMODEL" = "et4x00"; then
	AC_DEFINE(BOXMODEL_ET4X00, 1, [building for xtrend et4x00])
elif test "$BOXMODEL" = "et5x00"; then
	AC_DEFINE(BOXMODEL_ET5X00, 1, [building for xtrend et5x00])
elif test "$BOXMODEL" = "et6x00"; then
	AC_DEFINE(BOXMODEL_ET6X00, 1, [building for xtrend et6x00])
elif test "$BOXMODEL" = "et9x00"; then
	AC_DEFINE(BOXMODEL_ET9X00, 1, [building for xtrend et9x00])

elif test "$BOXMODEL" = "spark"; then
	AC_DEFINE(PLATFORM_SPARK, 1, [building for spark 7111])
elif test "$BOXMODEL" = "spark7162"; then
	AC_DEFINE(PLATFORM_SPARK7162, 1, [building for spark 7162])

elif test "$BOXMODEL" = "ufs910"; then
	AC_DEFINE(PLATFORM_UFS910, 1, [building for ufs910])
elif test "$BOXMODEL" = "ufs912"; then
	AC_DEFINE(PLATFORM_UFS912, 1, [building for ufs912])
elif test "$BOXMODEL" = "ufs913"; then
	AC_DEFINE(PLATFORM_UFS913, 1, [building for ufs913])
elif test "$BOXMODEL" = "ufs922"; then
	AC_DEFINE(PLATFORM_UFS922, 1, [building for ufs922])

elif test "$BOXMODEL" = "ipbox55"; then
	AC_DEFINE(PLATFORM_IPBOX55, 1, [building for ipbox55])
elif test "$BOXMODEL" = "ipbox99"; then
	AC_DEFINE(PLATFORM_IPBOX99, 1, [building for ipbox99])
elif test "$BOXMODEL" = "ipbox9900"; then
	AC_DEFINE(PLATFORM_IPBOX9900, 1, [building for ipbox9900])

elif test "$BOXMODEL" = "atevio700"; then
	AC_DEFINE(BOXMODEL_ATEVIO700, 1, [building for atevio700])
elif test "$BOXMODEL" = "atevio7000"; then
	AC_DEFINE(BOXMODEL_ATEVIO7000, 1, [building for atevio7000])
elif test "$BOXMODEL" = "atevio7500"; then
	AC_DEFINE(BOXMODEL_ATEVIO7500, 1, [building for atevio7500])
elif test "$BOXMODEL" = "atevio7600"; then
	AC_DEFINE(BOXMODEL_ATEVIO7600, 1, [building for atevio7600])

elif test "$BOXMODEL" = "vusolo"; then
	AC_DEFINE(BOXMODEL_VUSOLO, 1, [building for vuplus solo])
elif test "$BOXMODEL" = "vuduo"; then
	AC_DEFINE(BOXMODEL_VUDUO, 1, [building for vuplus duo])
elif test "$BOXMODEL" = "vuuno"; then
	AC_DEFINE(BOXMODEL_VUUNO, 1, [building for vuplus uno])
elif test "$BOXMODEL" = "vuultimo"; then
	AC_DEFINE(BOXMODEL_VUULTIMO, 1, [building for vuplus ultimo])

elif test "$BOXMODEL" = "tmtwin"; then
	AC_DEFINE(BOXMODEL_TMTWIN, 1, [building for technomate twin])
elif test "$BOXMODEL" = "tm2t"; then
	AC_DEFINE(BOXMODEL_TM2T, 1, [building for technomate 2t])
elif test "$BOXMODEL" = "tmsingle"; then
	AC_DEFINE(BOXMODEL_TMSINGLE, 1, [building for technomate single])
fi
])


