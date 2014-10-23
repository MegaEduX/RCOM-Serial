/*
 * RCOM - llclose
 * Grupo XXX
 */
#include <sys/types.h>
#include <sys/stat.h>

#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <termios.h>
#include <unistd.h>

#include "llclose.h"

#include "Shared.h"
#include "Defines.h"
#include "Messaging.h"

int _llclose_times_retried = 0;

volatile int _llclose_stop = false;
volatile int _llclose_got_data = false;

void llclose_timeoutHandler() {
	_llclose_stop = true;
}

void llclose_signalHandlerIO() {
	_llclose_got_data = true;
}

int llclose_readDiscMessage(int fd) {
	char buf[255];

	unsigned int rcv_error = false;

	unsigned char UA[5];

	kStateMachine state = kStateMachineStart;

	_llclose_stop = false;

	int got_data_once = false;

	signal(SIGALRM, llclose_timeoutHandler);

	alarm(TIMEOUT);

	while (_llclose_stop == false) {
		if (state == kStateMachineStop)
			break;	

		int res;

		while (true && !_llclose_stop) {
			if (!got_data_once)
				sleep(2);

			if (_llclose_got_data) {
				got_data_once = true;

				res = read(fd, buf, 1);

				break;
			}
		}

		if (_llclose_stop) {
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

				if (buf[0] == C_DISC)
					UA[state] = C_DISC;
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
			_llclose_stop = true;

		if (rcv_error) {
			printf("Error in state %d (received %s).\n", state, buf);

			_llclose_stop = true;
		}
	}

	alarm(0);

	_llclose_stop = true;

#if DEBUG

	if (!rcv_error)
		printf("[Successful]");
	else
		printf("[Error]");

	printf(" I got 'dis, mon! [%.2x] [%.2x] [%.2x] [%.2x] [%.2x]\n", UA[0], UA[1], UA[2], UA[3], UA[4]);

#endif

	return rcv_error;
}

int llclose_pt2(int fd) {
	if (_llclose_times_retried > 2) {
		printf("[llclose] Couldn't establish a successful connection in %d tries, giving up...\n", _llclose_times_retried + 1);

		return -1;
	}

	int res = sendNonInformationalMessage(getControlFlag(kControlFlagTypeDISC, 0), fd);

	if (res == -1) {
		printf("[llclose] Couldn't send disconnection message. Error: \"%s\".\n", strerror(errno));

		return -1;
	}

	printf("[llclose] Establishing connection - written %d bytes.\n", res);

	sleep(3);

	printf("[llclose] Reading DISC...\n");

	if (!llclose_readDiscMessage(fd)) {
		printf("[llclose] Connection terminated!\n");

		return 0;
	} else {
		printf("[llclose] Connection termination failed - retrying...\n");

		//    Snorlax is fast asleep.

		_llclose_times_retried++;

		llclose_pt2(fd);
	}

	return -1;
}

int llclose(int fd) {
	_llclose_times_retried = 0;
	
	_llclose_stop = false;
	_llclose_got_data = false;
	
	//  Setup the signal (as per slide 36)...
	
	struct sigaction saio;
	
	saio.sa_handler = llclose_signalHandlerIO;
	saio.sa_flags = 0;
	
	sigaction(SIGIO, &saio, NULL);
	
	return llclose_pt2(fd);
}
