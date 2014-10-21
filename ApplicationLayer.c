/*
 * RCOM - Application Layer
 * Grupo XXX
 */

void alsetup(int fd, kApplicationState state) {
	applicationLayerInstance->fileDescriptor = fd;
	applicationLayerInstance->state = state;
}