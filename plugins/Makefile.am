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
	ntvshows

#testing
if ENABLE_TESTING
SUBDIRS += ard \
	satipclient \
	opkg_manager \
	tuxcal
endif

if ENABLE_PYTHON
SUBDIRS += python
endif

if ENABLE_LUA
SUBDIRS += lua \
	Wunderschön
endif




