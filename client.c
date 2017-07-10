#include "library/library.h"

#define N_NAVI 2


fd_set master;
int socket_udp,socket_tcp;
struct sockaddr_in opponent;

void handle_connection_request(int);
void handle_connection_accepted(int);
void handle_receive_data(int);
void cmd_show();
void wait_for_opponent(short);
void protocol_error(int);


short in_game;
short waiting;
short ships_left;
char grid[6][6];			// - vuota; 0 nave integra; X nave colpita; T tentativi utente; ? incognita
char grid_opponent[6][6];
char *last_shot;

struct sockaddr_in setup_sockaddr(char *ip,int port){

	struct sockaddr_in sv_addr;

	memset(&sv_addr,0,sizeof(sv_addr));

	sv_addr.sin_family = AF_INET;
	sv_addr.sin_port = htons(port);
	inet_pton(AF_INET,ip,&sv_addr.sin_addr);
	return sv_addr;

}


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

int check_position(char *position){

	if(strlen(position)>2)
		return false;

	char a = position[0];
	char b = position[1];

	if(!(b >='0' && b<='5'))
		return false;

	int n = b-'0';

	if((a >= 'a' && a <='f') && (n>=0 && n<=5))			//credo sia ridondante 
		return true;
	else 
		return false;


}

int place_ship(char *position){

	

	int row = position[0] - 'a';
	int col = position[1] - '0';
	
	if(grid[row][col] != '-')
		return false;
	
	grid[row][col] = '0';
	return true;


}

void setup_grid(){
	
	ships_left = N_NAVI;

	int i,j,to_place;
	to_place = N_NAVI;				//sarebbe 7-1
	to_place--;
	char *position;

	for(i=0;i<6;i++)
		for(j=0;j<6;j++){
			grid[i][j] = '-';
			grid_opponent[i][j] = '?';
		}

	while(to_place >=0){
		printf("Dove vuoi posizionare la nave?\n");
		scanf("%ms",&position);

		if(!check_position(position)){
			printf("Posizione non valida; Formato a-z0-6 es: a0\n");
		} else {
			if(!place_ship(position)){
				printf("Posizione gia' occupata\n");
			} else {
				to_place--;
				if(to_place <0)
					continue;
				printf("Nave aggiunta correttamente, ne rimangono %d\n",to_place+1);
			}

		}

	}


}


void cmd_help_game(){

	char *desc[4] = {"--> mostra l'elenco dei comandi disponibili",
			"--> disconnette il client dall'attuale partita",
			"square --> fai un tentativo con la casella square",
			"--> visualizza la griglia di gioco\n"};

	int i;
	printf("\n");	
	printf("Sono disponibili i seguenti comandi:\n");
	for(i=0;i<4;i++){
		printf("%s %s\n",commands_list_game[i],desc[i]);
	}

}

void cmd_connect(int sock){

	char *username;
	//int port;

	int result;
	scanf("%ms",&username);
	if(!sendInt(sock,CONNECT_COMMAND))	return;
	if(!sendString(sock,username))		return;
	if(!recvInt(sock,&result))		return;

	switch(result){
		case CONNECT_REQ:
			handle_connection_request(sock);
			break;
		case CONNECT_ACPT:
			handle_connection_accepted(sock);
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
			handle_receive_data(sock);
			break;
	}


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

void cmd_disconnect(int sock){
	printf("Disconnessione avvenuta con successo: TI SEI ARRESO\n");
	in_game = false;

	if(!sendInt(sock,DISCONNECT_COMMAND))		return;

}


void cmd_shot(int sock){

	char *position;
	int row,col;

	scanf("%ms",&position);
	
	if(check_position(position) == false){
		printf("Posizione non valida!\n");
		return;	
	} 

	row = position[0] - 'a';
	col = position[1] - '0';
	
	if(grid_opponent[row][col] != '?'){
		printf("Hai gia' sparato qui!\n");
		return;
	} 

	last_shot = position;		

	
	if(!sendUDPInt(socket_udp,&opponent,SHOT_DATA))		protocol_error(sock);
	if(!sendUDPString(socket_udp,position,&opponent))	protocol_error(sock);
	
	wait_for_opponent(true);


}


void select_command(int sock,char *buffer){

	if(strcmp("!help",buffer) == 0){
		cmd_help();
	} else if(strcmp("!who",buffer) == 0){
		cmd_who(sock);
	} else if(strcmp("!quit",buffer) == 0){
		cmd_quit(sock);
	} else if(strcmp("!connect",buffer) == 0){
		cmd_connect(sock);
	} /*else {
		printf("Comando non riconosciuto\n");
	}*/

	free(buffer);

}

void select_command_game(int sock,char *buffer){

	if(strcmp("!help",buffer) == 0){
		cmd_help_game();
	} else if(strcmp("!disconnect",buffer) == 0){
		cmd_disconnect(sock);
	} else if(strcmp("!shot",buffer) == 0){
		cmd_shot(sock);
	} else if(strcmp("!show",buffer) == 0){
		cmd_show();
	} /*else {
		printf("Comando non riconosciuto\n");
	}*/

	free(buffer);

}


void read_input(int sock){

	char *buffer = 0;
	fflush(stdout);
	scanf("%ms",&buffer);

	if(!in_game)
		select_command(sock,buffer);
	else
		select_command_game(sock,buffer);
}

void wait_for_opponent(short cmd){

	waiting = cmd;
	if(cmd){
		FD_CLR(0,&master);
	} else {
		FD_SET(0,&master);
	}
}

void startGame(int sock,char *ip,int port,int socket_udp,int i_start){

	//creo indirizzo per UDP
	//struct sockaddr_in sv_addr;

	opponent = setup_sockaddr(ip,port);
	setup_grid();


	if(i_start == true){
		printf("E' il tuo turno!\n");
	} else {
		printf("In attesa dell'avversario\n");
		wait_for_opponent(true);
	}

}

void handle_connection_request(int sock){
	
	char *str = recvString(sock);
	char answ = 'x';

	do{
		printf("%s si vuole connettere a te, Accetti? (y/n)",str);
		fflush(stdout);
		getchar();				//flush per ritorno a capo
		scanf("%c",&answ);
		
	} while(answ != 'y' && answ != 'n');


	if(answ == 'y'){
		in_game = true;
		if(!sendInt(sock,CONNECT_ACPT))	return;				//gestire l'errore SISTEMARE
	} else {
		if(!sendInt(sock,CONNECT_RFSD))	return;
	}
}

void handle_connection_accepted(int sock){

	printf("L'utente ha accettato la partita\n");

	in_game = true;
	char *ip;
	int port;

	ip = recvString(sock);						//mi aspetto l'ip
	if(ip == NULL)				return;
	if(!recvInt(sock,&port))		return;				//porta altro client
	
	printf("%s:%d\n",ip,port);						//ELIMINARE
	
	//int res = sendUDPInt(socket_udp,sv_addr,555);

	startGame(sock,ip,port,socket_udp,true);


}

void handle_receive_data(int sock){
	char *ip;
	int port;

	ip = recvString(sock);						//mi aspetto l'ip
	if(ip == NULL)				return;
	if(!recvInt(sock,&port))		return;				//porta altro client
	printf("%s:%d\n",ip,port);					//ELIMINARE
			

	//struct sockaddr_in r = setup_sockaddr(ip,port);


	startGame(sock,ip,port,socket_udp,false);

}


void cmd_show(){
	int i,j;

	printf("- vuota; 0 nave integra; X nave colpita; T tentativi avversario; ? incognita\n \n");

	printf("La tua griglia:\n\n");

	for(i=0;i<6;i++){
		for(j=0;j<6;j++){
			printf("%c",grid[i][j]);
		}
		printf("\n");
	}


	printf("La griglia del tuo avversario:\n\n");

	for(i=0;i<6;i++){
		for(j=0;j<6;j++){
			printf("%c",grid_opponent[i][j]);
		}
		printf("\n");
	}


}

void protocol_error(int sock_tcp){

	printf("Errore del gioco. Termino\n");
	close(sock_tcp);
	exit(-1);

}

void handle_data_shot(int socket_tcp){

	char *recvd;
	recvd = recvUDPString(socket_udp,&opponent);
	if(recvd == NULL)
		protocol_error(socket_tcp);


	printf("L'avversario ha sparato in %s\n",recvd);


	int row = recvd[0] - 'a';
	int col = recvd[1] - '0';
	char c;
	int tosend;

	if(grid[row][col] == '-'){ // 0 è vuoto
			c = 'T';
			tosend = WATER;
			printf("L'avversario ha colpito l'acqua\n");
	} else {
		c = 'X';
		tosend = HIT;
		printf("L'avversario ti ha colpito una nave\n");
		ships_left--;
	}


	if(ships_left == 0){
		printf("Hai perso, era la tua ultima nave!\n");
		if(!sendUDPInt(socket_udp,&opponent,YOU_WON))		protocol_error(socket_tcp);
		if(!sendInt(socket_tcp,END_GAME))			return ;						//SISTEMARE ?
	
		in_game = false;
		wait_for_opponent(false);
		return;
	}


	grid[row][col] = c;

	if(!sendUDPInt(socket_udp,&opponent,RESPONSE_SHOT))	protocol_error(socket_tcp);
	if(!sendUDPInt(socket_udp,&opponent,tosend))		protocol_error(socket_tcp);


	wait_for_opponent(false);
	printf("E' il tuo turno\n");

}

void handle_response_shot(){

	int resp,row,col;
	char c;
	if(!recvUDPInt(socket_udp,&opponent,&resp)){
		protocol_error(socket_tcp);
	}

	if(resp == WATER){
		c = '-';
		printf("Hai colpito l'acqua\n");
	} else if(resp == HIT){
		c = 'X';
		printf("Hai colpito una nave\n");
	}

	row = last_shot[0] - 'a';
	col = last_shot[1] - '0';

	grid_opponent[row][col] = c;

}

void handle_win(){

	printf("Congratulazioni hai vinto!\n");
	in_game = false;
	wait_for_opponent(false);

}

void select_command_udp(int sock,int cmd,int socket_tcp){

	switch(cmd){
		case SHOT_DATA:
			handle_data_shot(socket_tcp);
			break;
		case RESPONSE_SHOT:
			handle_response_shot();
			break;
		case YOU_WON:
			handle_win();
			break;
	}

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
		case WON_RETIRED:
			printf("Complimenti hai vinto! Il tuo avversario si e' ritirato\n");
			in_game = false;
			break;
		case OPP_DISCONNECTED_TCP:
			printf("Il tuo avversario non e' più connesso con il server di gioco, Hai vinto!\n");
			in_game = false;
			wait_for_opponent(false);
			break;
		case YOU_TIMEOUT:
			printf("Timeout, hai perso!\n");
			in_game = false;
			wait_for_opponent(false);
			break;

	}

}

int main(int argc,char **argv){

	if(argc < 3){
		printf("[Errore] ip e porta necessari\n");
		exit(-1);
	}

	int status,portServer,port_udp,cmd;
	struct sockaddr_in serverAddress;

	int i,fdmax;
	fd_set read_fds;

	portServer = atoi(argv[2]);

	socket_tcp = socket(AF_INET,SOCK_STREAM,0);
	if(socket_tcp < 0){
		perror("[Errore] socket\n");
		return -1;
	}

	memset(&serverAddress,0,sizeof(serverAddress));
	
	serverAddress.sin_family = AF_INET;
	serverAddress.sin_port = htons(portServer);
	inet_pton(AF_INET,argv[1],&serverAddress.sin_addr);
	

	status = connect(socket_tcp, (struct sockaddr*)&serverAddress,sizeof(serverAddress));
	if(status < 0){
		perror("[Errore] connect\n");
		return -1;
	}
	
	printf("\nConnessione al server %s (port %d) effettuata con successo\n",argv[1],portServer);

	
	cmd_help();
	cmd_login(socket_tcp,&socket_udp,&port_udp);

	in_game = false;
	waiting = false;

	FD_ZERO(&master);	
	FD_ZERO(&read_fds);

	FD_SET(socket_udp,&master);
	FD_SET(socket_tcp,&master);
	FD_SET(0,&master);		// 0 è stdin

	fdmax = (socket_tcp > socket_udp)?socket_tcp:socket_udp;

	struct timeval timeout = {10,0}; 


	while(true){

		if(!in_game){
			printf("\r>");
			fflush(stdout);
		} else {
			if(!waiting){				//non molto elegante
				printf("#");
				fflush(stdout);
			}		
		}

		
		read_fds = master;

		if(waiting){
			if(select(fdmax+1,&read_fds,NULL,NULL,&timeout) <=0){				//SISTEMARE TIMEOUT

				if(in_game == false){
					continue;
				}

				printf("Timeout dell'avversario, hai vinto!\n");
				if(!sendInt(socket_tcp,NOTIFY_OPP_TIMEOUT))	return -1;
				wait_for_opponent(false);

				in_game = false;				

                		timeout.tv_sec = 10;
				continue;
			}
		} else {
			if(select(fdmax+1,&read_fds,NULL,NULL,NULL) <=0){
				perror("Errore select");
				exit(-1);
			}

		}



		for(i = 0; i <= fdmax; i++){
			if(FD_ISSET(i,&read_fds)){
				if(i == 0){			//stdin
					read_input(socket_tcp);					
					//continue;
				} else if(i == socket_tcp) {			//server tcp
					if(!recvInt(i,&cmd))			return -1;
					select_command_server(i,cmd);	
				} else if(i == socket_udp){			//server udp
					if(!recvUDPInt(i,&opponent,&cmd)){
						protocol_error(socket_tcp);
					}
					select_command_udp(i,cmd,socket_tcp);	

				}

			}

		}
		
		//read_input(socket_server);

	}	

	
	return 0;

}
