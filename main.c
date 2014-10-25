/*
 *  RCOM - Main
 *  Grupo XXX
 */

#include <sys/types.h>
#include <sys/stat.h>

#include <fcntl.h>

#include <libgen.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <termios.h>

#include <unistd.h>
#include <strings.h>

#include "Defines.h"
#include "ApplicationLayer.h"

#include "llopen.h"
#include "llwrite.h"
#include "llclose.h"

int main(int argc, char **argv) {
    if (argc < 2) {
        char progname[512];
        strcpy(progname, argv[0]);
        
        printf("Usage: %s file_to_send [serial_port = /dev/ttyS4 [baud_rate = B38400 [max_tries = 3 [timeout = 3]]]]\n", basename(progname));
        
        return 0;
    }
    
    char *path = malloc(256 * sizeof(char));
    
    strcpy(path, argv[1]);
    
    printf("Path: %s\n", path);
    
    char *serialPort = NULL;
    
    long baudRate = B38400;
    
    int maxRetries = 3, timeout = 3;
    
    if (argc > 2)
        serialPort = argv[2];
    
    if (argc > 3)
        baudRate = atol(argv[3]);
    
    if (argc > 4)
        maxRetries = atoi(argv[4]);
    
    llsetup(serialPort, baudRate, 0, timeout, maxRetries);
    
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
    
    newtio.c_cflag = baudRate | CS8 | CLOCAL | CREAD;
    newtio.c_iflag = IGNPAR;
    newtio.c_oflag = 0;
    newtio.c_lflag = 0;
    
    newtio.c_cc[VTIME]    = 5;   /* Do Not Block! */
    newtio.c_cc[VMIN]     = 0;   /* Minimum Characters to Read */
    
    tcflush(fd, TCIFLUSH);
    
    if (tcsetattr(fd, TCSANOW, &newtio) == -1) {
        perror("tcsetattr");
        return -1;
    }
    
    llopen(fd, kApplicationStateTransmitter);
    
    FILE *file = fopen(path, "r");
    
    if (file == NULL) {
        perror("fopen");
        return -1;
    }
    
    /*
     *  Start Control Packet
     */
    
    struct stat st;
    
    stat(path, &st);
    
    int size = st.st_size;
    
    int bplen = 0;
    
    TLVParameter *tlvArray = malloc(2 * sizeof(TLVParameter));
    
    tlvArray[0].type = 0;
    tlvArray[0].length = sizeof(int);       //  Placeholder!
    
    char value = (char) 1234;
    
    tlvArray[0].value = &value;               //  Placeholder!
    
    tlvArray[1].type = 1;
    tlvArray[1].length = strlen(path);    //  Placeholder!
    tlvArray[1].value = path;             //  Placeholder!
    
    char *beginPacket = makeControlPacket(kApplicationPacketControlStart, tlvArray, 2, &bplen);
    
    llwrite(fd, beginPacket, bplen);
    
    /*
     *  Data Packets
     */
    
    int finished = false;
    
    while (!finished) {
        char ch;
        
        char *buf = malloc(255 * sizeof(char));
        
        int i = 0, seq = 0;
        
        while ((ch = fgetc(file)) != EOF) {
            if (i < 255)
                buf[i] = ch;
            else
                break;
        }
        
        int plen = 0;
        
        char *dataPacket = makeDataPacket(seq, buf, i, &plen);
        
        if (llwrite(fd, dataPacket, plen) == -1) {
            //
            //  Error, even after retrying!
            //
            //  We should re-setup and retry.
            //
            //  Failing for now...
            //
            
            printf("Error in llwrite!\n");
            
            break;
        }
        
        if (i < 255)
            finished = true;
    }
    
    /*
     *  End Control Packet
     */
    
    int eplen = 0;
    
    char *endPacket = makeControlPacket(kApplicationPacketControlEnd, NULL, 0, &eplen);
    
    llwrite(fd, endPacket, eplen);
    
    llclose(fd);
    
    //  And back to original settings...
    
    tcsetattr(fd, TCSANOW, &oldtio);
    close(fd);
    
    printf("All done! Terminating...\n");
    
    return 0;
}
