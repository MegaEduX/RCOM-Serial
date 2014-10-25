/*
 * RCOM - Link Layer
 * Grupo XXX
 */

#ifndef __LINK_LAYER__
#define __LINK_LAYER__

typedef struct LinkLayer {
	
	char port[20];
	int baudRate;

	unsigned int sequenceNumber;
	unsigned int timeout;
	unsigned int maxRetries;
	
} LinkLayer;

LinkLayer *linkLayerInstance;

void llsetup(char *port, int baudRate, unsigned int sequenceNumber, unsigned int timeout, unsigned int maxRetries);

void lltoggle();

#endif