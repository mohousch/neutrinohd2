AM_CPPFLAGS =  \
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

noinst_LTLIBRARIES = nglcd.la

nglcd_la_SOURCES = nglcd.cpp glcdsetup.cpp
	 
nglcd_la_LDFLAGS = -rpath $(PLUGINDIR) -lglcddrivers -lglcdgraphics -lusb -module

nglcd_la_CXXFLAGS = -fno-rtti -fno-exceptions

install-exec-local:
	install -d $(DESTDIR)$(PLUGINDIR)/nglcd
	install .libs/nglcd.so $(DESTDIR)$(PLUGINDIR)/nglcd
	install -m 0644 nglcd.cfg $(DESTDIR)$(PLUGINDIR)/nglcd

uninstall-local:
	-rm $(DESTDIR)$(PLUGINDIR)/nglcd/nglcd.so
	-rm $(DESTDIR)$(PLUGINDIR)/nglcd/nglcd.cfg
	-rmdir $(DESTDIR)$(PLUGINDIR)/nglcd



