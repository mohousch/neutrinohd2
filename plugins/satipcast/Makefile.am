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

noinst_LTLIBRARIES = satipcast.la

satipcast_la_SOURCES = satipcast.cpp
	 
satipcast_la_LDFLAGS = -rpath $(PLUGINDIR) -module

satipcast_la_CXXFLAGS = -fno-rtti -fno-exceptions

install-exec-local:
	install -d $(DESTDIR)$(PLUGINDIR)/satipcast
	install .libs/satipcast.so $(DESTDIR)$(PLUGINDIR)/satipcast
	install -m 0644 satipcast.cfg $(DESTDIR)$(PLUGINDIR)/satipcast
	install -m 0644 satipcast.png $(DESTDIR)$(PLUGINDIR)/satipcast

uninstall-local:
	-rm $(DESTDIR)$(PLUGINDIR)/satipclient/satipcast.so
	-rm $(DESTDIR)$(PLUGINDIR)/satipclient/satipcast.cfg
	-rm $(DESTDIR)$(PLUGINDIR)/satipclient/satipcast.png
	-rmdir $(DESTDIR)$(PLUGINDIR)/satipcast

	




