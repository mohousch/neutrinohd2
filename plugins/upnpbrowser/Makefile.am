SUBDIRS = libupnpclient

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
	-I$(top_srcdir)/upnpbrowser/libupnpclient \
	@FREETYPE_CFLAGS@ 

noinst_LTLIBRARIES = upnpbrowser.la

upnpbrowser_la_SOURCES = upnpbrowser.cpp
	 
upnpbrowser_la_LDFLAGS = -rpath $(PLUGINDIR) -module
upnpbrowser_la_LIBADD = $(top_srcdir)/upnpbrowser/libupnpclient/libtuxbox-upnpclient.a

upnpbrowser_la_CXXFLAGS = -fno-rtti -fno-exceptions

install-exec-local:
	install -d $(DESTDIR)$(PLUGINDIR)/upnpbrowser
	install .libs/upnpbrowser.so $(DESTDIR)$(PLUGINDIR)/upnpbrowser
	install -m 0644 upnpbrowser.cfg $(DESTDIR)$(PLUGINDIR)/upnpbrowser
	install -m 0644 upnpbrowser.png $(DESTDIR)$(PLUGINDIR)/upnpbrowser

uninstall-local:
	-rm $(DESTDIR)$(PLUGINDIR)/upnpbrowser/upnpbrowser.so
	-rm $(DESTDIR)$(PLUGINDIR)/upnpbrowser/upnpbrowser.cfg
	-rm $(DESTDIR)$(PLUGINDIR)/upnpbrowser/upnpbrowser.png
	-rmdir $(DESTDIR)$(PLUGINDIR)/upnpbrowser

	




