/*
 * RCOM - Application Layer
 * Grupo XXX
 */

#include <stdlib.h>

#include "ApplicationLayer.h"

void alsetup(int fd, kApplicationState state) {
	applicationLayerInstance = (ApplicationLayer *) malloc(sizeof(ApplicationLayer));
	
	applicationLayerInstance->fileDescriptor = fd;
	applicationLayerInstance->state = state;
}