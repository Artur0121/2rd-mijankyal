#pragma once
#include "UDPSocket.h"
#define SERVER "127.0.0.1"	
#define BUFLEN 	1024		
#define PORT 8888			

class UDPServer : public UDPSocket
{
private:
	unsigned short		mServerPort;
	struct sockaddr_in	mServer;
	UDPServer();
public:
	UDPServer(unsigned short serverport);
	virtual ~UDPServer();
};
