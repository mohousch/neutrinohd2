/* DVB CI Resource Manager */
#include <stdio.h>

#include "dvbci_resmgr.h"

int eDVBCIResourceManagerSession::receivedAPDU(const unsigned char *tag,const void *data, int len)
{
#if 1
	printf("eDVBCIResourceManagerSession::%s >\n", __func__);
	printf("SESSION(%d) %02x %02x %02x (len = %d): \n", session_nb, tag[0], tag[1], tag[2], len);
#else
	printf("SESSION(%d) %02x %02x %02x: \n", session_nb, tag[0], tag[1], tag[2]); 
#endif
	for (int i=0; i<len; i++)
		printf("%02x ", ((const unsigned char*)data)[i]);
	printf("\n");
	if ((tag[0]==0x9f) && (tag[1]==0x80))
	{
		switch (tag[2])
		{
		case 0x10:  // profile enquiry
			printf("cam fragt was ich kann.");
			state=stateProfileEnquiry;
#if 1
			printf("%s <\n", __func__);
#endif
			return 1;
			break;
		case 0x11: // Tprofile
			printf("mein cam kann: ");
			if (!len)
				printf("nichts\n");
			else
				for (int i=0; i<len; i++)
					printf("%02x ", ((const unsigned char*)data)[i]);

			if (state == stateFirstProfileEnquiry)
			{
#if 1
				printf("%s <\n", __func__);
#endif
				// profile change
				return 1;
			}
			state=stateFinal;
			break;
		default:
			printf("unknown APDU tag 9F 80 %02x\n", tag[2]);
		}
	}
	
#if 1
	printf("%s <\n", __func__);
#endif
	return 0;
}

int eDVBCIResourceManagerSession::doAction()
{
#if 1
	printf("%s >\n", __func__);
#endif
	switch (state)
	{
	case stateStarted:
	{
		const unsigned char tag[3]={0x9F, 0x80, 0x10}; // profile enquiry
		sendAPDU(tag);
		state = stateFirstProfileEnquiry;
#if 1
		printf("%s <\n", __func__);
#endif
		return 0;
	}
	case stateFirstProfileEnquiry:
	{
		const unsigned char tag[3]={0x9F, 0x80, 0x12}; // profile change
		sendAPDU(tag);
		state=stateProfileChange;
#if 1
		printf("%s <\n", __func__);
#endif
		return 0;
	}
	case stateProfileChange:
	{
		printf("bla kaputt\n");
		break;
	}
	case stateProfileEnquiry:
	{
		const unsigned char tag[3]={0x9F, 0x80, 0x11};
		const unsigned char data[][4]=
			{
				{0x00, 0x01, 0x00, 0x41},
				{0x00, 0x02, 0x00, 0x41},
				{0x00, 0x03, 0x00, 0x41},
//				{0x00, 0x20, 0x00, 0x41}, // host control
				{0x00, 0x24, 0x00, 0x41},
				{0x00, 0x40, 0x00, 0x41}
//				{0x00, 0x10, 0x00, 0x41} // auth.
			};
		sendAPDU(tag, data, sizeof(data));
		state=stateFinal;
		return 0;
	}
	case stateFinal:
		printf("stateFinal und action! kann doch garnicht sein ;)\n");
	default:
		break;
	}
#if 1
	printf("%s <\n", __func__);
#endif
	return 0;
}
