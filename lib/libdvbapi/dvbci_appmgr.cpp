/* DVB CI Application Manager */
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <system/debug.h>

#include "dvbci_appmgr.h"

static const char * FILENAME = "[dvbci_appmgr.cpp]";

eDVBCIApplicationManagerSession::eDVBCIApplicationManagerSession(tSlot *tslot)
{
	dprintf(DEBUG_DEBUG, "%s >\n", __func__);

	slot = tslot;

	slot->hasAppManager = true;
	slot->appSession = this;

	dprintf(DEBUG_DEBUG, "%s <\n", __func__);
}

eDVBCIApplicationManagerSession::~eDVBCIApplicationManagerSession()
{
	dprintf(DEBUG_DEBUG, "%s >\n", __func__);

	slot->hasAppManager = false;
	slot->appSession = NULL;

	dprintf(DEBUG_DEBUG, "%s <\n", __func__);
}

int eDVBCIApplicationManagerSession::receivedAPDU(const unsigned char *tag,const void *data, int len)
{
	dprintf(DEBUG_DEBUG, "%s > %s >\n", FILENAME, __func__);

	printf("SESSION(%d)/APP %02x %02x %02x: ", session_nb, tag[0], tag[1], tag[2]);
	for (int i=0; i<len; i++)
		printf("%02x ", ((const unsigned char*)data)[i]);
	printf("\n");

	if ((tag[0]==0x9f) && (tag[1]==0x80))
	{
		switch (tag[2])
		{
		case 0x21:
		{
			int dl;
			printf("application info:\n");
			printf("  len: %d\n", len);
			printf("  application_type: %d\n", ((unsigned char*)data)[0]);
			printf("  application_manufacturer: %02x %02x\n", ((unsigned char*)data)[2], ((unsigned char*)data)[1]);
			printf("  manufacturer_code: %02x %02x\n", ((unsigned char*)data)[4],((unsigned char*)data)[3]);
			printf("  menu string: ");
			dl=((unsigned char*)data)[5];
			if ((dl + 6) > len)
			{
				printf("warning, invalid length (%d vs %d)\n", dl+6, len);
				dl=len-6;
			}
			char str[dl + 1];
			memcpy(str, ((char*)data) + 6, dl);
			str[dl] = '\0';
			for (int i = 0; i < dl; ++i)
				printf("%c", ((unsigned char*)data)[i+6]);
			printf("\n");

			strcpy(slot->name, str);
			printf("set name %s on slot %d, %p\n", slot->name, slot->slot, slot);
			break;
		}
		default:
			printf("unknown APDU tag 9F 80 %02x\n", tag[2]);
			break;
		}
	}
	dprintf(DEBUG_DEBUG, "%s <", __func__);
	return 0;
}

int eDVBCIApplicationManagerSession::doAction()
{
	dprintf(DEBUG_DEBUG, "%s >", __func__);
	switch (state)
	{
		case stateStarted:
		{
	    		const unsigned char tag[3]={0x9F, 0x80, 0x20}; // application manager info e    sendAPDU(tag);
			sendAPDU(tag);
			state=stateFinal;
			checkBlist();
			dprintf(DEBUG_DEBUG, "%s <", __func__);
			return 1;
		}
		case stateFinal:
			dprintf(DEBUG_DEBUG, "in final state.");
			wantmenu = 0;
			if (wantmenu)
			{
				printf("wantmenu: sending Tenter_menu\n");
				const unsigned char tag[3]={0x9F, 0x80, 0x22};  // Tenter_menu
				sendAPDU(tag);
				wantmenu=0;
				dprintf(DEBUG_DEBUG, "%s <\n", __func__);
				return 0;
			} else
				return 0;
		default:
			dprintf(DEBUG_DEBUG, "%s <\n", __func__);
			return 0;
	}
	dprintf(DEBUG_DEBUG, "%s <\n", __func__);
}

int eDVBCIApplicationManagerSession::startMMI()
{
	dprintf(DEBUG_INFO, "%s > %s >\n", FILENAME, __func__);
	const unsigned char tag[3]={0x9F, 0x80, 0x22};  // Tenter_menu
	sendAPDU(tag);
	slot->mmiOpened = true;
	dprintf(DEBUG_DEBUG, "%s > %s <\n", FILENAME, __func__);
	return 0;
}

bool eDVBCIApplicationManagerSession::readBlist()
{
	int rc, i;
	char cSid[4] = {0,0,0,0};
	uint16_t Sid;
	FILE *fd;
	char blacklist_file[32];

	sprintf(blacklist_file,"/etc/blacklist_slot_%d",slot->slot);

	if (access(blacklist_file, F_OK) != 0)
		return false;
	fd = fopen(blacklist_file,"r");
	if (!fd)
		return false;
	else
	{
		do
		{
			for (i = 0; i < 4; i++)
			{
				rc = fgetc(fd);
				if (rc == ',' || rc == EOF) break;
				cSid[i] = (char)rc;
			}
			if (rc == EOF) goto fin;
			if (i == 4)
			{
				Sid = (uint16_t)strtol(cSid, NULL, 16);
				slot->bsids.push_back(Sid);
			}
		} while (rc != EOF);
fin:
		fclose(fd);
	}
	if (slot->bsids.size())
		return true;
	else
		return false;
}

int eDVBCIApplicationManagerSession::checkBlist()
{
	if (readBlist())
	{
/* out commented: causes sometimes segfault when reboot....don't know why :( */
#if 1
		printf("Blacked sids: %d > ", slot->bsids.size());
		for (unsigned int i = 0; i < slot->bsids.size(); i++)
			printf("%04x ", slot->bsids[i]);
		printf("\n");
#endif
	}
	return 0;
}

