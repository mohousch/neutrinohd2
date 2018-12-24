AUTOMAKE_OPTIONS = gnu

SUBDIRS = \
	test \
	netzkino \
	fbcallmonitor \
	youtube \
	systeminfo \
	tuxdns \
	restart_enigma \
	tuxcom \
	dvdplayer \
	mediaplayer \
	fileexplorer \
	mediaportal \
	satipclient \
	icecast \
	upnpbrowser \
	internetradio \
	tsbrowser \
	moviebrowser \
	picviewer \
	audioplayer \
	opkg_manager \
	nfilm \
	ntvshows

if ENABLE_NGLCD
SUBDIRS += nglcd
endif

if ENABLE_TUXCAL
SUBDIRS += tuxcal
endif

#testing
if ENABLE_TESTING
SUBDIRS += upnp \
	ard
endif


