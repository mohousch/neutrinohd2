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

noinst_LTLIBRARIES = movieplayer.la

movieplayer_la_SOURCES = movieplayer.cpp
	 
movieplayer_la_LDFLAGS = -rpath $(PLUGINDIR) -module

movieplayer_la_CXXFLAGS = -fno-rtti -fno-exceptions

install-exec-local:
	install -d $(DESTDIR)$(PLUGINDIR)/movieplayer
	install .libs/movieplayer.so $(DESTDIR)$(PLUGINDIR)/movieplayer
	install -m 0644 movieplayer.cfg $(DESTDIR)$(PLUGINDIR)/movieplayer
	install -m 0644 movieplayer.png $(DESTDIR)$(PLUGINDIR)/movieplayer

uninstall-local:
	-rm $(DESTDIR)$(PLUGINDIR)/movieplayer/movieplayer.so
	-rm $(DESTDIR)$(PLUGINDIR)/movieplayer/movieplayer.cfg
	-rm $(DESTDIR)$(PLUGINDIR)/movieplayer/movieplayer.png
	-rmdir $(DESTDIR)$(PLUGINDIR)/movieplayer
	




