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

noinst_LTLIBRARIES = restart_enigma.la

restart_enigma_la_SOURCES = restart_enigma.cpp
	 
restart_enigma_la_LDFLAGS = -rpath $(PLUGINDIR) -module

restart_enigma_la_CXXFLAGS = -fno-rtti -fno-exceptions

install-exec-local:
	install -d $(DESTDIR)$(PLUGINDIR)/restart_enigma
	install .libs/restart_enigma.so $(DESTDIR)$(PLUGINDIR)/restart_enigma
	install -m 0644 restart_enigma.cfg $(DESTDIR)$(PLUGINDIR)/restart_enigma

uninstall-local:
	-rm $(DESTDIR)$(PLUGINDIR)/restart_enigma/restart_enigma.so
	-rm $(DESTDIR)$(PLUGINDIR)/restart_enigma/restart_enigma.cfg
	-rmdir $(DESTDIR)$(PLUGINDIR)/restart_enigma
	




