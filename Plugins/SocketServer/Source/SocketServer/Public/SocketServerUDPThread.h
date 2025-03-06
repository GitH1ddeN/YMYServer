// Copyright 2022 David Romanski (Socke). All Rights Reserved.

#pragma once

#include "SocketServer.h"


class SOCKETSERVER_API FSocketServerUDPThread : public FRunnable {

public:

	FSocketServerUDPThread(USocketServerUDP* udpServerP, bool multicastP);

	~FSocketServerUDPThread();

	virtual uint32 Run() override;

	void stopThread();


private:
	FString message;
	USocketServerUDP* udpServer;
	bool multicast;
	FRunnableThread* thread = nullptr;
	bool run = true;

};