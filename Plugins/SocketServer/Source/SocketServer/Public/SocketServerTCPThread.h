// Copyright 2022 David Romanski (Socke). All Rights Reserved.

#pragma once

#include "SocketServer.h"

class SOCKETSERVER_API FSocketServerTCPThread : public FNonAbandonableTask {

public:

	FSocketServerTCPThread(USocketServerTCP* tcpServerP, EReceiveFilterServer receiveFilterP, bool& runP);

	void DoWork();

	void stopThread();

	FORCEINLINE TStatId GetStatId() const {
		return TStatId();
	}

private:
	USocketServerTCP* tcpServer;
	EReceiveFilterServer	receiveFilter;
	bool& run;
};