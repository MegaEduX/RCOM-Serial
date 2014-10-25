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

int _llwrite_times_retried = 0;

volatile int _llwrite_stop = false;
volatile int _llwrite_got_data = false;

void llwrite_timeoutHandler() {
	_llwrite_stop = true;
	
	printf("Timeout!\n");
}

void llwrite_signalHandlerIO() {
	_llwrite_got_data = true;
}

typedef enum {
	kAckMessageResultSuccess,
	kAckMessageResultFailed,
	kAckMessageResultREJ
} kAckMessageResult;

int readAckMessage(int fd) {
	char buf[255];

	unsigned int rcv_error = false;

	unsigned char ACK[5];

	kStateMachine state = kStateMachineStart;

	_llwrite_stop = false;
	_llwrite_got_data = false;

	int got_data_once = false;

	signal(SIGALRM, llwrite_timeoutHandler);

	alarm(TIMEOUT);

	while (_llwrite_stop == false) {
		if (state == kStateMachineStop)
			break;	

		int res;

		while (true && !_llwrite_stop) {
			if (!got_data_once)
				sleep(2);

			if (_llwrite_got_data) {
				got_data_once = true;

				res = read(fd, buf, 1);

				break;
			}
		}

		if (_llwrite_stop) {
			rcv_error = true;

			break;
		}

		buf[res] = 0;

#if DEBUG

		printf("I got 'dis: %.2x\n", buf[0]);

#endif

		switch (state) {
			case kStateMachineStart:
			
				printf("Start: %.2x\n", buf[0]);

				if (buf[0] == F)
					ACK[state] = F;
				else {
					rcv_error = true;

					break;
				}

				state = kStateMachineFlagRcv;

				break;

			case kStateMachineFlagRcv:
			
				printf("Flag: %.2x\n", buf[0]);

				if (buf[0] == A_ANSWER)
					ACK[state] = A_ANSWER;
				else {
					rcv_error = true;

					break;
				}

				state = kStateMachineARcv;

				break;

			case kStateMachineARcv:
				
				printf("A: %.2x\n", buf[0]);
			
				linkLayerInstance->sequenceNumber = 1;
				
				if (buf[0] == makeControlFlag(kControlFlagTypeRR, linkLayerInstance->sequenceNumber) || 
					buf[0] == makeControlFlag(kControlFlagTypeREJ, linkLayerInstance->sequenceNumber))
					ACK[state] = buf[0];
				else {
					rcv_error = true;
					
					break;
				}

				state = kStateMachineCRcv;

				break;

			case kStateMachineCRcv:
				
				printf("C: %.2x\n", buf[0]);

				if (buf[0] == (ACK[1] ^ ACK[2]))
					ACK[state] = (ACK[1] ^ ACK[2]);
				else {
					rcv_error = true;

					break;
				}

				state = kStateMachineBccOkay;

				break;

			case kStateMachineBccOkay:
				
				printf("BCC: %.2x\n", buf[0]);

				if (buf[0] == F)
					ACK[state] = F;
				else {
					rcv_error = true;

					break;
				}

				state = kStateMachineStop;

				break;

			default:

				printf("We got more data than expected!\n");

				rcv_error = true;

				break;
		}

		if (buf[0] == '\0')
			_llwrite_stop = true;

		if (rcv_error) {
			printf("Error in state %d (received %s).\n", state, buf);

			_llwrite_stop = true;
		}
	}

	alarm(0);

	_llwrite_stop = true;
	
	if (!rcv_error)
		lltoggle();

#if DEBUG

	if (!rcv_error)
		printf("[Successful]");
	else
		printf("[Error]");

	printf(" I got 'dis, mon! [%.2x] [%.2x] [%.2x] [%.2x] [%.2x]\n", ACK[0], ACK[1], ACK[2], ACK[3], ACK[4]);

#endif

	return rcv_error;
}

char llwrite_calculateBcc(char *array, int length) {
	int i = 1;

	char currentBcc = array[0];

	for (; i < length; i++)
		currentBcc ^= array[i];

	return currentBcc;
}

int llwrite(int fd, char *buffer, int length) {
	char bcc = llwrite_calculateBcc(buffer, length);

	int buflen = 0, bcclen = 0, bytesSent = 0, maxRetries = TIMEOUT, tries = 0;

	char *stuffedBuffer = performStuffing(buffer, length, &buflen);

	char *stuffedBcc = performStuffing(&bcc, 1, &bcclen);
	
	while (tries < maxRetries) {
		printf("[llwrite] Sending I...\n");
		
		if ((bytesSent = sendInformationalMessage(linkLayerInstance->sequenceNumber, stuffedBuffer, buflen, stuffedBcc, bcclen, fd))) {
			printf("[llwrite] Sent I. Waiting for ACK...\n");
			
			lltoggle();		//	Toggle it.
			
			struct sigaction saio;
			
			saio.sa_handler = llwrite_signalHandlerIO;
			saio.sa_flags = 0;
			
			sigaction(SIGIO, &saio, NULL);
			
			if (!readAckMessage(fd))
				return bytesSent;
			
			lltoggle();		//	Erm, toggle-toggle?
			
			printf("[llwrite] ACK failed. Retrying...");
		}
		
		tries++;
	}
	
	return -1;
}
