/*
 * RCOM - Sending Part
 * Grupo XXX
 */

#include <sys/types.h>
#include <sys/stat.h>

#include <fcntl.h>

#include <stdio.h>
#include <stdlib.h>

#include <termios.h>

#include <unistd.h>
#include <strings.h>

#include "Defines.h"

#include "llopen.h"
#include "llwrite.h"
#include "llclose.h"

int main(int argc, char **argv) {
    struct termios oldtio, newtio;
    
    printf("[Serial Setup] Waiting for connection...\n");
    
    int fd = open(MODEMDEVICE, O_RDWR | O_NOCTTY);
    
    printf("[Serial Setup] Connected!\n");
    
    if (fd < 0) {
        perror(MODEMDEVICE);
        
        return -1;
    }
    
    if (tcgetattr(fd,&oldtio) == -1) {
        perror("tcgetattr");
        
        return -1;
    }
    
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
        return -1;
    }
    
    llopen(fd, kApplicationStateTransmitter);
    
    //  llwrite();
    
    llclose(fd);
    
    //  And back to original settings...
    
    tcsetattr(fd, TCSANOW, &oldtio);
    close(fd);
    
    printf("All done! Terminating...\n");
    
    return 0;
}
