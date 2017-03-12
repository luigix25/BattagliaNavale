#include <stdio.h>
#include <stdlib.h>

#include <sys/types.h>
#include <sys/socket.h>

#include <arpa/inet.h>

#include <string.h>
//#include <ctype.h>

int main(int argc,char **argv){

	if(argc < 2){
		printf("[Errore] porta mancante\n");
		exit(-1);
	}

	struct sockaddr_in listenerAddress;
	struct sockaddr_in clientAddress;

	int porta = atoi(argv[1]);
	int fdmax,listener;
	unsigned int addrlen;
	int i,nbyte,status,new_sock;
	fd_set master,read_fds;
		
	/*if(porta < 0){
		pritnf("[Errore] porta errata");
		exit(-1);
	}*/


	listener = socket(AF_INET, SOCK_STREAM, 0);
	if(listener < 0){
		printf("[Errore] socket\n");
		exit(-1);
	}


	memset(&listenerAddress,0,sizeof(listenerAddress));
	FD_ZERO(&master);	
	FD_ZERO(&read_fds);

	listenerAddress.sin_port = htons(porta);
	listenerAddress.sin_family = AF_INET;
	listenerAddress.sin_addr.s_addr = INADDR_ANY;
	
	status = bind(listener, (struct sockaddr*)& listenerAddress, sizeof(listenerAddress));
	if(status < 0){
		printf("[Errore] bind\n");
		exit(-1);
	}

	status = listen(listener,10);
	if(status < 0){
		printf("[Errore] listen\n");
		exit(-1);
	}

	
	printf("[LOG] Attendo connessioni sulla porta %d\n",porta);

	
	FD_SET(listener,&master);
	fdmax = listener;
	
	while(1){
		read_fds = master;
		select(fdmax+1,&read_fds,NULL,NULL,NULL);

		for(i = 0; i <= fdmax; i++){
			if(FD_ISSET(i,&read_fds)){			
				if(i==listener){			//qualcuno si vuole connettere
					addrlen = sizeof(clientAddress);
					new_sock = accept(listener,(struct sockaddr*)&clientAddress,&addrlen);
					if(new_sock < 0){
						printf("[Errore] accept\n");
						continue;
					}					
					FD_SET(new_sock,&master);
					if(new_sock > fdmax) fdmax = new_sock;
					
					printf("Connessione stabilita con il client\n");

				} else {				//qualcuno vuole scrivere
										
				}
			}

		}	

	}


	return 0;

}
