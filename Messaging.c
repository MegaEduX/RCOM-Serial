/*
 * RCOM - Messaging
 * Grupo XXX
 */

#include <stdio.h>
#include <stdlib.h>

#include <unistd.h>

#include "Messaging.h"

char makeControlFlag(kControlFlagType type, unsigned int nR) {
	if (type != kControlFlagTypeRR || type != kControlFlagTypeREJ || nR > 1)
		return 0x00;
	
	if (!nR)
		return type;
	
	return (char) (type + 128);
}

int sendNonInformationalMessage(char control, int fd) {
	unsigned char SET[5];

	SET[0] = F;
	SET[1] = A;
	SET[2] = control;
	SET[3] = SET[1] ^ SET[2];
	SET[4] = F;

	return write(fd, SET, sizeof(SET));
}

int sendInformationalMessage(int messageNumber, char *data, int dataLen, char *bcc, int bccLen, int fd) {
	unsigned char *INF = malloc((6 + dataLen) * sizeof(char));

	unsigned char *baseptr = INF;

	(* baseptr) = F;
	baseptr++;

	(* baseptr) = A;
	baseptr++;

	(* baseptr) = messageNumber << 7;
	baseptr++;

	int i = 0;

	for (i = 0; i < dataLen; i++) {
		(* baseptr) = (* data);     //  I gotta check on this, though.
		
		//	printf("%.2x\n", *data);

		data++;
		baseptr++;
	}

	for (i = 0; i < bccLen; i++) {
		(* baseptr) = (* bcc);      //  This is also untested. Same problem as above.
		
		//	printf("%.2x\n", *bcc);
		
		bcc++;
		baseptr++;
	}

	(* baseptr) = F;

	return write(fd, INF, sizeof(INF));
}
