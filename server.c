#include "library/library.h"

typedef struct user{
	char *username;
	int port;
	struct user *next;
	unsigned int status;
	int sock;
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

void addUser(char *username,unsigned int port,int sock){
	user *new_user = malloc(sizeof(user));		
	new_user->username = username;
	new_user->port = port;	
	new_user->next = NULL;
	new_user->status = FREE;
	new_user->sock = sock;

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
	}
	
	if(existingUsername(username) || port < 0){
		if(!sendInt(sock,LOGIN_FAIL)) 	return;
		return;
	}	

	addUser(username,port,sock);
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

	username = recvString(sock);
	if(username == NULL)				return;	

	me = searchUserBySocket(sock);			//se stesso
	me->status = CONNECTING;
	
	if(strcmp(username,me->username) == 0){		//connessione a se stessi
		if(!sendInt(sock,CONNECT_NOUSER))	return;
		return;
	}

	
	him = searchUserByName(username);

	if(him == NULL){
		me->status = FREE;
		if(!sendInt(sock,CONNECT_NOUSER))		return;
		return;
	}

	if(him->status == CONNECTING || him->status == BUSY){
		me->status = FREE;
		if(!sendInt(sock,CONNECT_BUSY))		return;
		return;
	}

	if(!sendInt(him->sock,CONNECT_REQ))		return;
	if(!sendString(him->sock,me->username))		return;
			

}

void select_command(int cmd,int sock){

	switch (cmd){
		case LOGIN_COMMAND:
			login_function(sock);
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
	fd_set read_fds;
		
	/*if(porta < 0){
		pritnf("[Errore] porta errata");
		exit(-1);
	}*/

	user_list = NULL;
	n_users = 0;

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
			exit(-1);
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
						removeUser(i);				
					} else {
						select_command(cmd,i);
					}
				}
			}

		}	

	}


	return 0;

}
