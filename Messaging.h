/*
 * RCOM - Messaging
 * Grupo XXX
 */

#ifndef __MESSAGING__
#define __MESSAGING__

int sendNonInformationalMessage(char control, int fd);

int sendInformationalMessage(int messageNumber, char *data, int dataLen, char *bcc, int bccLen, int fd);

#endif