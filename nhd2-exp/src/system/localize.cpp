/*
	Neutrino-GUI  -   DBoxII-Project

	Copyright (C) 2001 Steffen Hehn 'McClean'
	Homepage: http://dbox.cyberphoria.org/

	Kommentar:

	Diese GUI wurde von Grund auf neu programmiert und sollte nun vom
	Aufbau und auch den Ausbaumoeglichkeiten gut aussehen. Neutrino basiert
	auf der Client-Server Idee, diese GUI ist also von der direkten DBox-
	Steuerung getrennt. Diese wird dann von Daemons uebernommen.


	License: GPL

	This program is free software; you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation; either version 2 of the License, or
	(at your option) any later version.

	This program is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
	GNU General Public License for more details.

	You should have received a copy of the GNU General Public License
	along with this program; if not, write to the Free Software
	Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
*/


#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <stdio.h>
#include <stdlib.h>

#include <system/localize.h>
#include <system/locals_intern.h>

#include <cstring>
#include <fstream>
#include <string>

#include <iostream>
#include <map>


static const char * iso639filename = DATADIR "/neutrino/iso-codes/iso-639.tab";

std::map<std::string, std::string> iso639;
std::map<std::string, std::string> iso639rev;

void initialize_iso639_map(void)
{
	std::string s, t, u, v;
	std::ifstream in(iso639filename);
	
	if (in.is_open())
	{
		while (in.peek() == '#')
			getline(in, s);
		while (in >> s >> t >> u >> std::ws)
		{
			getline(in, v);
			iso639[s] = v;
			if (s != t) {
				iso639[t] = v;
			}
			iso639rev[v] = s;
		}
	}
	else
		std::cout << "Loading " << iso639filename << " failed." << std::endl;
}

const char * getISO639Description(const char * const iso)
{
	std::map<std::string, std::string>::const_iterator it = iso639.find(std::string(iso));

	if (it == iso639.end())
		return iso;
	else
		return it->second.c_str();
}

CLocaleManager::CLocaleManager()
{
	localeData = new char * [sizeof(locale_real_names)/sizeof(const char *)];

	for (unsigned int i = 0; i < (sizeof(locale_real_names)/sizeof(const char *)); i++)
		localeData[i] = (char *)locale_real_names[i];
}

CLocaleManager::~CLocaleManager()
{
	for (unsigned j = 0; j < (sizeof(locale_real_names)/sizeof(const char *)); j++)
		if (localeData[j] != locale_real_names[j])
			free(localeData[j]);

	delete localeData;
}

const char * path[2] = {CONFIGDIR "/locale/", DATADIR "/neutrino/locale/"};

CLocaleManager::loadLocale_ret_t CLocaleManager::loadLocale(const char * const locale)
{
	unsigned int i;
	FILE * fd;

	initialize_iso639_map();

	for (i = 0; i < 2; i++)
	{
		std::string filename = path[i];
		filename += locale;
		filename += ".locale";
		
		fd = fopen(filename.c_str(), "r");
		if (fd)
			break;
	}
	
	if (i == 2)
	{		
		perror("cannot read locale");
		return NO_SUCH_LOCALE;
	}

	for (unsigned j = 0; j < (sizeof(locale_real_names)/sizeof(const char *)); j++)
	{
		if (localeData[j] != locale_real_names[j])
		{
			free(localeData[j]);
			localeData[j] = (char *)locale_real_names[j];
		}
	}

	char buf[1000];

	i = 1;

	while(!feof(fd))
	{
		if(fgets(buf, sizeof(buf), fd) != NULL)
		{
			char * val    = NULL;
			char * tmpptr = buf;

			for(; (*tmpptr!=10) && (*tmpptr!=13); tmpptr++)
			{
				if ((*tmpptr == ' ') && (val == NULL))
				{
					*tmpptr  = 0;
					val = tmpptr + 1;
				}
			}
			*tmpptr = 0;

			if (val == NULL)
				continue;

			std::string text = val;

			int pos;
			do
			{
				pos = text.find("\\n");

				if ( pos != -1 )
				{
					text.replace(pos, 2, "\n", 1);
				}
			} while ( ( pos != -1 ) );

			for(i = 1; i < sizeof(locale_real_names)/sizeof(const char *); i++)
			{
				//printf("[%s] [%s]\n", buf,locale_real_names[i]);
				
				if(!strcmp(buf, locale_real_names[i]))
				{
					if(localeData[i] == locale_real_names[i])
						localeData[i] = strdup(text.c_str());
					else
						printf("[%s.locale] dup entry: %s\n", locale, locale_real_names[i]);
					break;
				}
			}

			//printf("i=%d\n", i);
			if(i == sizeof(locale_real_names)/sizeof(const char *))
				printf("[%s.locale] superfluous entry: %s\n", locale, buf);
		}
	}

	fclose(fd);
	
	for (unsigned j = 1; j < (sizeof(locale_real_names)/sizeof(const char *)); j++)
	{
		if (localeData[j] == locale_real_names[j])
		{
			printf("[%s.locale] missing entry: %s\n", locale, locale_real_names[j]);
		}
	}

	return (
		(strcmp(locale, "bosanski") == 0) ||
		(strcmp(locale, "ellinika") == 0) ||
		(strcmp(locale, "russkij") == 0) ||
		(strcmp(locale, "utf8") == 0)
		/* utf8.locale is a generic name that can be used for new locales which need characters outside the ISO-8859-1 character set */
		) ? UNICODE_FONT : ISO_8859_1_FONT;
}

const char * CLocaleManager::getText(const neutrino_locale_t keyName) const
{
	return localeData[keyName];
}

static const neutrino_locale_t locale_weekday[7] =
{
	LOCALE_DATE_SUN,
	LOCALE_DATE_MON,
	LOCALE_DATE_TUE,
	LOCALE_DATE_WED,
	LOCALE_DATE_THU,
	LOCALE_DATE_FRI,
	LOCALE_DATE_SAT
};

static const neutrino_locale_t locale_month[12] =
{
	LOCALE_DATE_JAN,
	LOCALE_DATE_FEB,
	LOCALE_DATE_MAR,
	LOCALE_DATE_APR,
	LOCALE_DATE_MAY,
	LOCALE_DATE_JUN,
	LOCALE_DATE_JUL,
	LOCALE_DATE_AUG,
	LOCALE_DATE_SEP,
	LOCALE_DATE_OCT,
	LOCALE_DATE_NOV,
	LOCALE_DATE_DEC
};

neutrino_locale_t CLocaleManager::getMonth(const struct tm * struct_tm_p)
{
	return locale_month[struct_tm_p->tm_mon];
}

neutrino_locale_t CLocaleManager::getWeekday(const struct tm * struct_tm_p)
{
	return locale_weekday[struct_tm_p->tm_wday];
}

//
CLocaleManager::loadLocale_ret_t CLocaleManager::loadCustomLocale(const char * const locale, const char ** custom_locale_real_names, const size_t length, const char * path)
{
	//
	customData = new char * [length];

	for (unsigned int i = 0; i < length; i++)
		customData[i] = (char *)custom_locale_real_names[i];

	//
	unsigned int i;
	FILE * fd;

	initialize_iso639_map();

	//
	std::string filename = path;
	filename += locale;
	filename += ".locale";
		
	fd = fopen(filename.c_str(), "r");
	
	if (!fd)
	{		
		perror("cannot read locale");
		return NO_SUCH_LOCALE;
	}

	for (unsigned j = 0; j < length; j++)
	{
		if (customData[j] != custom_locale_real_names[j])
		{
			free(customData[j]);
			customData[j] = (char *)custom_locale_real_names[j];
		}
	}

	char buf[1000];

	i = 1;

	while(!feof(fd))
	{
		if(fgets(buf, sizeof(buf), fd) != NULL)
		{
			char * val    = NULL;
			char * tmpptr = buf;

			for(; (*tmpptr != 10) && (*tmpptr != 13); tmpptr++)
			{
				if ((*tmpptr == ' ') && (val == NULL))
				{
					*tmpptr  = 0;
					val = tmpptr + 1;
				}
			}
			*tmpptr = 0;

			if (val == NULL)
				continue;

			std::string text = val;

			int pos;
			do
			{
				pos = text.find("\\n");

				if ( pos != -1 )
				{
					text.replace(pos, 2, "\n", 1);
				}
			} while ( ( pos != -1 ) );


			for(i = 1; i < length; i++)
			{
				if(!strcmp(buf, custom_locale_real_names[i]))
				{
					if(customData[i] == custom_locale_real_names[i])
						customData[i] = strdup(text.c_str());
					else
						printf("[%s.locale] dup entry: %s\n", locale, custom_locale_real_names[i]);
					break;
				}
			}

			if(i == length)
				printf("[%s.locale] superfluous entry: %s\n", locale, buf);
		}
	}

	fclose(fd);
	
	for (unsigned j = 1; j < length; j++)
	{
		if (customData[j] == custom_locale_real_names[j])
		{
			printf("[%s.locale] missing entry: %s\n", locale, custom_locale_real_names[j]);
		}
	}

	return (
		(strcmp(locale, "bosanski") == 0) ||
		(strcmp(locale, "ellinika") == 0) ||
		(strcmp(locale, "russkij") == 0) ||
		(strcmp(locale, "utf8") == 0)
		/* utf8.locale is a generic name that can be used for new locales which need characters outside the ISO-8859-1 character set */
		) ? UNICODE_FONT : ISO_8859_1_FONT;
}

void CLocaleManager::unloadCustomLocale(const char **custom_locale_real_names, const size_t length)
{
	for (unsigned j = 0; j < length; j++)
		if (customData[j] != custom_locale_real_names[j])
			free(customData[j]);

	delete customData;
}

const char * CLocaleManager::getCustomText(const neutrino_locale_t keyName) const
{
	return customData[keyName];
}

