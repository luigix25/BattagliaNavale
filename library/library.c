#include "library.h"

const char *commands_list[4] = {"!help","!who","!connect","!quit"};
const char *commands_list_game[4] = {"!help","!disconnect","!shot","!show"};

int sendUDPInt(int sd,struct sockaddr_in *sv_addr,int value){
	uint32_t tosend;
	int ret;
	tosend = htonl(value);

	ret = sendto(sd,&tosend,sizeof(uint32_t),0,(struct sockaddr*)sv_addr,sizeof(*sv_addr));
	if(ret <= 0){
		perror("[Errore sendUDPInt]");
		return ret;
	}

	return (ret == sizeof(uint32_t));

}

int recvUDPInt(int sd,struct sockaddr_in *cl_addr,int *value){
	uint32_t torecv;
	int ret,addrlen;

	addrlen = sizeof(cl_addr);

	ret = recvfrom(sd,&torecv,sizeof(uint32_t),0,(struct sockaddr*)cl_addr,&addrlen);
	if(ret <= 0){
		perror("[Errore recvUDPInt]");
		return ret;
	}

	*value = ntohl(torecv);

	return (ret == sizeof(uint32_t));

}


int sendInt(int sd,int value){
	int status;
	uint32_t tosend;

	tosend = htonl(value);
	//status = send(sd, &tosend, sizeof(uint32_t), 0);
	status = sendto(sd, &tosend, sizeof(uint32_t), 0,NULL,0);
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
	
	//ret = send(sd,buffer,len,0);
	ret = sendto(sd,buffer,len,0,NULL,0);
	if(ret < len){
		perror("[Errore] send");
	} 
	
	return (len == ret);

}

int sendUDPString(int sd,char *buffer,struct sockaddr_in *sv_addr){
	int len,ret;//,addrlen;

	//addrlen = sizeof(cl_addr);

	len = strlen(buffer)+1;

	if(!sendUDPInt(sd,sv_addr,len))
		return false;
	
	//ret = send(sd,buffer,len,0);
	ret = sendto(sd,buffer,len,0,(struct sockaddr*)sv_addr,sizeof(*sv_addr));
	if(ret < len){
		perror("[Errore] UDP send");
	} 
	
	return (len == ret);

}



char* recvUDPString(int sd,struct sockaddr_in *cl_addr){
	int ret,len,addrlen;
    	addrlen = sizeof(*cl_addr);

	if(!recvUDPInt(sd,cl_addr,&len)){
		perror("[Errore] UDP recv");
		return NULL;
	}
	char *buffer = (char *)malloc(len);

	ret = recvfrom(sd,buffer,len,0,(struct sockaddr *)cl_addr,&addrlen);
	if(ret < len){
		perror("[Errore] recv");
		return NULL;	
	}
	return buffer;
}


char* recvString(int sd){
	int ret,len;

	if(!recvInt(sd,&len)){
		perror("[Errore] recv");
		return NULL;
	}
	char *buffer = (char *)malloc(len);

	//ret = recv(sd,buffer,len,MSG_WAITALL);
	ret = recvfrom(sd,buffer,len,0,NULL,0);
	if(ret < len){
		perror("[Errore] recv");
		return NULL;	
	}
	return buffer;
}

int recvInt(int sd,int* val){

	int ret,tmp;

	ret = recvfrom(sd,&tmp,sizeof(uint32_t),0,NULL,0);
	if(ret < sizeof(uint32_t)){
		perror("[Errore] recv");
	}

	*val = ntohl(tmp);

	return (ret == sizeof(uint32_t));

}

