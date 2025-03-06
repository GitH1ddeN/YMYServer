// Copyright 2022 David Romanski (Socke). All Rights Reserved.

#include "SocketServerCleanerThread.h"

FSocketServerCleanerThread::FSocketServerCleanerThread() {
	FString threadName = "FSocketServerPluginCleanerThread_" + FGuid::NewGuid().ToString();
	thread = FRunnableThread::Create(this, *threadName, 0, EThreadPriority::TPri_Lowest);
}


void FSocketServerCleanerThread::addSession(FSocketServerPluginSession& session) {
	session.addToCleanerTime = FDateTime::Now().GetTicks();
	sessionQueue.Enqueue(session);
}

void FSocketServerCleanerThread::changeSettings(bool showLogsP, int32 minLiveTimeInSecondsP){
	showLogs = showLogsP;
	minLiveTimeInSeconds = minLiveTimeInSecondsP;
}

uint32 FSocketServerCleanerThread::Run() {
	while (true) {
		TArray<FSocketServerPluginSession> tryItAgain;

		while (sessionQueue.IsEmpty() == false) {
			FSocketServerPluginSession session;
			sessionQueue.Dequeue(session);

			//if (session.recieverThread == nullptr && session.sendThread == nullptr) {
			//	continue;
			//}

			
			//one second = 10000000 ticks
			if ((FDateTime::Now().GetTicks() - session.addToCleanerTime) < (10000000 * minLiveTimeInSeconds)) {
				tryItAgain.Add(session);
				continue;
			}

			if (showLogs) {
				UE_LOG(LogTemp, Display, TEXT("SocketServer: Clean Session: %s"), *session.sessionID);
			}

		
			delete session.tcpSendThread;
			delete session.tcpRecieverThread;
			delete session.tcpFileHandlerThread;

			delete session.udpServerThread;
			delete session.udpSocketReceiver;
			delete session.udpSendThread;

			delete session.socket;
			
		}

		for (int32 i = 0; i < tryItAgain.Num(); i++) {
			sessionQueue.Enqueue(tryItAgain[i]);
		}
		tryItAgain.Empty();

		
		FPlatformProcess::Sleep(minLiveTimeInSeconds);
	}
	return 0;
};