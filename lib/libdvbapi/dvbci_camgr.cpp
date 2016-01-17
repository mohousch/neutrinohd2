/* DVB CI CA Manager */
#include <stdio.h>
#include <stdint.h>

#include "dvbci_camgr.h"

#include <algorithm>

eDVBCICAManagerSession::eDVBCICAManagerSession(tSlot *tslot)
{
#if 1
	printf("%s >\n", __func__);
#endif
	slot = tslot;
#if 1
	printf("%s <\n", __func__);
#endif
}

eDVBCICAManagerSession::~eDVBCICAManagerSession()
{
#if 1
	printf("%s >\n", __func__);
#endif
	slot->hasCAManager = false;
        slot->camgrSession = NULL;
#if 1
	printf("%s <\n", __func__);
#endif
}

int eDVBCICAManagerSession::receivedAPDU(const unsigned char *tag, const void *data, int len)
{
#if 1
	printf("eDVBCICAManagerSession::%s >\n", __func__);
#endif
	printf("SESSION(%d)/CA %02x %02x %02x: ", session_nb, tag[0], tag[1],tag[2]);
	for (int i=0; i<len; i++)
		printf("%02x ", ((const unsigned char*)data)[i]);
	printf("\n");

	if ((tag[0]==0x9f) && (tag[1]==0x80))
	{
		switch (tag[2])
		{
		case 0x31:
			printf("ca info:\n");
			for (int i=0; i<len; i+=2)
			{
				printf("%04x ", (((const unsigned char*)data)[i]<<8)|(((const unsigned char*)data)[i+1]));
				caids.push_back((((const unsigned char*)data)[i]<<8)|(((const unsigned char*)data)[i+1]));
			}
			sort(caids.begin(), caids.end());
			printf("\n");
			
			slot->pollConnection = false;
	                slot->hasCAManager = true;
                        slot->camgrSession = this;
			
			//fixme eDVBCIInterfaces::getInstance()->recheckPMTHandlers();
			break;
		default:
			printf("unknown APDU tag 9F 80 %02x\n", tag[2]);
			break;
		}
	}
#if 1
	printf("%s <\n", __func__);
#endif
	return 0;
}

int eDVBCICAManagerSession::doAction()
{
#if 1
	printf("%s >\n", __func__);
#endif
	switch (state)
	{
	case stateStarted:
	{
		const unsigned char tag[3]={0x9F, 0x80, 0x30}; // ca info enq
		sendAPDU(tag);
		state=stateFinal;
#if 1
		printf("%s <", __func__);
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

int eDVBCICAManagerSession::sendCAPMT(unsigned char *data, int len)
{
	const unsigned char tag[3]={0x9F, 0x80, 0x32}; // ca_pmt

#if 1
	printf("%s >\n", __func__);
#endif
	sendAPDU(tag, data, len);

#if 1
	printf("%s <\n", __func__);
#endif
	return 0;
}

