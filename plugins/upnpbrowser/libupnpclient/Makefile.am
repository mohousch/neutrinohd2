noinst_LIBRARIES = libtuxbox-upnpclient.a

AM_CPPFLAGS = \
	-I$(top_srcdir)/../nhd2-exp/lib/libxmltree \
	-I$(top_srcdir)/../nhd2-exp/src \
	-I$(top_srcdir)/upnpbrowser/libupnpclient

AM_CPPFLAGS += -fno-rtti -fPIC

libtuxbox_upnpclient_a_SOURCES = UPNPSocket.cpp UPNPDevice.cpp UPNPService.cpp
