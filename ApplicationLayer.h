/*
 * RCOM - Application Layer
 * Grupo XXX
 */

#ifndef __APPLICATION_LAYER__
#define __APPLICATION_LAYER__

typedef enum {
	kApplicationStateTransmitter,
	kApplicationStateReceiver
} kApplicationState;

typedef enum {
	kApplicationPacketControlData = 1,
	kApplicationPacketControlStart = 2,
	kApplicationPacketControlEnd = 3
} kApplicationPacketControl;

typedef struct ApplicationLayer {
	
	int fileDescriptor;
	
	kApplicationState state;
	
} ApplicationLayer;

typedef struct TLVParameter {
	
	int type;
	int length;
	
	char *value;
	
} TLVParameter;

ApplicationLayer *applicationLayerInstance;

void alsetup(int fd, kApplicationState state);

char * makeDataPacket(int sequenceNumber, char *data, int dataLen);
char * makeControlPacket(kApplicationPacketControl type, TLVParameter *tlvArray, int size);

#endif
