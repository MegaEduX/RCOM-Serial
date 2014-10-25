/*
 * RCOM - Messaging
 * Grupo XXX
 */

#ifndef __MESSAGING__
#define __MESSAGING__

#include "Defines.h"

char makeControlFlag(kControlFlagType type, unsigned int nR);

int sendNonInformationalMessage(char control, int fd);

int sendInformationalMessage(int messageNumber, char *data, int dataLen, char *bcc, int bccLen, int fd);

#endif
