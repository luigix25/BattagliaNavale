#include "library/library.h"

typedef struct user{
	char *username;
	char ip[INET_ADDRSTRLEN];
	int port;
	struct user *next;
	unsigned int status;
	int sock;
	
	struct user *pending_request;

} user;


user *user_list; 
int n_users;

fd_set master;

user* searchUserByName(char *username){
	user *tmp = user_list;	
	while(tmp != NULL){
		if(strcmp(tmp->username,username) == 0){
			return tmp;
		}	
	
		tmp = tmp->next;
	}

	return NULL;
}

user* searchUserBySocket(int sock){
	user *tmp = user_list;	

	while(tmp != NULL){
		if(tmp->sock == sock){
			return tmp;
		}	
	
		tmp = tmp->next;
	}

	return NULL;

}

int existingUsername(const char *username){

	user *root = user_list;
	while(root != NULL){
		if(strcmp(root->username,username)==0){
			return true;
		}
		root = root->next;
	}
	return false;
}

void addUser(char *username,unsigned int port,int sock,struct sockaddr_in socket_full){
	user *new_user = malloc(sizeof(user));		
	new_user->username = username;
	new_user->port = port;	
	new_user->next = NULL;
	new_user->status = FREE;
	new_user->sock = sock;

	inet_ntop(AF_INET,&socket_full.sin_addr,new_user->ip,INET_ADDRSTRLEN);		//converto l'ip in stringa

	n_users++;

	if(user_list == NULL){
		user_list = new_user;
		return;	
	}	

	user *tmp = user_list;
	while( tmp->next != NULL){
		tmp = tmp->next;
	}
	tmp->next = new_user;

}

void removeUser(int sock){

	user *tmp = user_list;
	user *prec = NULL;

	while(tmp != NULL){
		if(tmp->sock == sock){
			break;
		}	
	
		prec = tmp;
		tmp = tmp->next;
	}
	
	FD_CLR(sock,&master);
	close(sock);

	if(tmp == NULL){
		return;	
	}

	n_users--;

	if(tmp->status == BUSY){			//il tizio stava giocando
		user *him = tmp->pending_request;
		if(!sendInt(him->sock,OPP_DISCONNECTED_TCP)) 		return;
		him->status = FREE;
		printf("%s ha vinto!\n",him->username);			//il nome dell'avversario e' andato perso
		printf("%s e' libero\n",him->username);


	}
	

	printf("%s si e' disconnesso\n",tmp->username);

	if(tmp == user_list){
		user_list = user_list->next;
		free(tmp->username);
		free(tmp);
		return;
	}

	prec->next = tmp->next;
	free(tmp);

}

void who_function(int sock){

	if(!sendInt(sock,n_users)) return;
	
	user *tmp = user_list;	
	while(tmp != NULL){
		if(!sendString(sock,tmp->username)) 	return;
		if(!sendInt(sock,tmp->status))		return;		

		tmp = tmp->next;
	}

}

void login_function(int sock,struct sockaddr_in socket_full){

	char *username;
	int port;

	username = recvString(sock);
	if(username == NULL){
		printf("Username non ricevuto\n");
		//gestione errore
	}
	
	if(!recvInt(sock,&port)){
		printf("Errore\n");
	}
	
	if(existingUsername(username) || port < 0){
		if(!sendInt(sock,LOGIN_FAIL)) 	return;
		return;
	}	

	addUser(username,port,sock,socket_full);
	if(!sendInt(sock,LOGIN_OK))		return;

	printf("%s si e' connesso\n",username);
	printf("%s e' libero\n",username);
}

void quit_function(int sock){

	removeUser(sock);

}

void connect_function(int sock){

	char *username;
	user *me,*him;

	//int status;
	username = recvString(sock);
	if(username == NULL)				return;	

	me = searchUserBySocket(sock);			//se stesso
	me->status = CONNECTING;
	
	if(strcmp(username,me->username) == 0){		//connessione a se stessi
		if(!sendInt(sock,CONNECT_NOUSER))	return;
		return;
	}

	
	him = searchUserByName(username);		//non esiste

	if(him == NULL){
		me->status = FREE;
		if(!sendInt(sock,CONNECT_NOUSER))		return;
		return;
	}

	if(him->status == CONNECTING || him->status == BUSY){		//occupato
		me->status = FREE;
		if(!sendInt(sock,CONNECT_BUSY))		return;
		return;
	}

	him->pending_request = me;
	me->pending_request = him;

	printf("%s vuole connettersi a %s\n",me->username,him->username);

	if(!sendInt(him->sock,CONNECT_REQ))		return;
	if(!sendString(him->sock,me->username))		return;

}


void connect_acpt(int sock){

	user *me,*him;
	//int status;

	me = searchUserBySocket(sock);

	him = me->pending_request;
	
	printf("%s e %s stanno giocando\n",me->username,him->username);

	him->status = me->status = BUSY;		
	
	if(!sendInt(sock,CONNECT_DATA))		return;		//mando la porta		
	if(!sendString(sock,him->ip))		return;		//mando a lui l'ip
	if(!sendInt(sock,him->port))		return;		//mando la porta		

	if(!sendInt(him->sock,CONNECT_ACPT))	return;		//avviso che ha accettato la partita
	if(!sendString(him->sock,me->ip))	return;		
	if(!sendInt(him->sock,me->port))	return;

	//me->pending_request = NULL; ??

}

void connect_rfsd(int sock){

	user *me,*him;
	//int status;

	me = searchUserBySocket(sock);
	him = me->pending_request;

	him->status = me->status = FREE;		
	if(!sendInt(him->sock,CONNECT_REFUSED))	return;

	me->pending_request = NULL;
	him->pending_request = NULL;

}

void end_game(int sock){

	user *me,*him;
	me = searchUserBySocket(sock);
	him = me->pending_request;

	me->status = him->status = FREE;
	
	printf("%s ha vinto la partita contro %s\n",him->username,me->username);
	printf("%s e' libero\n",him->username);
	printf("%s e' libero\n",me->username);


	me->pending_request = NULL;
	him->pending_request = NULL;

}


void disconnect_function(int sock){

	user *me,*him;
	me = searchUserBySocket(sock);
	him = me->pending_request;

	me->status = him->status = FREE;
	if(!sendInt(him->sock,WON_RETIRED))	return;
	
	printf("%s ha vinto la partita contro %s\n",him->username,me->username);
	printf("%s e' libero\n",him->username);
	printf("%s e' libero\n",me->username);


	me->pending_request = NULL;
	him->pending_request = NULL;

}

void handle_timeout(int sock){

	user *me,*him;
	me = searchUserBySocket(sock);
	him = me->pending_request;

	if(him == NULL){	// non credo si possa arrivare qui, il server segnala la disconnessione prima che scatti il TO
		me->status = FREE;
		printf("%s e' libero\n",me->username);
		return;
	}

	him->status = me->status = FREE;

	me->pending_request = him->pending_request = NULL;
	if(!sendInt(him->sock,YOU_TIMEOUT))				return;
	

	printf("%s ha vinto la partita contro %s\n",me->username,him->username);
	printf("%s e' libero\n",him->username);
	printf("%s e' libero\n",me->username);


}


void select_command(int cmd,int sock,struct sockaddr_in socket_full){

	switch (cmd){
		case LOGIN_COMMAND:
			login_function(sock,socket_full);
			break;
		case WHO_COMMAND:
			who_function(sock);
			break;
		case QUIT_COMMAND:
			quit_function(sock);	
			break;
		case CONNECT_COMMAND:
			connect_function(sock);
			break;	
		case CONNECT_ACPT:
			connect_acpt(sock);
			break;
		case CONNECT_RFSD:
			connect_rfsd(sock);
			break;
		case DISCONNECT_COMMAND:
			disconnect_function(sock);
			break;
		case END_GAME:
			end_game(sock);
			break;
		case NOTIFY_OPP_TIMEOUT:
			handle_timeout(sock);
			break;
		}


}


int initialize_server(int port){
	
	int listener;
	int status;

	listener = socket(AF_INET, SOCK_STREAM, 0);
	if(listener < 0){
		printf("[Errore] socket\n");
		exit(-1);
	}

	struct sockaddr_in listenerAddress;
	memset(&listenerAddress,0,sizeof(listenerAddress));
	
	listenerAddress.sin_port = htons(port);
	listenerAddress.sin_family = AF_INET;
	inet_pton(AF_INET,"0.0.0.0",&listenerAddress.sin_addr);


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


	return listener;

}

int main(int argc,char **argv){

	if(argc != 2){
		printf("[Errore] parametri errati\n");
		exit(-1);
	}


	struct sockaddr_in clientAddress;

	int port = atoi(argv[1]);
	int fdmax,server_socket;
	unsigned int addrlen;
	int i,status,new_sock;
	int cmd;
	fd_set read_fds;
		
	/*if(porta < 0){
		pritnf("[Errore] porta errata");
		exit(-1);
	}*/

	user_list = NULL;
	n_users = 0;


	memset(&clientAddress,0,sizeof(clientAddress));

	server_socket = initialize_server(port);

	printf("[LOG] Attendo connessioni sulla porta %d\n",port);

	
	FD_ZERO(&master);	
	FD_ZERO(&read_fds);
	FD_SET(server_socket,&master);

	fdmax = server_socket;
	
	while(true){
		read_fds = master;
		if(select(fdmax+1,&read_fds,NULL,NULL,NULL) <=0){
			perror("[Errore] Select");
			exit(-1);
		}

		for(i = 0; i <= fdmax; i++){
			if(FD_ISSET(i,&read_fds)){			
				if(i==server_socket){			//qualcuno si vuole connettere

					memset(&clientAddress,0,sizeof(clientAddress));
					addrlen = sizeof(clientAddress);
					new_sock = accept(server_socket,(struct sockaddr*)&clientAddress,&addrlen);
					if(new_sock < 0){
						perror("[Errore] accept\n");
						continue;
					}					
					FD_SET(new_sock,&master);
					if(new_sock > fdmax) 
						fdmax = new_sock;
					
					printf("Connessione stabilita con il client\n");
					continue;

				} else {				//qualcuno vuole scrivere
					status = recvInt(i,&cmd);
					if(status == false){
						removeUser(i);				
					} else {
						select_command(cmd,i,clientAddress);
					}
				}
			}

		}	

	}
	
	close(server_socket);

	return 0;

}
