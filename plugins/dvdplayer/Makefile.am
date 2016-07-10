AM_CPPFLAGS =  \
	-I$(srcdir) \
	-I$(top_srcdir)/../nhd2-exp \
	-I$(top_srcdir)/../nhd2-exp/src \
	-I$(top_srcdir)/../nhd2-exp/src/zapit/include/zapit \
	-I$(top_srcdir)/../nhd2-exp/lib \
	-I$(top_srcdir)/../nhd2-exp/lib/connection \
	-I$(top_srcdir)/../nhd2-exp/lib/libeventserver \
	-I$(top_srcdir)/../nhd2-exp/lib/libconfigfile \
	-I$(top_srcdir)/../nhd2-exp/lib/libnet \
	-I$(top_srcdir)/../nhd2-exp/lib/libxmltree \
	-I$(top_srcdir)/../nhd2-exp/lib/libmd5sum \
	-I$(top_srcdir)/../nhd2-exp/lib/libdvbapi \
	-I$(top_srcdir)/../nhd2-exp/lib/libupnpclient \
	@FREETYPE_CFLAGS@ 

noinst_LTLIBRARIES = dvdplayer.la

dvdplayer_la_SOURCES = dvdplayer.cpp
	 
dvdplayer_la_LDFLAGS = -rpath $(PLUGINDIR) -module

dvdplayer_la_CXXFLAGS = -fno-rtti -fno-exceptions

install-exec-local:
	install -d $(DESTDIR)$(PLUGINDIR)/dvdplayer
	install .libs/dvdplayer.so $(DESTDIR)$(PLUGINDIR)/dvdplayer
	install -m 0644 dvdplayer.cfg $(DESTDIR)$(PLUGINDIR)/dvdplayer

uninstall-local:
	-rm $(DESTDIR)$(PLUGINDIR)/dvdplayer/dvdplayer.so
	-rm $(DESTDIR)$(PLUGINDIR)/dvdplayer/dvdplayer.cfg
	-rmdir $(DESTDIR)$(PLUGINDIR)/dvdplayer





