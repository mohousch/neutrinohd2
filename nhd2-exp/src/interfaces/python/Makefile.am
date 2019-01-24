
AM_CPPFLAGS = \
	-I$(top_builddir) \
	-I$(top_srcdir) \
	-I$(srcdir) \
	-I$(top_srcdir)/src \
	-I$(top_srcdir)/src/zapit/include/zapit \
	-I$(top_srcdir)/src/gui/widget \
	-I$(top_srcdir)/src/interfaces/python \
	-I$(top_srcdir)/lib \
	-I$(top_srcdir)/lib/connection \
	-I$(top_srcdir)/lib/libeventserver \
	-I$(top_srcdir)/lib/libconfigfile \
	-I$(top_srcdir)/lib/libnet \
	-I$(top_srcdir)/lib/libxmltree \
	-I$(top_srcdir)/lib/libmd5sum \
	-I$(top_srcdir)/lib/libupnpclient \
	@FREETYPE_CFLAGS@ \
	@FRIBIDI_CFLAGS@ \
	@PYTHON_CPPFLAGS@

if BOXTYPE_COOLSTREAM
AM_CPPFLAGS += -I$(top_srcdir)/src/zapit/include \
	-I$(top_srcdir)/lib/libcoolstream
else
AM_CPPFLAGS += -I$(top_srcdir)/lib/libdvbapi
if ENABLE_CI
AM_CPPFLAGS += -I$(top_srcdir)/lib/libdvbci
endif
endif

if !ENABLE_GSTREAMER
AM_CPPFLAGS += -I$(top_srcdir)/lib/libeplayer3/include
endif

AM_CPPFLAGS += -fno-rtti -fno-exceptions

noinst_LIBRARIES = libneutrino_interfaces_python.a

libneutrino_interfaces_python_a_SOURCES = \
	nhdpython.cpp

#pkgpython_PYTHON = neutrino.py

EXTRA_DIST = \
	python/neutrino_python.i

.i.cpp:
	$(AM_V_GEN)$(SWIG) $(AX_SWIG_PYTHON_OPT) -MT $@ -MD -MF $(DEPDIR)/$*.Tpo -I$(top_srcdir) -O -nortti -nothreads -o $@ $<
	$(AM_V_at)mv -f $(DEPDIR)/$*.Tpo $(DEPDIR)/$*.Pcpp
	$(AM_V_at)$(PYTHON) python/neutrino.py

python/neutrino.py: python/nhdpython.cpp

CLEANFILES = python/neutrino.py python/nhdpython.cpp

#nobase_install_PYTHON = \
#	python/neutrino.py




