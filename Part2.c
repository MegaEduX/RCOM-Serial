/*Non-Canonical Input Processing*/

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <termios.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <unistd.h>

#define BAUDRATE B38400
#define MODEMDEVICE "/dev/ttyS4"
#define _POSIX_SOURCE 1 /* POSIX compliant source */
#define FALSE 0
#define TRUE 1

#define F 0x7E
#define A 0x03
#define C 0x03
#define C_UA 0x07

#define TIMEOUT 3

volatile int STOP = FALSE;
volatile int READ_TIMEOUT = FALSE;

typedef enum {
    kStateMachineStart,
    kStateMachineFlagRcv,
    kStateMachineARcv,
    kStateMachineCRcv,
    kStateMachineBccOkay,
    kStateMachineStop
} kStateMachine;

kStateMachine state = kStateMachineStart;

void timeoutReached() {
    STOP = TRUE;
    READ_TIMEOUT = TRUE;
}

/*int sendAsStopAndWait(char *message) {
    int res;
    char[1024] buf;
    
    write(fd, message, sizeof(message);

    signal(SIGALRM, timeoutReached);

    alarm(TIMEOUT);
    
    while (STOP == FALSE) {
        res = read(fd,buf,1);
        buf[res] = 0;
        printf("%s", buf, res);
        
        if (buf[0] == '\0') 
	    STOP = TRUE;
    }
}*/

int sendSetupMessage(int fd) {
    unsigned char SET[5];
    
    SET[0] = F;
    SET[1] = A;
    SET[2] = C;
    SET[3] = SET[1] ^ SET[2];
    SET[4] = F;
    
    //	Do it!
    
    return write(fd, SET, sizeof(SET));
}

int readUaMessage(int fd) {
    char buf[255];
    
    unsigned int rcv_error = FALSE;
    
    unsigned char UA[5];
    
    signal(SIGALRM, timeoutReached);
    
    alarm(TIMEOUT);
    
    while (STOP == FALSE) {
        int res = read(fd, buf, 1);

	if (READ_TIMEOUT)
	    return 1;
        
        buf[res] = 0;
        
        switch (state) {
	    case kStateMachineStart:
		if (buf[0] == F)
		    UA[0] = F;
		else {
		    rcv_error = TRUE;
		    
		    break;
		}

		state = kStateMachineFlagRcv;
		
		break;
	   
	    case kStateMachineFlagRcv:
		if (buf[0] == F)
		    UA[0] = F;
		else {
		    rcv_error = TRUE;
		    
		    break;
		}

		state = kStateMachineARcv;
		
		break;
	   
	    case kStateMachineARcv:
		if (buf[0] == F)
		    UA[0] = F;
		else {
		    rcv_error = TRUE;
		    
		    break;
		}

		state = kStateMachineCRcv;
		
		break;
	   
	    case kStateMachineCRcv:
		if (buf[0] == F)
		    UA[0] = F;
		else {
		    rcv_error = TRUE;
		    
		    break;
		}

		state = kStateMachineBccOkay;
		
		break;
	   
	    case kStateMachineBccOkay:
		if (buf[0] == F)
		    UA[0] = F;
		else {
		    rcv_error = TRUE;
		    
		    break;
		}

		state = kStateMachineStop;
		
		break;
	}
      
        if (buf[0] == '\0') 
	    STOP = TRUE;
	
	if (rcv_error) {
	    printf("Error in state %d (received %s).\n", state, buf);
	    
	    STOP = TRUE;
	}
    }

    return rcv_error;
}

void doEverything(int fd) {
    int res = sendSetupMessage(fd);
       
    printf("Written %d bytes.\n", res);
    
    printf("Reading message now...\n");
    
    if (!readUaMessage(fd))
	printf("Connection established!\n");
    else {
	doEverything(fd);
	
        printf("Connection failed - retrying...\n");
    }
}

int main(int argc, char** argv) {
    struct termios oldtio,newtio;

    printf("Waiting for connection... ");

    int fd = open(MODEMDEVICE, O_RDWR | O_NOCTTY );

    printf("Connected!\n");

    if (fd < 0) {
	perror(MODEMDEVICE);
	
	exit(-1);
    }

    if (tcgetattr(fd,&oldtio) == -1) { /* save current port settings */
        perror("tcgetattr");
	
        exit(-1);
    }

    bzero(&newtio, sizeof(newtio));
    newtio.c_cflag = BAUDRATE | CS8 | CLOCAL | CREAD;
    newtio.c_iflag = IGNPAR;
    newtio.c_oflag = 0;
    
    newtio.c_lflag = 0;

    newtio.c_cc[VTIME]    = TIMEOUT * 10;   /* inter-character timer unused */
    newtio.c_cc[VMIN]     = 1;   /* blocking read until 5 chars received */

    tcflush(fd, TCIFLUSH);

    if (tcsetattr(fd,TCSANOW,&newtio) == -1) {
        perror("tcsetattr");
        exit(-1);
    }
    
    doEverything(fd);
    
    tcsetattr(fd, TCSANOW, &oldtio);
    close(fd);

    return 0;
}

