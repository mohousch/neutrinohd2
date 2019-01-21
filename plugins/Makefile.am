AUTOMAKE_OPTIONS = gnu

SUBDIRS = \
	test \
	netzkino \
	youtube \
	systeminfo \
	tuxcom \
	mediaplayer \
	fileexplorer \
	mediaportal \
	icecast \
	upnp \
	internetradio \
	tsbrowser \
	moviebrowser \
	picviewer \
	audioplayer \
	nfilm \
	ntvshows \
	python

if ENABLE_NGLCD
SUBDIRS += nglcd
endif

if ENABLE_TUXCAL
SUBDIRS += tuxcal
endif

#testing
if ENABLE_TESTING
SUBDIRS += ard \
	fbcallmonitor \
	tuxdns \
	restart_enigma \
	dvdplayer \
	satipclient \
	opkg_manager
endif


