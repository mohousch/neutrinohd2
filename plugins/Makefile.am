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

if ENABLE_NGLCD
SUBDIRS += nglcd
endif

if ENABLE_TUXCAL
SUBDIRS += tuxcal
endif

#testing
if ENABLE_TESTING
SUBDIRS += ard \
	restart_enigma \
	satipclient \
	opkg_manager
endif

if ENABLE_PYTHON
SUBDIRS += python
endif

if ENABLE_LUA
SUBDIRS += lua
endif




