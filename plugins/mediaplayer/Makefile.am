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

noinst_LTLIBRARIES = mediaplayer.la

mediaplayer_la_SOURCES = mediaplayer.cpp
	 
mediaplayer_la_LDFLAGS = -rpath $(PLUGINDIR) -module

mediaplayer_la_CXXFLAGS = -fno-rtti -fno-exceptions

install-exec-local:
	install -d $(DESTDIR)$(PLUGINDIR)/mediaplayer
	install .libs/mediaplayer.so $(DESTDIR)$(PLUGINDIR)/mediaplayer
	install -m 0644 mediaplayer.cfg $(DESTDIR)$(PLUGINDIR)/mediaplayer

uninstall-local:
	-rm $(DESTDIR)$(PLUGINDIR)/mediaplayer/mediaplayer.so
	-rm $(DESTDIR)$(PLUGINDIR)/mediaplayer/mediaplayer.cfg
	-rmdir $(DESTDIR)$(PLUGINDIR)/mediaplayer





