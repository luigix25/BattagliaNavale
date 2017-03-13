#include "library/library.h"

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

void cmd_login(int sock){
	int port;
	char *buffer;

	if(!sendInt(sock,LOGIN_COMMAND)){ 
		printf("Errore cmd login\n");
		return;	
	}
	
	printf("Inserisci il tuo nome: ");
	fflush(stdout);	
	scanf("%ms",&buffer);
	fflush(stdout);
	printf("Inserisci la porta: ");
	scanf("%d",&port);

	if(!sendString(sock,buffer)) 	return;
	if(!sendInt(sock,port))		return;

}

int main(int argc,char **argv){

	if(argc < 3){
		printf("[Errore] ip e porta necessari\n");
		exit(-1);
	}

	int sock,status,nbyte,port;
	struct sockaddr_in serverAddress;
	char buffer[128];

	port = atoi(argv[2]);

	sock = socket(AF_INET,SOCK_STREAM,0);
	if(sock < 0){
		perror("[Errore] socket\n");
		return -1;
	}

	memset(&serverAddress,0,sizeof(serverAddress));
	
	serverAddress.sin_family = AF_INET;
	serverAddress.sin_port = htons(port);
	inet_pton(AF_INET,argv[2],&serverAddress.sin_addr);
	

	status = connect(sock, (struct sockaddr*)&serverAddress,sizeof(serverAddress));
	if(status < 0){
		perror("[Errore] connect\n");
		return -1;
	}
	
	printf("\nConnessione al server %s (port %d) effettuata con successo\n",argv[2],port);

	
	cmd_help();
	cmd_login(sock);		
		
	int a;
	read(sock,&a,4,0);

	return 0;

}
