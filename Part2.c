/*Non-Canonical Input Processing*/

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <termios.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define BAUDRATE B38400
#define MODEMDEVICE "/dev/ttyS4"
#define _POSIX_SOURCE 1 /* POSIX compliant source */
#define FALSE 0
#define TRUE 1

#define F 0x7E
#define A 0x03
#define C 0x03
#define K 0x00

volatile int STOP = FALSE;

int main(int argc, char** argv) {
    int fd, c, res;
    struct termios oldtio,newtio;
    char buf[255];
    int i, sum = 0, speed = 0;

    printf("Waiting for connection... ");

    fd = open(MODEMDEVICE, O_RDWR | O_NOCTTY );

    printf("Connected!\n");

    if (fd <0) {perror(MODEMDEVICE); exit(-1); }

    if ( tcgetattr(fd,&oldtio) == -1) { /* save current port settings */
      perror("tcgetattr");
      exit(-1);
    }

    bzero(&newtio, sizeof(newtio));
    newtio.c_cflag = BAUDRATE | CS8 | CLOCAL | CREAD;
    newtio.c_iflag = IGNPAR;
    newtio.c_oflag = 0;
    
    newtio.c_lflag = 0;

    newtio.c_cc[VTIME]    = 0;   /* inter-character timer unused */
    newtio.c_cc[VMIN]     = 1;   /* blocking read until 5 chars received */

    tcflush(fd, TCIFLUSH);

    if ( tcsetattr(fd,TCSANOW,&newtio) == -1) {
      perror("tcsetattr");
      exit(-1);
    }
    
    unsigned char SET[5];
    
    SET[0] = F;
    SET[1] = A;
    SET[2] = C;
    SET[3] = SET[1] ^ SET[2];
    SET[4] = F;

    res = write(fd,SET,sizeof(SET));
       
    printf("Written %d bytes.\n", res);
    
    tcsetattr(fd,TCSANOW,&oldtio);
    close(fd);

    return 0;
}

