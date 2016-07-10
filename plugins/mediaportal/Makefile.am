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

noinst_LTLIBRARIES = mediaportal.la

mediaportal_la_SOURCES = mediaportal.cpp
	 
mediaportal_la_LDFLAGS = -rpath $(PLUGINDIR) -module

mediaportal_la_CXXFLAGS = -fno-rtti -fno-exceptions

install-exec-local:
	install -d $(DESTDIR)$(PLUGINDIR)/mediaportal
	install .libs/mediaportal.so $(DESTDIR)$(PLUGINDIR)/mediaportal
	install -m 0644 mediaportal.cfg $(DESTDIR)$(PLUGINDIR)/mediaportal
	install -m 0644 mp.png $(DESTDIR)$(PLUGINDIR)/mediaportal
	install -m 0644 musicdeluxe.png $(DESTDIR)$(PLUGINDIR)/mediaportal

uninstall-local:
	-rm $(DESTDIR)$(PLUGINDIR)/mediaportal/mediaportal.so
	-rm $(DESTDIR)$(PLUGINDIR)/mediaportal/mediaportal.cfg
	-rm $(DESTDIR)$(PLUGINDIR)/mediaportal/mp.png
	-rm $(DESTDIR)$(PLUGINDIR)/mediaportal/musicdeluxe.png
	-rmdir $(DESTDIR)$(PLUGINDIR)/mediaportal



