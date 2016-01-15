/* DVB CI CA Manager */
#include <stdio.h>
#include <stdint.h>
#include <system/debug.h>

#include "dvbci_camgr.h"

#include <algorithm>

eDVBCICAManagerSession::eDVBCICAManagerSession(tSlot *tslot)
{
	dprintf(DEBUG_DEBUG, "%s >\n", __func__);
	slot = tslot;
	dprintf(DEBUG_DEBUG, "%s <\n", __func__);
}

eDVBCICAManagerSession::~eDVBCICAManagerSession()
{
	dprintf(DEBUG_DEBUG, "%s >\n", __func__);
	slot->hasCAManager = false;
        slot->camgrSession = NULL;
	dprintf(DEBUG_DEBUG, "%s <\n", __func__);
}

int eDVBCICAManagerSession::receivedAPDU(const unsigned char *tag, const void *data, int len)
{
	dprintf(DEBUG_DEBUG, "eDVBCICAManagerSession::%s >\n", __func__);

	dprintf(DEBUG_NORMAL, "SESSION(%d)/CA %02x %02x %02x: ", session_nb, tag[0], tag[1],tag[2]);
	for (int i=0; i<len; i++)
		dprintf(DEBUG_NORMAL, "%02x ", ((const unsigned char*)data)[i]);
	dprintf(DEBUG_NORMAL, "\n");

	if ((tag[0]==0x9f) && (tag[1]==0x80))
	{
		switch (tag[2])
		{
		case 0x31:
			dprintf(DEBUG_INFO, "ca info: ");
			for (int i=0; i<len; i+=2)
			{
				dprintf(DEBUG_INFO, "%04x ", (((const unsigned char*)data)[i]<<8)|(((const unsigned char*)data)[i+1]));
				caids.push_back((((const unsigned char*)data)[i]<<8)|(((const unsigned char*)data)[i+1]));
			}
			dprintf(DEBUG_INFO, "\n");

			sort(caids.begin(), caids.end());
			slot->pollConnection = false;
	                slot->hasCAManager = true;
                        slot->camgrSession = this;
			
			//fixme eDVBCIInterfaces::getInstance()->recheckPMTHandlers();
			break;
		default:
			dprintf(DEBUG_NORMAL, "unknown APDU tag 9F 80 %02x\n", tag[2]);
			break;
		}
	}
	dprintf(DEBUG_DEBUG, "%s <\n", __func__);
	return 0;
}

int eDVBCICAManagerSession::doAction()
{
	dprintf(DEBUG_DEBUG, "%s >\n", __func__);
	switch (state)
	{
		case stateStarted:
		{
			const unsigned char tag[3]={0x9F, 0x80, 0x30}; // ca info enq
			sendAPDU(tag);
			state=stateFinal;
			dprintf(DEBUG_DEBUG, "%s <", __func__);
			return 0;
		}
		case stateFinal:
			printf("stateFinal und action! kann doch garnicht sein ;)\n");
		default:
			dprintf(DEBUG_DEBUG, "%s <\n", __func__);
			return 0;
	}
	dprintf(DEBUG_DEBUG, "%s <\n", __func__);
}

/* nowhere used ? */
int eDVBCICAManagerSession::sendCAPMT(unsigned char *data, int len)
{
	const unsigned char tag[3]={0x9F, 0x80, 0x32}; // ca_pmt
	dprintf(DEBUG_DEBUG, "%s >\n", __func__);
	sendAPDU(tag, data, len);
	dprintf(DEBUG_DEBUG, "%s <\n", __func__);
	return 0;
}

