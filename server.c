#include "library/library.h"

void addUser(const char *username,unsigned int port){
	printf("Si e' aggiunto %s sulla porta %d\n",username,port);
}

void login_function(int sock){

	char *username;
	int port;

	username = recvString(sock);
	if(username == NULL){
		printf("NULL\n");
		//gestione errore
	}
	
	if(!recvInt(sock,&port)){
		printf("Errore\n");
		exit(1);
	}

	addUser(username,port);

}

void select_command(int cmd,int sock){

	switch (cmd){
		case LOGIN_COMMAND:
			login_function(sock);
			break;
	
	}


}

int main(int argc,char **argv){

	if(argc != 2){
		printf("[Errore] parametri errati\n");
		exit(-1);
	}

	struct sockaddr_in listenerAddress;
	struct sockaddr_in clientAddress;

	int port = atoi(argv[1]);
	int fdmax,listener;
	unsigned int addrlen;
	int i,status,new_sock;
	int cmd;
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

	listenerAddress.sin_port = htons(port);
	listenerAddress.sin_family = AF_INET;
	listenerAddress.sin_addr.s_addr = INADDR_ANY;
	
	status = bind(listener, (struct sockaddr*)& listenerAddress, sizeof(listenerAddress));
	if(status < 0){
		perror("[Errore] bind\n");
		exit(-1);
	}

	status = listen(listener,10);
	if(status < 0){
		perror("[Errore] listen\n");
		exit(-1);
	}

	
	printf("[LOG] Attendo connessioni sulla porta %d\n",port);

	
	FD_SET(listener,&master);
	fdmax = listener;
	
	while(true){
		read_fds = master;
		if(select(fdmax+1,&read_fds,NULL,NULL,NULL) <=0){
			perror("[Errore] Select");
			exit(1);
		}

		for(i = 0; i <= fdmax; i++){
			if(FD_ISSET(i,&read_fds)){			
				if(i==listener){			//qualcuno si vuole connettere
					memset(&clientAddress,0,sizeof(clientAddress));
					addrlen = sizeof(clientAddress);
					new_sock = accept(listener,(struct sockaddr*)&clientAddress,&addrlen);
					if(new_sock < 0){
						perror("[Errore] accept\n");
						continue;
					}					
					FD_SET(new_sock,&master);
					if(new_sock > fdmax) fdmax = new_sock;
					
					printf("Connessione stabilita con il client\n");

				} else {				//qualcuno vuole scrivere
					status = recvInt(i,&cmd);
					if(status == false){
						//disconnesso;					
					} else {
						select_command(cmd,i);
					}
				}
			}

		}	

	}


	return 0;

}
