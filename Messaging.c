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
	unsigned char *INF = malloc((5 + dataLen + bccLen) * sizeof(char));
	
	INF[0] = F;
	INF[1] = A;
	INF[2] = messageNumber << 6;
	INF[3] = INF[1] ^ INF[2];
	
	printf("0: %.2x, 1: %.2x, 2: %.2x, 3: %.2x\n", INF[0], INF[1], INF[2], INF[3]);
	
	int i = 4, j = 0;

	for (j = 0; j < dataLen; i++, j++) {
		INF[i] = data[j];
		
		printf("%d: %.2x ", i, INF[i]);
	}

	for (j = 0; j < bccLen; i++, j++) {
		INF[i] = bcc[j];
		
		printf("%d: %.2x ", i, INF[i]);
	}
	
	printf("\n");
	
	INF[i] = F;
	
	printf("F: %d\n", INF[i]);
	
	//	printf("Size of INF: %d ~ Size on Write: %d\n", (int)sizeof(INF), (int)((5 + dataLen + bccLen) * sizeof(char)));
	
	return write(fd, INF, (int)sizeof(INF), (int)((5 + dataLen + bccLen) * sizeof(char)));
}
