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

    /* set input mode (non-canonical, no echo,...) */
    newtio.c_lflag = 0;

    newtio.c_cc[VTIME]    = 0;   /* inter-character timer unused */
    newtio.c_cc[VMIN]     = 1;   /* blocking read until 5 chars received */

    tcflush(fd, TCIFLUSH);

    if ( tcsetattr(fd,TCSANOW,&newtio) == -1) {
      perror("tcsetattr");
      exit(-1);
    }
	
    char inputstr[256];
    
    printf("Enter string to transmit: ");
    
    gets(inputstr);

    res = write(fd,inputstr,strlen(inputstr)+1);
       
    printf("Written %d bytes.\n", res);
    
    /*if ( tcsetattr(fd,TCSANOW,&oldtio) == -1) {
      perror("tcsetattr");
      exit(-1);
    }*/
    
    printf("Receive mode set!\n");

    while (STOP==FALSE) {       /* loop for input */
      res = read(fd,buf,1);   /* returns after 5 chars have been input */
      buf[res] = 0;               /* so we can printf... */
      printf("%s", buf, res);
      if (buf[0] == '\0') 
	STOP=TRUE;
    }

    printf("\n");

    sleep(3);
    
    tcsetattr(fd,TCSANOW,&oldtio);
    close(fd);

    return 0;
}
