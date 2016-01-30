SUBDIRS = zapit gui daemonc driver system timerd nhttpd sectionsd

AM_CPPFLAGS = \
	-I$(top_srcdir) \
	-I$(srcdir) \
	-I$(top_srcdir)/src/zapit/include/zapit \
	-I$(top_srcdir)/lib \
	-I$(top_srcdir)/lib/connection \
	-I$(top_srcdir)/lib/libeventserver \
	-I$(top_srcdir)/lib/libconfigfile \
	-I$(top_srcdir)/lib/libnet \
	-I$(top_srcdir)/lib/libxmltree \
	-I$(top_srcdir)/lib/libmd5sum \
	-I$(top_srcdir)/lib/libupnpclient \
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

bin_PROGRAMS = neutrino

neutrino_SOURCES = neutrino_menue.cpp neutrino.cpp

neutrino_LDADD = \
	daemonc/libneutrino_daemonc.a \
	gui/bedit/libneutrino_gui_bedit.a \
	gui/libtimerlist.a \
	gui/libneutrino_gui.a \
	gui/widget/libneutrino_gui_widget.a \
	gui/widget/libneutrino_gui_widget2.a \
	driver/pictureviewer/libneutrino_pictureviewer.a \
	system/libneutrino_system.a \
	driver/libneutrino_driver.a \
	gui/movieinfo.o \
	driver/audiodec/libneutrino_driver_audiodec.a \
	gui/bedit/bouqueteditor_bouquets.o \
	gui/libneutrino_gui2.a \
	sectionsd/libsectionsd.a \
	timerd/libtimerd.a \
	zapit/src/libzapit.a \
	nhttpd/libnhttpd.a \
	nhttpd/tuxboxapi/libnhttpd_tuxboxapi.a \
	nhttpd/yhttpd_mods/libyhttpdmods.a \
	nhttpd/yhttpd_core/libyhttpd.a \
	gui/video_setup.o \
	gui/widget/items2detailsline.o \
	$(top_builddir)/lib/sectionsdclient/libsectionsdclient.a \
	$(top_builddir)/lib/timerdclient/libtimerdclient.a \
	$(top_builddir)/src/zapit/lib/libzapitclient.a \
	$(top_builddir)/lib/libconfigfile/libtuxbox-configfile.a \
	$(top_builddir)/lib/connection/libtuxbox-connection.a \
	$(top_builddir)/lib/libeventserver/libtuxbox-eventserver.a \
	$(top_builddir)/lib/libxmltree/libtuxbox-xmltree.a \
	$(top_builddir)/lib/libnet/libtuxbox-net.a \
	$(top_builddir)/lib/libmd5sum/libtuxbox-md5sum.a \
	$(top_builddir)/lib/libdvbsub/libdvbsub.a \
	$(top_builddir)/lib/libtuxtxt/libtuxtxt.a \
	$(top_builddir)/lib/libupnpclient/libtuxbox-upnpclient.a \
	$(top_builddir)/lib/jsoncpp/libjsoncpp.a \
	@CURL_LIBS@ \
	@FREETYPE_LIBS@ \
	@PNG_LIBS@ \
	@MAD_LIBS@ \
	@ID3TAG_LIBS@ \
	-ljpeg -lrt -lpthread -lblkid -ldl -lFLAC -lavformat -lavcodec -lavutil 

neutrino_LDFLAGS = -Wl,--export-dynamic

if !BOXTYPE_COOLSTREAM
neutrino_LDADD += $(top_builddir)/lib/libdvbapi/libdvbapi.a
endif

if BOXTYPE_COOLSTREAM
neutrino_LDADD += \
	-lcoolstream-mt \
	-lnxp \
	-lOpenThreads
endif

if ENABLE_GSTREAMER
neutrino_LDADD += -lgstreamer-0.10 -lgobject-2.0 -lglib-2.0
else
neutrino_LDADD += $(top_builddir)/lib/libeplayer3/libeplayer3.a
endif

# LCD display
if ENABLE_LCD
neutrino_LDADD += $(top_builddir)/lib/liblcddisplay/liblcddisplay.a
endif

if USE_OPENGL
neutrino_LDADD += \
	-lglut -lGL -lGLU -lGLEW -lOpenThreads -lgstinterfaces-0.10 
endif

if ENABLE_FRIBIDI
neutrino_LDADD += -lfribidi
endif







	



