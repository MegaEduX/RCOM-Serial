/*
 * RCOM - Link Layer
 * Grupo XXX
 */

typedef struct LinkLayer {
	char port[20];
	int baudRate;

	unsigned int sequenceNumber;
	unsigned int timeout;
	unsigned int maxRetries;
} LinkLayer;

LinkLayer *linkLayerInstance;

void llsetup(char *port, int baudRate, unsigned int sequenceNumber, unsigned int timeout, unsigned int maxRetries);
