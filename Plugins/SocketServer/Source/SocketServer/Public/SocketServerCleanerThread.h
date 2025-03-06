// Copyright 2022 David Romanski (Socke). All Rights Reserved.

#pragma once

#include "SocketServer.h"


class SOCKETSERVER_API FSocketServerCleanerThread : public FRunnable {

public:

	FSocketServerCleanerThread();


	virtual uint32 Run() override;

	void addSession(FSocketServerPluginSession& session);
	void changeSettings(bool showLogs, int32 minLiveTimeInSeconds);

private:
	bool showLogs = false;
	int32 minLiveTimeInSeconds = 10;
	FRunnableThread* thread = nullptr;
	TQueue<FSocketServerPluginSession> sessionQueue;
};
