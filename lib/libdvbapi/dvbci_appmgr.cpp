/* DVB CI Application Manager */
#include <stdio.h>
#include <string.h>

#include "dvbci_appmgr.h"

eDVBCIApplicationManagerSession::eDVBCIApplicationManagerSession(tSlot *tslot)
{
#if 1
	printf("%s >\n", __func__);
#endif
	slot = tslot;
#if 1
	slot->hasAppManager = true;
	slot->appSession = this;

	printf("%s <\n", __func__);
#endif
}

eDVBCIApplicationManagerSession::~eDVBCIApplicationManagerSession()
{
#if 1
	printf("%s >\n", __func__);
#endif
#if 1
	slot->hasAppManager = false;
	slot->appSession = NULL;

	printf("%s <\n", __func__);
#endif
}

int eDVBCIApplicationManagerSession::receivedAPDU(const unsigned char *tag,const void *data, int len)
{
#if 1
	printf("eDVBCIApplicationManagerSession::%s >\n", __func__);
#endif
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
			printf("  menu string: \n");
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
#if 1
	printf("%s <", __func__);
#endif
	return 0;
}

int eDVBCIApplicationManagerSession::doAction()
{
#if 1
	printf("%s >", __func__);
#endif
  switch (state)
  {
  case stateStarted:
  {
    const unsigned char tag[3]={0x9F, 0x80, 0x20}; // application manager info e    sendAPDU(tag);
    
    sendAPDU(tag);
    state=stateFinal;
#if 1
	printf("%s <", __func__);
#endif
    return 1;
  }
  case stateFinal:
    printf("in final state.");
    
    wantmenu = 0;
    
    if (wantmenu)
    {
      printf("wantmenu: sending Tenter_menu\n");
      const unsigned char tag[3]={0x9F, 0x80, 0x22};  // Tenter_menu
      sendAPDU(tag);
      wantmenu=0;
#if 1
	printf("%s <\n", __func__);
#endif
      return 0;
    } else
      return 0;
  default:
#if 1
	printf("%s <\n", __func__);
#endif
    return 0;
  }
#if 1
	printf("%s <\n", __func__);
#endif
}

int eDVBCIApplicationManagerSession::startMMI()
{
#if 1
	printf("%s >\n", __func__);
#endif
	printf("in appmanager -> startmmi()\n");
	const unsigned char tag[3]={0x9F, 0x80, 0x22};  // Tenter_menu
	sendAPDU(tag);

	slot->mmiOpened = true;

#if 1
	//fixme slot->mmiOpened();
	printf("%s <\n", __func__);
#endif
	return 0;
}

