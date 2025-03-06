// Copyright 2022 David Romanski (Socke). All Rights Reserved.

#pragma once

#include "SocketServer.h"


class SOCKETSERVER_API FSocketServerTCPClientSendDataThread : public FRunnable {

public:

	FSocketServerTCPClientSendDataThread(USocketServerTCP* tcpServerP, FSocketServerPluginSession& sessionP);
	~FSocketServerTCPClientSendDataThread();

	virtual uint32 Run() override;
	FRunnableThread* getThread();
	void setThread(FRunnableThread* threadP);
	void stopThread();
	bool isRun();
	void setMessage(FString messageP, TArray<uint8> byteArrayP);
	void sendMessage(FString messageP, TArray<uint8> byteArrayP);
	void pauseThread(bool pause);

private:
	USocketServerTCP* tcpServer = nullptr;
	FSocketServerPluginSession session;
	FRunnableThread* thread = nullptr;
	FSocket* socket = nullptr;
	bool					run = true;
	bool					paused;
	bool waitForInit = true;
	TQueue<FString> messageQueue;
	TQueue<TArray<uint8>> byteArrayQueue;
};