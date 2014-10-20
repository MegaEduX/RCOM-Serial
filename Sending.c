/*
 * RCOM - Sending Part
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

#include "Defines.h"

void timeoutHandler() {
    _stop = true;
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

int sendInformationalMessage(int messageNumber, char *data, int dataLen, char *bcc, int bccLen, int fd) {
    unsigned char *INF = malloc((6 + dataLen) * sizeof(char));
    
    unsigned char *baseptr = INF;
    
    (* baseptr) = F;
    baseptr++;
    
    (* baseptr) = A;
    baseptr++;
    
    (* baseptr) = messageNumber << 7;
    baseptr++;
    
    int i = 0;
    
    for (i = 0; i < dataLen; i++) {
        (* baseptr) = (* data);     //  I gotta check on this, though.
        
        data++;
        baseptr++;
    }
    
    for (i = 0; i < bccLen, i++) {
        (* baseptr) = (* bcc);      //  This is also untested. Same problem as above.
        
        bcc++;
        baseptr++;
    }
    
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
            
            return ackNumber << 7 | 0x05;
            
        case kControlFlagTypeREJ:
            
            return ackNumber << 7 | 0x01;
            
        default:
            
            printf("'Eh, something isn't right mon!\n");
            
            return 0x00;
            
    }
}

char llwrite_calculateBcc(char *array, int length) {
    int i = 1;
    
    char currentBcc = array[0];
    
    for (; i < length; i++)
        currentBcc ^= array[i];
    
    return currentBcc;
}

char * llwrite_performStuffing(char *buffer, int length) {
    //  To be implemented.
    
    char *stuffed = malloc(length * 2 * sizeof(char));  //  Worse case scenario!
    
    //  stuffed[i] or j or some crap then increment then whatever
    
    return buffer;
}

int llwrite(int fd, char *buffer, int length) {
    char bcc = llwrite_calculateBcc(buffer, length);
    
    char *stuffedBuffer = llwrite_performStuffing(buffer, length);
    char *stuffedBcc = llwrite_performStuffing(&bcc, 1);
    
    
}

int readUaMessage(int fd) {
    char buf[255];
    
    unsigned int rcv_error = false;
    
    unsigned char UA[5];
    
    kStateMachine state = kStateMachineStart;
    
    _stop = false;
    
    int got_data_once = false;
    
    signal(SIGALRM, timeoutHandler);
    
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
    
    int fd = open(MODEMDEVICE, O_RDWR | O_NOCTTY);
    
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
    
    newtio.c_cc[VTIME]    = 0;   /* Do Not Block! */
    newtio.c_cc[VMIN]     = 1;   /* Minimum Characters to Read */
    
    tcflush(fd, TCIFLUSH);
    
    if (tcsetattr(fd,TCSANOW,&newtio) == -1) {
        perror("tcsetattr");
        exit(-1);
    }
    
    llopen(fd);

    //	Setting old terminal...
    
    tcsetattr(fd, TCSANOW, &oldtio);
    close(fd);
    
    return 0;
}
