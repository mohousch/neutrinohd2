AM_CPPFLAGS = \
	-I$(top_builddir) \
	-I$(srcdir) \
	-I$(top_srcdir) \
	-I$(top_srcdir)/src \
	-I$(top_srcdir)/src/zapit/include/zapit \
	-I$(top_srcdir)/lib \
	-I$(top_srcdir)/lib/libnet \
	-I$(top_srcdir)/lib/libconfigfile \
	-I$(top_srcdir)/lib/libxmltree \
	@FREETYPE_CFLAGS@

if BOXTYPE_COOLSTREAM
AM_CPPFLAGS += -I$(top_srcdir)/src/zapit/include \
	-I$(top_srcdir)/lib/libcoolstream
else
AM_CPPFLAGS += -I$(top_srcdir)/lib/libdvbapi
endif

noinst_LIBRARIES = libneutrino_gui_widget.a libneutrino_gui_widget2.a 

libneutrino_gui_widget_a_SOURCES = \
	drawable.cpp \
	buttons.cpp \
	items2detailsline.cpp \
	menue.cpp \
	stringinput.cpp \
	stringinput_ext.cpp \
	infobox.cpp \
	messagebox.cpp \
	hintbox.cpp \
	helpbox.cpp \
	listbox.cpp \
	textbox.cpp \
	listframe.cpp \
	window.cpp

libneutrino_gui_widget2_a_SOURCES = \
	colorchooser.cpp \
	keychooser.cpp \
	mountchooser.cpp \
	vfdcontroler.cpp \
	progresswindow.cpp \
	progressbar.cpp



