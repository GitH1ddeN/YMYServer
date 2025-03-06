// Copyright 2022 David Romanski (Socke). All Rights Reserved.

#pragma once

#include "SocketServer.h"

class SOCKETSERVER_API FSocketServerTCPClientSendFileToThread : public FRunnable {

public:

	FSocketServerTCPClientSendFileToThread(FSocket* socketP, FString filePathP, int64 fileSizeP, int64 startPositionP, FSocketServerPluginSession& sessionP);
	~FSocketServerTCPClientSendFileToThread();

	virtual uint32 Run() override;
	void triggerFileOverTCPProgress(FString sessionIDP, FString filePathP, float percentP, float mbitP, int64 bytesReceivedP, int64 fileSizeP);
	void triggerFileTransferOverTCPInfoEvent(FString messageP, FString sessionIDP, FString filePathP, bool successP);
	void stopThread();
	bool isRun();


private:
	FSocket* socket = nullptr;
	FString filePath = FString();
	int64 fileSize = 0;
	int64 startPosition = 0;
	FSocketServerPluginSession session;
	FRunnableThread* thread = nullptr;
	bool					run = true;

};
