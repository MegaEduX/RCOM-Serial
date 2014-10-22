/*
 * RCOM - Global Defines
 * Grupo XXX
 */
 
#ifndef __DEFINES__
#define __DEFINES__

#define BAUDRATE B38400
#define MODEMDEVICE "/dev/ttyS4"
#define _POSIX_SOURCE 1 /* POSIX compliant source */

#define false 0
#define true 1

#define DEBUG 0

#define F 0x7E
#define A 0x03
#define C 0x03
#define C_UA 0x07

#define TIMEOUT 3

/*  volatile int _stop = false;
volatile int _read_timeout = false;
volatile int _got_data = false;

int _llopen_times_retried = 0;  */

typedef enum {
    kStateMachineStart,
    kStateMachineFlagRcv,
    kStateMachineARcv,
    kStateMachineCRcv,
    kStateMachineBccOkay,
    kStateMachineStop
} kStateMachine;

typedef enum {
    kControlFlagTypeSET,
    kControlFlagTypeDISC,
    kControlFlagTypeUA,
    kControlFlagTypeRR,
    kControlFlagTypeREJ
} kControlFlagType;

#endif
