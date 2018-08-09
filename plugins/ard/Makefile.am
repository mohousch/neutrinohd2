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

noinst_LTLIBRARIES = ard.la

ard_la_SOURCES = ard.cpp
	 
ard_la_LDFLAGS = -rpath $(PLUGINDIR) -module

ard_la_CXXFLAGS = -fno-rtti -fno-exceptions

install-exec-local:
	install -d $(DESTDIR)$(PLUGINDIR)/ard
	install .libs/ard.so $(DESTDIR)$(PLUGINDIR)/ard
	install -m 0644 ard.cfg $(DESTDIR)$(PLUGINDIR)/ard
	install -m 0644 ard.png $(DESTDIR)$(PLUGINDIR)/ard
	install -m 0644 ard_small.png $(DESTDIR)$(PLUGINDIR)/ard

uninstall-local:
	-rm $(DESTDIR)$(PLUGINDIR)/ard/ard.so
	-rm $(DESTDIR)$(PLUGINDIR)/ard/ard.cfg
	-rm $(DESTDIR)$(PLUGINDIR)/ard/ard.png
	-rm $(DESTDIR)$(PLUGINDIR)/ard/ard_small.png
	-rmdir $(DESTDIR)$(PLUGINDIR)/ard



