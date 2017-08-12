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

noinst_LTLIBRARIES = fbcallmonitor.la

fbcallmonitor_la_SOURCES = fbcallmonitor.cpp
	 
fbcallmonitor_la_LDFLAGS = -rpath $(PLUGINDIR) -module

fbcallmonitor_la_CXXFLAGS = -fno-rtti -fno-exceptions

install-exec-local:
	install -d $(DESTDIR)$(PLUGINDIR)/fbcallmonitor
	install .libs/fbcallmonitor.so $(DESTDIR)$(PLUGINDIR)/fbcallmonitor
	install -m 0644 fbcallmonitor.cfg $(DESTDIR)$(PLUGINDIR)/fbcallmonitor
	install -m 0644 fritzbox.png $(DESTDIR)$(PLUGINDIR)/fbcallmonitor
	install -m 0755 fb.sh $(DESTDIR)$(PLUGINDIR)/fbcallmonitor
	install -m 0644 fb.csv $(DESTDIR)$(PLUGINDIR)/fbcallmonitor

uninstall-local:
	-rm $(DESTDIR)$(PLUGINDIR)/fbcallmonitor/fbcallmonitor.so
	-rm $(DESTDIR)$(PLUGINDIR)/fbcallmonitor/fbcallmonitor.cfg
	-rm $(DESTDIR)$(PLUGINDIR)/fbcallmonitor/fritzbox.png
	-rm $(DESTDIR)$(PLUGINDIR)/fbcallmonitor/fb.sh
	-rm $(DESTDIR)$(PLUGINDIR)/fbcallmonitor/fb.csv
	-rmdir $(DESTDIR)$(PLUGINDIR)/fbcallmonitor

	




