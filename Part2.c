/*
 * RCOM - Sending Part
 * Eduardo Almeida and Scrublord Santiago
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

volatile int _stop = false;
volatile int _read_timeout = false;
volatile int _got_data = false;

int _llopen_times_retried = 0;

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

kStateMachine state = kStateMachineStart;

void timeoutReached() {
    if (_stop == false) {
        printf("Timeout reached!\n");
        
        _stop = true;
        _read_timeout = true;
    }
}

int setupNonInformationalMessage(char control, int fd) {
    unsigned char SET[5];
    
    SET[0] = F;
    SET[1] = A;
    SET[2] = control;
    SET[3] = SET[1] ^ SET[2];
    SET[4] = F;
    
    return write(fd, SET, sizeof(SET));
}

int sendInformationalMessage(int messageNumber, char *data, int dataLen, int fd) {
    unsigned char *INF = malloc((6 + dataLen) * sizeof(char));
    
    unsigned char *baseptr = INF;
    
    (* baseptr) = F;
    baseptr++;
    
    (* baseptr) = A;
    baseptr++;
    
    (* baseptr) = (messageNumber % 16) << 4;
    baseptr++;
    
    int i = 0;
    
    for (i = 0; i < dataLen; i++) {
        (* baseptr) = (* data);     //  I gotta check on this, though.
        
        data++;
        baseptr++;
    }
    
    (* baseptr) = 0x00;             //    'Dis ain't right, mon! (It really isn't!)
    baseptr++;
    
    (* baseptr) = F;
    
    return write(fd, INF, sizeof(INF));
}

char getControlFlag(kControlFlagType type, int ackNumber) {
    switch (type) {
            
        case kControlFlagTypeSET:
            
            return 0x03;
            
        case kControlFlagTypeDISC:
            
            return 0x0B;
            
        case kControlFlagTypeUA:
            
            return 0x07;
            
        case kControlFlagTypeRR:
            
            return (ackNumber % 32) << 5 | 0x05;
            
        case kControlFlagTypeREJ:
            
            return (ackNumber % 32) << 5 | 0x01;
            
        default:
            
            printf("'Eh, something isn't right mon!\n");
            
            return 0x00;
            
    }
}

int readUaMessage(int fd) {
    //    printf("I reached the beginning of readUaMessage, mon!\n");
    
    char buf[255];
    
    unsigned int rcv_error = false;
    
    unsigned char UA[5];
    
    state = kStateMachineStart;
    
    signal(SIGALRM, timeoutReached);
    
    alarm(TIMEOUT);
    
    _stop = false;
    
    while (_stop == false) {
        if (state == kStateMachineStop)
            break;
        
        sleep(TIMEOUT * 10);
        
        if (_read_timeout || !_got_data)
            return 1;
        
        int res = read(fd, buf, 1);
        
        _got_data = false;
        
        if (_read_timeout)
            return 1;
        
        buf[res] = 0;
        
        printf("I got 'dis: %.2x\n", buf[0]);
        
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
    
    _stop = true;
    
#if DEBUG
    
    if (!rcv_error)
        printf("[Successful]");
    else
        printf("[Error]");
    
    printf(" I got 'dis, mon! [%.2x] [%.2x] [%.2x] [%.2x] [%.2x]", UA[0], UA[1], UA[2], UA[3], UA[4]);
    
#endif
    
    return rcv_error;
}

int readAckMessage(int fd) {
    //    The darker the night, the brighter the stars!
}

void llopen(int fd) {
    if (_llopen_times_retried > 2) {
        printf("[llopen] Couldn't establish a successful connection in %d tries, giving up...", _llopen_times_retried + 1);
        
        return;
    }
    
    int res = setupNonInformationalMessage(getControlFlag(kControlFlagTypeSET, 0), fd);
    
    printf("[llopen] Establishing connection - written %d bytes.\n", res);
    
    //    Snorlax used rest!
    //    It's fast asleep.
    
    sleep(3);
    
    //    Snorlax woke up!
    //    Snorlax used snore!
    
    printf("[llopen] Reading UA...\n");
    
    if (!readUaMessage(fd))
        printf("[llopen] Connection established!\n");
    else {
        printf("[llopen] Connection failed - retrying...\n");
        
        //    Snorlax is fast asleep.
        
        _llopen_times_retried++;
        
        llopen(fd);
    }
}

void signalHandlerIO(int status) {
    _got_data = true;
}

int main(int argc, char **argv) {
    struct termios oldtio, newtio;
    
    struct sigaction saio;
    
    printf("[serialmain] Waiting for connection...\n");
    
    int fd = open(MODEMDEVICE, O_RDWR | O_NOCTTY );
    
    printf("[serialmain] Connected!\n");
    
    if (fd < 0) {
        perror(MODEMDEVICE);
        
        exit(-1);
    }
    
    if (tcgetattr(fd,&oldtio) == -1) {
        perror("tcgetattr");
        
        exit(-1);
    }
    
    //  Setup the signal (as per slide 36)...
    
    saio.sa_handler = signalHandlerIO;
    saio.sa_flags = 0;
    saio.sa_restorer = NULL;
    
    sigaction(SIGIO, &saio, NULL);
    
    fcntl(fd, F_SETOWN, getpid());
    fcntl(fd, F_SETFL, FASYNC);
    
    //  Setup the terminal...
    
    bzero(&newtio, sizeof(newtio));
    
    newtio.c_cflag = BAUDRATE | CS8 | CLOCAL | CREAD;
    newtio.c_iflag = IGNPAR;
    newtio.c_oflag = 0;
    newtio.c_lflag = 0;
    
    //    These guys (usually) know what they are doing - http://stackoverflow.com/questions/2917881/how-to-implement-a-timeout-in-read-function-call
    //  newtio.c_lflag &= ~ICANON;
    
    newtio.c_cc[VTIME]    = TIMEOUT * 10;   /* inter-character timer unused */
    newtio.c_cc[VMIN]     = 1;   /* blocking read until 5 chars received */
    
    tcflush(fd, TCIFLUSH);
    
    if (tcsetattr(fd,TCSANOW,&newtio) == -1) {
        perror("tcsetattr");
        exit(-1);
    }
    
    llopen(fd);
    
    tcsetattr(fd, TCSANOW, &oldtio);
    close(fd);
    
    return 0;
}
