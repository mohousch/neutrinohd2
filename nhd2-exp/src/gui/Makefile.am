AM_CPPFLAGS = -fno-strict-aliasing

BUILT_SOURCES  = svn_version.h

svn_version.h:
	@if test -d ../../.svn ; then \
		sleep 180 && rm svn_version.h & echo "#define SVNVERSION \"$$(svnversion -n || echo svn_oops!)\" " > svn_version.h ; \
	else \
		rm svn_version.h; echo '#define BUILT_DATE "'`date`'"' > svn_version.h ; \
	fi

noinst_HEADERS = svn_version.h

SUBDIRS = widget bedit

AM_CPPFLAGS += \
	-I$(top_builddir) \
	-I$(top_builddir)/src \
	-I$(top_srcdir) \
	-I$(srcdir) \
	-I$(top_srcdir)/src \
	-I$(top_srcdir)/src/zapit/include/zapit \
	-I$(top_srcdir)/lib \
	-I$(top_srcdir)/lib/libeventserver \
	-I$(top_srcdir)/lib/libnet \
	-I$(top_srcdir)/lib/libconfigfile \
	-I$(top_srcdir)/lib/libxmltree \
	-I$(top_srcdir)/lib/libupnpclient \
	@CURL_CFLAGS@ \
	@FREETYPE_CFLAGS@ 

if BOXTYPE_COOLSTREAM
AM_CPPFLAGS += -I$(top_srcdir)/src/zapit/include \
	-I$(top_srcdir)/lib/libcoolstream
else
AM_CPPFLAGS += -I$(top_srcdir)/lib/libdvbapi
endif

if !ENABLE_GSTREAMER
AM_CPPFLAGS += -I$(top_srcdir)/lib/libeplayer3/include
endif

noinst_LIBRARIES = libtimerlist.a libneutrino_gui.a libneutrino_gui2.a

libneutrino_gui_a_SOURCES = \
	bouquetlist.cpp \
	channellist.cpp \
	eventlist.cpp \
	pluginlist.cpp \
	sleeptimer.cpp \
	screensetup.cpp \
	epgview.cpp \
	infoviewer.cpp \
	scan.cpp \
	update.cpp \
	filebrowser.cpp \
	audioplayer.cpp \
	nfs.cpp \
	pictureviewer.cpp \
	movieplayer.cpp \
	rc_lock.cpp \
	timeosd.cpp \
	epgplus.cpp \
	streaminfo2.cpp \
	dboxinfo.cpp \
	plugins.cpp \
	imageinfo.cpp \
	audio_select.cpp \
	moviebrowser.cpp \
	movieinfo.cpp \
	scan_setup.cpp \
	zapit_setup.cpp \
	proxyserver_setup.cpp \
	themes.cpp \
	satip_setup.cpp \
	service_setup.cpp \
	video_setup.cpp \
	audio_setup.cpp \
	parentallock_setup.cpp \
	network_setup.cpp \
	movieplayer_setup.cpp \
	osd_setup.cpp \
	audioplayer_setup.cpp \
	pictureviewer_setup.cpp \
	lcd_setup.cpp \
	rc_setup.cpp \
	recording_setup.cpp \
	misc_setup.cpp \
	main_setup.cpp \
	power_menu.cpp

libneutrino_gui2_a_SOURCES = \
	color.cpp \
	motorcontrol.cpp \
	upnpbrowser.cpp \
	hdd_menu.cpp \
	alphasetup.cpp \
	dvbsub_select.cpp \
	psisetup.cpp \
	opkg_manager.cpp \
	webtv.cpp \
	channel_select.cpp \
	audio_video_select.cpp \
	mediaplayer.cpp

if ENABLE_CI
libneutrino_gui2_a_SOURCES += cam_menu.cpp
endif

libtimerlist_a_SOURCES = timerlist.cpp
