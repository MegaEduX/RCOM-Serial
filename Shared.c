/*
 * RCOM - Shared Code
 * Grupo XXX
 */

#include <stdio.h>

#include "Shared.h"

char getControlFlag(kControlFlagType type, int ackNumber) {
	switch (type) {

		case kControlFlagTypeSET:

			return 0x03;

		case kControlFlagTypeDISC:

			return 0x0B; 

		case kControlFlagTypeUA:

			return 0x07;

		case kControlFlagTypeRR:

			return ackNumber << 7 | 0x05;

		case kControlFlagTypeREJ:

			return ackNumber << 7 | 0x01;

		default:

			printf("'Eh, something isn't right mon!\n");

			return 0x00;

	}
}