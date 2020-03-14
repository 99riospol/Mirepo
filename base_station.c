#include <stdio.h>
#include <time.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <netinet/in.h>

#define MAXPENDING 5 //Maximum number of simultaneos connections
#define BUFFSIZE 255 //Size of message to be received

void err_sys(char *mess) { perror(mess); exit(1);}
void delay(int  numsec){
        int milli_seconds=1000*numsec;
        clock_t start_time=clock();
        while(clock()<start_time+milli_seconds);
}

void handle_client(int sock, FILE *fptr){
	char buffer[BUFFSIZE];
	int received=-1;
	//CREATE DATE AND TIME
	int hours, minutes, seconds, day, month, year;
        time_t now;
        time(&now);
        struct tm *local=localtime(&now);
        hours=local->tm_hour;
        minutes=local->tm_min;
        seconds=local->tm_sec;
        day=local->tm_mday;
        month=local->tm_mon+1;
        year=local->tm_year+1900;
	//GETTING INFO AND WRITING TO FILE
	int number; int wordlen;
	while(1){
		read(sock,&number,sizeof(number)); //LLEGIM NUM
		if(number==0) break;
		//Reading length word
		read(sock,&wordlen,sizeof(wordlen)); //LLEGIM LLARGADA PARAULA
		for(int i=1;i<=number;i++){
			read(sock,&buffer[0],wordlen); //LLEGIM PARAULA
			if(hours<12) fprintf(fptr,"[%02d/%02d/%d %02d:%02d:%02d] LINE %d: %s \n", day, month, year, hours, minutes, seconds, i, buffer);
                        else fprintf(fptr,"[%02d/%02d/%d %02d:%02d:%02d] LINE %d: %s \n", day, month, year, hours-12, minutes, seconds, i, buffer);
		}
	}
}

int main(int argc, char *argv[]){
	struct sockaddr_in echoserver, echoclient;
	int serversock, clientsock;
	int result;

	//CHECK INPUT ARGUMENTS
	if(argc!=2){
		fprintf(stderr, "Usage: %s <logfilename>\n", argv[0]);
		exit(1);
	}
	//CREATE FILE WITH THE LOGFILENAME
	FILE *fptr=NULL;
	strcat(argv[1],".txt");
	fptr=fopen(argv[1],"w");
	//CREATE TCP SOCKET
	serversock= socket(PF_INET, SOCK_STREAM, IPPROTO_TCP);
	if(serversock<0){
		err_sys("Error socket");
	}

	//SET INFO FOR sockaddr_in STRUCTURE
	memset(&echoserver, 0, sizeof(echoserver)); //reset memory
	echoserver.sin_family= AF_INET; //internet/ip
	echoserver.sin_addr.s_addr=htonl(INADDR_ANY); //any addres
	echoserver.sin_port=htons(8080);

	//BIND SOCKET
	result=bind(serversock, (struct sockaddr *) &echoserver, sizeof(echoserver));
	if(result<0){
		err_sys("Error bind");
	}

	//LISTEN SOCKET
	result=listen(serversock, MAXPENDING);
	if(result<0){
		err_sys("Error listen");
	}

	//LOOP
	while(1){
		unsigned int clientlen=sizeof(echoclient);

		//Wait for a connection from a client
		clientsock= accept(serversock, (struct sockaddr *) &echoclient, &clientlen);
		if(clientsock<0){
			err_sys("Error accept");
		}
		fprintf(stdout, "Client: %s\n", inet_ntoa(echoclient.sin_addr));
		 /* Call function to handle socket */
        	handle_client(clientsock,fptr);
		close(clientsock);
		exit(0);
	}
}

