// Copyright 2022 David Romanski (Socke). All Rights Reserved.

#pragma once

#include "SocketServer.h"


class SOCKETSERVER_API FSocketServerTCPFileHandlerThread : public FRunnable {

public:

	FSocketServerTCPFileHandlerThread(USocketServerTCP* tcpServerP, FSocketServerPluginSession& sessionP);
	~FSocketServerTCPFileHandlerThread();

	virtual uint32 Run() override;
	void triggerFileOverTCPProgress(FString sessionIDP, FString filePathP, float percentP, float mbitP, int64 bytesReceivedP, int64 fileSizeP);
	void triggerFileTransferOverTCPInfoEvent(FString messageP, FString sessionIDP, FString filePathP, bool successP);
	void sendEndMessage(FString fullFilePathP, FString tokenP, FString md5ClientP, FString sessionIDP, FSocket* clientSocketP);
	void stopThread();


private:
	USocketServerTCP* tcpServer = nullptr;
	FSocketServerPluginSession session;
	bool fileServer = false;
	FRunnableThread* thread = nullptr;
	bool run = true;
	int32 commandProgress = 0;
	int64 fileSize;
	FSocketServerTCPClientSendFileToThread* sendFileThread = nullptr;
};
