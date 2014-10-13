/*Non-Canonical Input Processing*/

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <termios.h>
#include <stdio.h>

#define BAUDRATE B38400
#define _POSIX_SOURCE 1 /* POSIX compliant source */
#define FALSE 0
#define TRUE 1

#define F 0x7E
#define A 0x03
#define C_SET 0x03
#define C_UA 0x07

volatile int STOP=FALSE;

int main(int argc, char** argv)
{
    int fd, res;
    struct termios oldtio,newtio;
    char buf[255];
	int set_equal = TRUE;

	unsigned char SET[5];
	SET[0] = F;
	SET[1] = A;
	SET[2] = C_SET;
	SET[3] = SET[1]^SET[2];
	SET[4] = F;

    unsigned char UA[5];
	UA[0] = F;
	UA[1] = A;
	UA[2] = C_UA;
	UA[3] = UA[1]^UA[2];
	UA[4] = F;

    /*if ( (argc < 2) || 
  	     ((strcmp("/dev/ttyS0", argv[1])!=0) && 
  	      (strcmp("/dev/ttyS1", argv[1])!=0) )) {
      printf("Usage:\tnserial SerialPort\n\tex: nserial /dev/ttyS1\n");
      exit(1);
    }*/


  /*
    Open serial port device for reading and writing and not as controlling tty
    because we don't want to get killed if linenoise sends CTRL-C.
  */
  
    
    fd = open("/dev/ttyS0", O_RDWR | O_NOCTTY );
    if (fd <0) {perror(argv[1]); exit(-1); }

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



  /* 
    VTIME e VMIN devem ser alterados de forma a proteger com um temporizador a 
    leitura do(s) pr�ximo(s) caracter(es)
  */



    tcflush(fd, TCIOFLUSH);

    if ( tcsetattr(fd,TCSANOW,&newtio) == -1) {
      perror("tcsetattr");
      exit(-1);
    }

    printf("New termios structure settttt\n");
 	char *str = (char *)malloc(256*sizeof(char));

	int i = 0;
	unsigned char temp[5];
    while (TRUE) {       /* loop for input */
      res = read(fd,buf,1);   /* returns after 5 chars have bee*/
      buf[res]=0; 
	  printf("r: 0x");              /* so we can printf... */
     
	  if (i > 4)
		break;
  
	printf("%.2x ", buf[0]);

	  if (buf[0] == SET[i]) {
		printf("eigual\n");
		i++;
	  }
	  else {
		set_equal = FALSE;
		break;
		}
	}
printf("22222\n");
	//printf("%s", str);
    if (set_equal == TRUE) {
   		//int r = write(fd,UA,5);   
    	printf("\n%d bytes written - %s\n", r, str);
	}


  /* 
    O ciclo WHILE deve ser alterado de modo a respeitar o indicado no gui�o 
  */

printf("alalal\n");

sleep(3);
    tcsetattr(fd,TCSANOW,&oldtio);
    close(fd);
    return 0;
}

