// Copyright 2022 David Romanski (Socke). All Rights Reserved.

#pragma once

#include "SocketServer.h"


class SOCKETSERVER_API FSocketServerTCPClientReceiveDataThread : public FRunnable {

public:

	FSocketServerTCPClientReceiveDataThread(USocketServerTCP* tcpServerP,
		FSocketServerPluginSession& sessionP,
		EReceiveFilterServer receiveFilterP);
	
	~FSocketServerTCPClientReceiveDataThread();

	virtual uint32 Run() override;
	void triggerMessageEvent(TArray<uint8>& byteDataArray, FString& sessionID, FString& serverID, bool addNullTerminator = true);
	void readDataLength(TArray<uint8>& byteDataArray, int32& byteLenght);
	void stopThread();

private:
	USocketServerTCP* tcpServer = nullptr;
	FSocketServerPluginSession session;
	EReceiveFilterServer receiveFilter;
	FRunnableThread* thread = nullptr;
	bool run = true;
	bool deathConnection = false;
};
