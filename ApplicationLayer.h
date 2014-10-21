/*
 * RCOM - Application Layer
 * Grupo XXX
 */

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
