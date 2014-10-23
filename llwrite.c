/*
 * RCOM - llwrite
 * Grupo XXX
 */

#include <sys/types.h>
#include <sys/stat.h>

#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <termios.h>
#include <unistd.h>
 
#include "llwrite.h"
#include "Defines.h"
#include "Messaging.h"
#include "Stuffing.h"

char llwrite_calculateBcc(char *array, int length) {
	int i = 1;

	char currentBcc = array[0];

	for (; i < length; i++)
		currentBcc ^= array[i];

	return currentBcc;
}

int llwrite(int fd, char *buffer, int length) {
	char bcc = llwrite_calculateBcc(buffer, length);

	int buflen = 0, bcclen = 0;

	char *stuffedBuffer = performStuffing(buffer, length, &buflen);

	char *stuffedBcc = performStuffing(&bcc, 1, &bcclen);

	return sendInformationalMessage(linkLayerInstance->sequenceNumber, stuffedBuffer, buflen, stuffedBcc, bcclen, fd);
}