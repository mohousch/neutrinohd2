################################################################################################################################################################################################################################################################
# Makefile for building neutrinoHD2 for x86		
# build options:
#  --with-configdir=PATH   where to find the config files [PREFIX/var/tuxbox/config]
#  --with-datadir=PATH     where to find data [PREFIX/share/tuxbox]
#  --with-plugindir=PATH   where to find the plugins [PREFIX/var/tuxbox/plugins]
#  --with-dvbincludes=PATH path for dvb includes [NONE]
#  --with-driver=PATH      path for driver sources [NONE]
#  --with-boxtype          valid values: generic,dgs,gigablue,dreambox,xtrend,fulan,kathrein,ipbox,topfield,fortis_hdbox,octagon,atevio,adb_box,whitebox,vip,homecast,vuplus,azbox,technomate,coolstream,hypercube,venton,xp1000,odin,ixuss,iqonios,e3hd,ebox5000
#  --with-boxmodel         valid for dgs: cuberevo,cuberevo_mini,cuberevo_mini2,cuberevo_mini_fta,cuberevo_250hd,cuberevo_2000hd,cuberevo_9500hd
#                                valid for gigablue: gbsolo,gb800se,gb800ue,gb800seplus,gb800ueplus,gbquad
#                                valid for dreambox: dm500, dm500plus, dm600pvr, dm56x0, dm7000, dm7020, dm7025, dm500hd, dm7020hd, dm8000, dm800, dm800se
#                                valid for xtrend: et4x00,et5x00,et6x00,et9x00
#                                valid for fulan: spark, spark7162
#                                valid for kathrein: ufs910, ufs922, ufs912, ufs913, ufc960
#                                valid for ipbox: ipbox55, ipbox99, ipbox9900
#                                valid for atevio: atevio700,atevio7000,atevio7500,atevio7600
#                                valid for octagon: octagon1008
#                                valid for vuplus: vusolo,vuduo,vuuno,vuultimo
#                                valid for azbox: azboxhd,azboxme,azboxminime
#                                valid for technomate: tmtwin,tm2t,tmsingle,tmnano
#                                valid for venton: ventonhde,ventonhdx,inihde,inihdp
#                                valid for ixuss: ixusszero,ixussone
#                                valid for iqonios: iqonios100hd,iqonios300hd,mediabox,optimussos1,optimussos2
#  --enable-keyboard-no-rc enable keyboard control, disable rc control
#  --enable-opengl         include opengl framebuffer support for x86
#  --enable-playback       include enable playback for opengl and satip
#  --enable-libeplayer3    include libeplayer3 as player engine support
#  --enable-gstreamer      include gstreamer as player engine support
#  --enable-lcd            include lcd support
#  --enable-scart          enable scart output
#  --enable-ci             enable ci cam
#  --enable-4digits        include 5 segment lcd support
#  --enable-functionkeys   include RC functions keys support
################################################################################################################################################################################################################################################################

BOXTYPE = generic
DEST = $(PWD)/$(BOXTYPE)

N_SRC  = $(PWD)/nhd2-exp

CFLAGS = -Wall -O2 -fno-strict-aliasing -O0 -g -ggdb3

CXXFLAGS = $(CFLAGS)

export CFLAGS CXXFLAGS

# first target is default...
default: neutrino plugins
	make run

run:
	gdb -ex run $(DEST)/bin/neutrino

neutrino: $(N_SRC)/config.status
	-rm -f $(N_SRC)/src/gui/svn_version.h
	$(MAKE) -C $(N_SRC) install

$(N_SRC)/config.status: | $(N_SRC) $(DEST)
	$(N_SRC)/autogen.sh
	set -e; cd $(N_SRC); \
		$(N_SRC)/configure \
			--prefix=$(DEST)  \
			--build=i686-pc-linux-gnu \
			--enable-maintainer-mode \
			--with-boxtype=$(BOXTYPE) \
			--with-datadir=$(DEST)/share/tuxbox \
			--with-plugindir=$(DEST)/var/tuxbox/plugins \
			--with-configdir=$(DEST)/var/tuxbox/config \
			--enable-opengl \
			--enable-playback \
			--enable-lcd \
			--enable-scart \
			--enable-ci \
			--enable-gstreamer \
			--with-gstversion=0.10 \
			--enable-python \
			--enable-lua
				
$(DEST):
	mkdir $@

$(N_SRC):
	git pull

neutrino-checkout: $(N_SRC)

neutrino-clean:
	-$(MAKE) -C $(N_SRC) clean

neutrino-distclean:
	-$(MAKE) -C $(N_SRC) distclean
	rm -f $(N_SRC)/config.status

# plugins
PLUGINS_SRC = $(PWD)/plugins
$(PLUGINS_SRC):
	git pull

plugins-checkout: $(PLUGINS_SRC)

plugins: $(PLUGINS_SRC)/config.status $(N_SRC)/config.status
	$(MAKE) -C $(PLUGINS_SRC) install

$(PLUGINS_SRC)/config.status: $(PLUGINS_SRC) $(DEST)
	$(PLUGINS_SRC)/autogen.sh
	set -e; cd $(PLUGINS_SRC); \
		$(PLUGINS_SRC)/configure \
			--prefix=$(DEST)  \
			--build=i686-pc-linux-gnu \
			--enable-maintainer-mode \
			--without-debug \
			--with-boxtype=$(BOXTYPE) \
			--with-datadir=$(DEST)/share/tuxbox \
			--with-plugindir=$(DEST)/var/tuxbox/plugins \
			--with-configdir=$(DEST)/var/tuxbox/config \
			--enable-testing

plugins-clean:
	-$(MAKE) -C $(PLUGINS_SRC) clean

plugins-distclean:
	-$(MAKE) -C $(PLUGINS_SRC) distclean
	rm -f $(PLUGINS)/config.status

update:
	git pull

clean: neutrino-clean plugins-clean
distclean: neutrino-distclean plugins-distclean

PHONY = neutrino-checkout plugins-checkout
.PHONY: $(PHONY)
