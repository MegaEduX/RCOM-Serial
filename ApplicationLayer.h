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

typedef struct ApplicationLayer {
	int fileDescriptor;
	
	kApplicationState state;
} ApplicationLayer;

ApplicationLayer *applicationLayerInstance;

void alsetup(int fd, kApplicationState state);

#endif