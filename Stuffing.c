/*
 * RCOM - Stuffing
 * Grupo XXX
 */

#include <stdlib.h>

#include "Stuffing.h"

char * performStuffing(char *buffer, int length, int *destLength) {
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

char * performDestuffing(char *buffer, int length, int *destLength) {
	char *destuffed = malloc(length * sizeof(char));	//	Worse case scenario!
	
	int i = 0, j = 0;
	
	for (; i < length; i++, j++) {
		
		if (buffer[i] == 0x7d) {
			
			if (buffer[i + 1] == 0x5e) {
				
				destuffed[j] = 0x7e;
				
				i++;
				
			} else if (buffer[i + 1] == 0x5d) {
				
				destuffed[j] = 0x7d;
				
				i++;
				
			} else
				destuffed[j] = 0x7d;
				
		} else
			destuffed[j] = buffer[i];
		
	}
	
	(* destLength) = j;
	
	return destuffed;
}
