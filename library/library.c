#include "library.h"

const char *commands_list[4] = {"!help","!who","!connect","!quit"};

int sendInt(int sd,int value){
	int status;
	uint32_t tosend;

	tosend = htonl(value);
	status = send(sd, &tosend, sizeof(uint32_t), 0);
	if(status < sizeof(uint32_t))	{
		perror("[Errore] send");
	}

	return (status==sizeof(uint32_t));

}

int sendString(int sd,char *buffer){
	int len,ret;

	len = strlen(buffer)+1;

	if(!sendInt(sd,len))
		return false;
	
	ret = send(sd,buffer,len,0);
	if(ret < sizeof(uint32_t)){
		perror("[Errore] send");
	} 
	
	return (len == ret);

}

char* recvString(int sd){
	int ret,len;

	if(!recvInt(sd,&len)){
		perror("[Errore] recv");
		return NULL;
	}
	char *buffer = (char *)malloc(len);

	ret = recv(sd,buffer,len,0);
	if(ret < sizeof(uint32_t)){
		perror("[Errore] recv");
		return NULL;	
	}
	return buffer;
}

int recvInt(int sd,int* val){

	int ret,tmp;

	ret = recv(sd,&tmp,sizeof(uint32_t),0);
	if(ret < sizeof(uint32_t)){
		perror("[Errore] recv");
	}

	*val = ntohl(tmp);

	return (ret == sizeof(uint32_t));

}

