#include <stdio.h>
#include <stdlib.h>

#include <sys/types.h>
#include <sys/socket.h>

#include <arpa/inet.h>

#include <string.h>


int main(int argc,char **argv){

	if(argc < 3){
		printf("[Errore] ip e porta necessari\n");
		exit(-1);
	}

	int sock,status,nbyte,porta;
	struct sockaddr_in serverAddress;

	porta = atoi(argv[2]);

	sock = socket(AF_INET,SOCK_STREAM,0);
	if(sock < 0){
		printf("[Errore] socket\n");
		return -1;
	}

	memset(&serverAddress,0,sizeof(serverAddress));
	
	serverAddress.sin_family = AF_INET;
	serverAddress.sin_port = htons(porta);
	inet_pton(AF_INET,argv[2],&serverAddress.sin_addr);
	

	status = connect(sock, (struct sockaddr*)&serverAddress,sizeof(serverAddress));
	if(status < 0){
		printf("[Errore] connect\n");
		return -1;
	}
	
	printf("Connessione al server %s (porta %d) effettuata con successo\n",argv[2],porta);
	return 0;

}
