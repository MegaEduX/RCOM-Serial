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

char llwrite_calculateBcc(char *array, int length);
char * llwrite_performStuffing(char *buffer, int length, int *destLength);

char llwrite_calculateBcc(char *array, int length) {
	int i = 1;

	char currentBcc = array[0];

	for (; i < length; i++)
		currentBcc ^= array[i];

	return currentBcc;
}

char * llwrite_performStuffing(char *buffer, int length, int *destLength) {
	char *stuffed = malloc(length * 2 * sizeof(char));  //  Worse case scenario!

	int i = 0, j = 0;

	for (; i < length; i++) {

		if (buffer[i] == 0x7e) {

			stuffed[j++] = 0x7d;
			stuffed[j++] = 0x5e;

		} else if (buffer[i] == 0x7d) {

			stuffed[j++] = 0x7d;
			stuffed[j++] = 0x5d;

		} else
			stuffed[j++] = buffer[i];

	}

	(* destLength) = j;

	return stuffed;
}

int llwrite(int fd, char *buffer, int length) {
	char bcc = llwrite_calculateBcc(buffer, length);

	int buflen = 0, bcclen = 0;

	char *stuffedBuffer = llwrite_performStuffing(buffer, length, &buflen);

	char *stuffedBcc = llwrite_performStuffing(&bcc, 1, &bcclen);

	return sendInformationalMessage(linkLayerInstance->sequenceNumber, stuffedBuffer, buflen, stuffedBcc, bcclen, fd);
}