/* DVB CI DateTime Manager */
#include <stdio.h>

#include "dvbci_datetimemgr.h"

eDVBCIDateTimeSession::eDVBCIDateTimeSession(tSlot *tslot)
{
#if 1
	printf("%s >\n", __func__);
#endif
	slot = tslot;
	slot->hasDateTime = true;
	slot->pollConnection = true;
#if 1
	printf("%s <\n", __func__);
#endif
}

eDVBCIDateTimeSession::~eDVBCIDateTimeSession()
{
#if 1
	printf("%s >\n", __func__);
#endif
	slot->hasDateTime = false;
#if 1
	printf("%s <\n", __func__);
#endif
}

int eDVBCIDateTimeSession::receivedAPDU(const unsigned char *tag,const void *data, int len)
{
#if 1
	printf("eDVBCIDateTimeSession::%s >\n", __func__);
#endif
	printf("SESSION(%d)/DATETIME %02x %02x %02x: ", session_nb, tag[0],tag[1], tag[2]);
	for (int i=0; i<len; i++)
		printf("%02x ", ((const unsigned char*)data)[i]);
	printf("\n");

	if ((tag[0]==0x9f) && (tag[1]==0x84))
	{
		switch (tag[2])
		{
		case 0x40:
			state=stateSendDateTime;
#if 1
			printf("%s <", __func__);
#endif
			return 1;
			break;
		default:
			printf("unknown APDU tag 9F 84 %02x\n", tag[2]);
			break;
		}
	}
#if 1
	printf("%s <\n", __func__);
#endif
	return 0;
}

int eDVBCIDateTimeSession::doAction()
{
#if 1
	printf("%s >\n", __func__);
#endif
	switch (state)
	{
	case stateStarted:
#if 1
		printf("%s <\n", __func__);
#endif
		return 0;
	case stateSendDateTime:
	{
		unsigned char tag[3]={0x9f, 0x84, 0x41}; // date_time_response
		unsigned char msg[7]={0, 0, 0, 0, 0, 0, 0};
		sendAPDU(tag, msg, 7);
#if 1
		printf("%s <\n", __func__);
#endif
		return 0;
	}
	case stateFinal:
		printf("stateFinal und action! kann doch garnicht sein ;)\n");
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
