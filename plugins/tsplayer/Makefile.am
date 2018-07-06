AM_CPPFLAGS =  \
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
	@FREETYPE_CFLAGS@ 

noinst_LTLIBRARIES = tsplayer.la

tsplayer_la_SOURCES = tsplayer.cpp
	 
tsplayer_la_LDFLAGS = -rpath $(PLUGINDIR) -module

tsplayer_la_CXXFLAGS = -fno-rtti -fno-exceptions

install-exec-local:
	install -d $(DESTDIR)$(PLUGINDIR)/tsplayer
	install .libs/tsplayer.so $(DESTDIR)$(PLUGINDIR)/tsplayer
	install -m 0644 tsplayer.cfg $(DESTDIR)$(PLUGINDIR)/tsplayer
	install -m 0664 tsplayer.png $(DESTDIR)$(PLUGINDIR)/tsplayer

uninstall-local:
	-rm $(DESTDIR)$(PLUGINDIR)/tsplayer/tsplayer.so
	-rm $(DESTDIR)$(PLUGINDIR)/tsplayer/tsplayer.cfg
	-rm $(DESTDIR)$(PLUGINDIR)/tsplayer/tsplayer.png
	-rmdir $(DESTDIR)$(PLUGINDIR)/tsplayer
	




