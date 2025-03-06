// Copyright 2022 David Romanski (Socke). All Rights Reserved.

#include "SocketServerUDPClientSendDataThread.h"

FSocketServerUDPClientSendDataThread::FSocketServerUDPClientSendDataThread(USocketServerUDP* udpServerP) :
	udpServer(udpServerP) {
	FString threadName = "FUDPClientSendDataToServerThread_" + FGuid::NewGuid().ToString();
	thread = FRunnableThread::Create(this, *threadName, 0, EThreadPriority::TPri_BelowNormal);
}

FSocketServerUDPClientSendDataThread::~FSocketServerUDPClientSendDataThread() {
	delete thread;
}


uint32 FSocketServerUDPClientSendDataThread::Run() {
	//FString serverID = tcpServer->getServerID();
	//socket = session.socket;
	//FString sessionID = session.sessionID;

	while (run && thread == nullptr) {
		FPlatformProcess::Sleep(0.1);
	}

	TSharedRef<FInternetAddr> addr = USocketServerBPLibrary::getSocketSubSystem()->CreateInternetAddr();
	int32 sent = 0;
	bool bIsValid = false;

	while (run) {

		while (messageQueue.IsEmpty() == false) {
			FSendUDPMessageStruct messageStuct;
			messageQueue.Dequeue(messageStuct);


			addr->SetIp(*messageStuct.ip, bIsValid);
			addr->SetPort(messageStuct.port);

			if (bIsValid && messageStuct.socketUDP != nullptr) {

				if (messageStuct.bytes.Num() > 0) {
					udpServer->sendBytes(messageStuct.socketUDP, messageStuct.bytes, sent, addr);
				}

				if (messageStuct.message.Len() > 0) {
					FTCHARToUTF8 Convert(*messageStuct.message);
					messageStuct.bytes.Append((uint8*)Convert.Get(), Convert.Length());
					udpServer->sendBytes(messageStuct.socketUDP, messageStuct.bytes, sent, addr);
				}

			}
			else {
				UE_LOG(LogTemp, Warning, TEXT("SimpleSocketServer Plugin: Can't send data. Wrong adress."));
			}

		}


		if (run) {
			pauseThread(true);
			//workaround. suspend do not work on all platforms. lets sleep
			while (paused && run) {
				FPlatformProcess::Sleep(0.01);
			}
		}
	}

	run = false;

	return 0;
}

FRunnableThread* FSocketServerUDPClientSendDataThread::getThread() {
	return thread;
}

void FSocketServerUDPClientSendDataThread::setThread(FRunnableThread* threadP) {
	thread = threadP;
}

void FSocketServerUDPClientSendDataThread::stopThread() {
	run = false;
	if (thread != nullptr) {
		pauseThread(false);
	}
}

bool FSocketServerUDPClientSendDataThread::isRun() {
	return run;
}


void FSocketServerUDPClientSendDataThread::sendMessage(FString ip, int32	port, FString message, TArray<uint8>	bytes, FSocket* socketUDP) {
	FSendUDPMessageStruct messageStuct;
	messageStuct.ip = ip;
	messageStuct.port = port;
	messageStuct.message = message;
	messageStuct.bytes = bytes;
	messageStuct.socketUDP = socketUDP;
	messageQueue.Enqueue(messageStuct);
	pauseThread(false);
}


void FSocketServerUDPClientSendDataThread::pauseThread(bool pause) {
	paused = pause;
	thread->Suspend(pause);
}