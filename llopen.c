/*
 * RCOM - llopen
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

#include "llopen.h"

#include "Shared.h"

#include "Defines.h"

int _llopen_times_retried = 0;

volatile int _llopen_stop = false;

void llopen_timeoutHandler() {
	_llopen_stop = true;
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

int readUaMessage(int fd) {
	char buf[255];

	unsigned int rcv_error = false;

	unsigned char UA[5];

	kStateMachine state = kStateMachineStart;

	_stop = false;

	int got_data_once = false;

	signal(SIGALRM, llopen_timeoutHandler);

	alarm(TIMEOUT);

	while (_stop == false) {
		if (state == kStateMachineStop)
			break;	

		int res;

		while (true && !_stop) {
			if (!got_data_once)
				sleep(2);

			if (_got_data) {
				got_data_once = true;

		res = read(fd, buf, 1);

				break;
			}
		}

		if (_stop) {
			rcv_error = true;

			break;
		}

		buf[res] = 0;

#if DEBUG

		printf("I got 'dis: %.2x\n", buf[0]);

#endif

		switch (state) {
			case kStateMachineStart:

				if (buf[0] == F)
					UA[state] = F;
				else {
					rcv_error = true;

					break;
				}

				state = kStateMachineFlagRcv;

				break;

			case kStateMachineFlagRcv:

				if (buf[0] == A)
					UA[state] = A;
				else {
					rcv_error = true;

					break;
				}

				state = kStateMachineARcv;

				break;

			case kStateMachineARcv:

				if (buf[0] == C_UA)
					UA[state] = C_UA;
				else {
					rcv_error = true;

					break;
				}

				state = kStateMachineCRcv;

				break;

			case kStateMachineCRcv:

				if (buf[0] == (UA[1] ^ UA[2]))
					UA[state] = (UA[1] ^ UA[2]);
				else {
					rcv_error = true;

					break;
				}

				state = kStateMachineBccOkay;

				break;

			case kStateMachineBccOkay:

				if (buf[0] == F)
					UA[state] = F;
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
			_stop = true;

		if (rcv_error) {
			printf("Error in state %d (received %s).\n", state, buf);

			_stop = true;
		}
	}

	alarm(0);

	_stop = true;

#if DEBUG

	if (!rcv_error)
		printf("[Successful]");
	else
		printf("[Error]");

	printf(" I got 'dis, mon! [%.2x] [%.2x] [%.2x] [%.2x] [%.2x]\n", UA[0], UA[1], UA[2], UA[3], UA[4]);

#endif

	return rcv_error;
}

int llopen_pt2(int fd) {
	if (_llopen_times_retried > 2) {
		printf("[llopen] Couldn't establish a successful connection in %d tries, giving up...\n", _llopen_times_retried + 1);

		return -1;
	}

	int res = sendNonInformationalMessage(getControlFlag(kControlFlagTypeSET, 0), fd);

	printf("[llopen] Establishing connection - written %d bytes.\n", res);

	//    Snorlax used rest!
	//    It's fast asleep.

	sleep(3);

	//    Snorlax woke up!
	//    Snorlax used snore!

	printf("[llopen] Reading UA...\n");

	if (!readUaMessage(fd)) {
		printf("[llopen] Connection established!\n");
		
		return 0;
	} else {
		printf("[llopen] Connection failed - retrying...\n");

		//    Snorlax is fast asleep.

		_llopen_times_retried++;

		llopen(fd);
	}
}

int llopen(int port, kApplicationState state) {
	alsetup(port, state);
	
	_llopen_times_retried = 0;
	
	_llopen_stop = false;
	
	//  Setup the signal (as per slide 36)...
	
	saio.sa_handler = signalHandlerIO;
	saio.sa_flags = 0;
	saio.sa_restorer = NULL;
	
	sigaction(SIGIO, &saio, NULL);
	
	return llopen_pt2(fd);
}
