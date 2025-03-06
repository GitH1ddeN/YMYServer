// Copyright 2022 David Romanski (Socke). All Rights Reserved.

#pragma once

#include "SocketServer.h"


class SOCKETSERVER_API FSocketServerUDPClientSendDataThread : public FRunnable {

public:

	FSocketServerUDPClientSendDataThread(USocketServerUDP* udpServerP);

	~FSocketServerUDPClientSendDataThread();

	virtual uint32 Run() override;
	FRunnableThread* getThread();
	void setThread(FRunnableThread* threadP);
	void stopThread();
	bool isRun();
	void sendMessage(FString ip, int32	port, FString message, TArray<uint8>	bytes, FSocket* socketUDP);
	void pauseThread(bool pause);

private:
	USocketServerUDP* udpServer;
	FRunnableThread* thread = nullptr;
	bool					run = true;
	bool					paused;
	TQueue<FSendUDPMessageStruct> messageQueue;
};