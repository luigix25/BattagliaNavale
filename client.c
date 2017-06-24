#include "library/library.h"

fd_set master;
int socket_udp;

void cmd_help(){

	char *desc[4] = {"--> mostra l'elenco dei comandi disponibili",
			"--> mostra l'elenco dei client connessi al server",
			"username --> avvia una partita con l'utente username",
			"--> disconnette il client dal server\n"};

	int i;
	printf("\n");	
	printf("Sono disponibili i seguenti comandi:\n");
	for(i=0;i<4;i++){
		printf("%s %s\n",commands_list[i],desc[i]);
	}

}

void cmd_connect(int sock){

	char *username;
	char *ip;
	int port;

	int result;
	scanf("%ms",&username);
	if(!sendInt(sock,CONNECT_COMMAND))	return;
	if(!sendString(sock,username))		return;
	/*if(!recvInt(sock,&result))		return;

	switch(result){
		case CONNECT_NOUSER:
			printf("Username non esistente\n");
			break;
		case CONNECT_BUSY:
			printf("L'utente occupato\n");
			break;
		case CONNECT_REFUSED:
			printf("L'utente ha rifiutato la partita\n");
			break;
		case CONNECT_OK:
			printf("L'utente ha accettato la partita\n");
			
			ip = recvString(sock);	
			if(ip==NULL)				return;			//ip

			if(!recvInt(sock,&port));		return;			//porta

			printf(">%s:%d\n",ip,port);
			printf("provo a ricevere UDP\n");

			struct sockaddr_in cl;
			int ricevuto;
			//recvUDPInt(socket_udp,&cl,&ricevuto);
			//printf("ricevuto %d\n",ricevuto);	

			break;
	}
*/
}

void cmd_login(int sock,int *sock_client,int *port_client){
	int result;
	char *buffer;

	struct sockaddr_in my_addr;

	do{
	
		printf("Inserisci il tuo nome: ");
		fflush(stdout);	
		scanf("%ms",&buffer);
		printf("Inserisci la porta: ");
		fflush(stdout);
		scanf("%d",port_client);
		if(port_client < 0){
			printf("Porta non valida\n");
			continue;			
		}

		if(!sendInt(sock,LOGIN_COMMAND))	return;
		if(!sendString(sock,buffer)) 		return;
		if(!sendInt(sock,*port_client))		return;
		if(!recvInt(sock,&result)) 		return;
		if(result == LOGIN_FAIL){
			printf("Username gia' in uso\n");
		}


	} while(result != LOGIN_OK);


	//creo server UDP

	*sock_client = socket(AF_INET,SOCK_DGRAM,0);
	if(*sock_client < 0){
		perror("Errore socket");
		exit(-1);
	}	

	memset(&my_addr,0,sizeof(my_addr));
	my_addr.sin_family = AF_INET;
	my_addr.sin_port = htons(*port_client);
	my_addr.sin_addr.s_addr = INADDR_ANY;

	result = bind(*sock_client,(struct sockaddr*)&my_addr,sizeof(my_addr));
	if(result < 0){
		perror("Errore bind");
		exit(-1);
	}
	
}

void cmd_who(int sock){

	int n,i,status;
	char *username;

	if(!sendInt(sock,WHO_COMMAND))	return;

	printf("Client connessi al server:\n");

	if(!recvInt(sock,&n))		return;
	
	for(i=0;i<n;i++){
		username = recvString(sock);
		if(!username)			return;

		if(!recvInt(sock,&status))	return;
		
		if(status == FREE){
			printf("\t%s(libero)\n",username);
		} else {
			printf("\t%s(occupato)\n",username);		
		}

	}


}

void cmd_quit(int sock){

	printf("\nClient disconnesso correttamente\n");
	if(!sendInt(sock,QUIT_COMMAND))		return;
	close(sock);
	exit(1);
}

void select_command(int sock,char *buffer){

	if(strcmp("!help",buffer) == 0){
		cmd_help();
		return;
	} else if(strcmp("!who",buffer) == 0){
		cmd_who(sock);
		return;
	} else if(strcmp("!quit",buffer) == 0){
		cmd_quit(sock);
		return;	
	} else if(strcmp("!connect",buffer) == 0){
		cmd_connect(sock);
		return;	
	}

}

void read_input(int sock){

	char *buffer = 0;
	fflush(stdout);
	scanf("%ms",&buffer);
	select_command(sock,buffer);
}

void startGame(int socket,char *ip,int port,int socket_udp){

	//creo indirizzo per UDP
	struct sockaddr_in sv_addr;
	int res;

	memset(&sv_addr,0,sizeof(sv_addr));
	sv_addr.sin_family = AF_INET;
	sv_addr.sin_port = htons(port);
	inet_pton(AF_INET,ip,&sv_addr.sin_addr);

	
	
	res = sendUDPInt(socket_udp,sv_addr,555);



}

void handle_connection_request(int sock){
	
	#warning gestire_disconnessione
	char *ip;
	char *str = recvString(sock);
	char answ = 'x';
	int port;

	do{
		printf("%s si vuole connettere a te, Accetti? (y/n)",str);
		fflush(stdout);
		getchar();				//flush per ritorno a capo
		scanf("%c",&answ);
		
	} while(answ != 'y' && answ != 'n');


	if(!sendInt(sock,CONNECT_ANSWER))		return;			//gestire l'errore

	if(answ == 'y'){
		if(!sendInt(sock,CONNECT_ACPT))	return;				//gestire l'errore
	} else {
		if(!sendInt(sock,CONNECT_RFSD))	return;
	}
}

void handle_connection_accepted(int socket){

	printf("wewe si gioca\n");


}

void handle_receive_data(int socket){
	char *ip;
	int port;

	ip = recvString(socket);						//mi aspetto l'ip
	if(ip == NULL)				return;
	if(!recvInt(socket,&port))		return;				//porta altro client
	printf("%s:%d\n",ip,port);
	startGame(socket,ip,port,socket_udp);

}


void select_command_server(int socket,int cmd){

	switch(cmd){
		case CONNECT_REQ:
			handle_connection_request(socket);
			break;
		case CONNECT_ACPT:
			handle_connection_accepted(socket);
			break;
		case CONNECT_REFUSED:
			printf("L'utente ha rifiutato la tua partita\n");
			return;
			break;
		case CONNECT_NOUSER:
			printf("Utente non esistente\n");
			break;
		case CONNECT_BUSY:
			printf("L'utente e' impegnato in un'altra partita\n");
			break;
		case CONNECT_DATA:
			handle_receive_data(socket);
			break;
	}

}

int main(int argc,char **argv){

	if(argc < 3){
		printf("[Errore] ip e porta necessari\n");
		exit(-1);
	}

	int socket_server,status,portServer,port_udp,cmd;
	struct sockaddr_in serverAddress;

	int i,fdmax;
	fd_set read_fds;

	portServer = atoi(argv[2]);

	socket_server = socket(AF_INET,SOCK_STREAM,0);
	if(socket_server < 0){
		perror("[Errore] socket\n");
		return -1;
	}

	memset(&serverAddress,0,sizeof(serverAddress));
	
	serverAddress.sin_family = AF_INET;
	serverAddress.sin_port = htons(portServer);
	inet_pton(AF_INET,argv[1],&serverAddress.sin_addr);
	

	status = connect(socket_server, (struct sockaddr*)&serverAddress,sizeof(serverAddress));
	if(status < 0){
		perror("[Errore] connect\n");
		return -1;
	}
	
	printf("\nConnessione al server %s (port %d) effettuata con successo\n",argv[1],portServer);

	
	cmd_help();
	cmd_login(socket_server,&socket_udp,&port_udp);

	FD_ZERO(&master);	
	FD_ZERO(&read_fds);

	FD_SET(socket_server,&master);
	FD_SET(0,&master);		// 0 è stdin

	fdmax = socket_server;

	while(true){
		printf("\r>");
		fflush(stdout);
		
		read_fds = master;
		if(select(fdmax+1,&read_fds,NULL,NULL,NULL) <=0){
			perror("[Errore] Select");
			exit(-1);
		}

		for(i = 0; i <= fdmax; i++){
			if(FD_ISSET(i,&read_fds)){
				if(i == 0){			//stdin
					read_input(socket_server);					
					//continue;
				} else {			//socket
					if(!recvInt(i,&cmd))	return -1;
					select_command_server(i,cmd);	
				}

			}

		}
		
		//read_input(socket_server);

	}	

	
	return 0;

}
