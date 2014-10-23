all:
	gcc ApplicationLayer.c LinkLayer.c Shared.c llopen.c llwrite.c llclose.c Messaging.c main.c -o rcom

clean:
	rm -rf *o rcom
