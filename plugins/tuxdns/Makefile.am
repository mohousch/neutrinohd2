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
	@FREETYPE_CFLAGS@ 

noinst_LTLIBRARIES = tuxdns.la

tuxdns_la_SOURCES = tuxdns.cpp
	 
tuxdns_la_LDFLAGS = -rpath $(PLUGINDIR) -module

tuxdns_la_CXXFLAGS = -fno-rtti -fno-exceptions

install-exec-local:
	install -d $(DESTDIR)$(PLUGINDIR)/tuxdns
	install .libs/tuxdns.so $(DESTDIR)$(PLUGINDIR)/tuxdns
	install -m 0644 tuxdns.cfg $(DESTDIR)$(PLUGINDIR)/tuxdns

uninstall-local:
	-rm $(DESTDIR)$(PLUGINDIR)/tuxdns/tuxdns.so
	-rm $(DESTDIR)$(PLUGINDIR)/tuxdns/tuxdns.cfg
	-rmdir $(DESTDIR)$(PLUGINDIR)/tuxdns
	
	




