// Copyright 2022 David Romanski (Socke). All Rights Reserved.

#include "SocketServerTCPClientSendDataThread.h"


FSocketServerTCPClientSendDataThread::FSocketServerTCPClientSendDataThread(USocketServerTCP* tcpServerP, FSocketServerPluginSession& sessionP) :
	tcpServer(tcpServerP),
	session(sessionP) {
	FString threadName = "FTCPClientSendDataToServerThread_" + FGuid::NewGuid().ToString();
	thread = FRunnableThread::Create(this, *threadName, 0, EThreadPriority::TPri_BelowNormal);
}

FSocketServerTCPClientSendDataThread::~FSocketServerTCPClientSendDataThread() {
	delete thread;
}

uint32 FSocketServerTCPClientSendDataThread::Run() {
	FString serverID = tcpServer->getServerID();
	socket = session.socket;
	FString sessionID = session.sessionID;

	//message wrapping
	FString stringSeparator = FString();
	uint8 byteSeparator = 0x00;
	ESocketServerTCPSeparator messageWrapping = ESocketServerTCPSeparator::E_None;

	tcpServer->getTcpSeparator(stringSeparator, byteSeparator, messageWrapping);

	TArray<TCHAR> stringSeparatorArray = stringSeparator.GetCharArray();
	if (stringSeparatorArray.Num() > 0 && stringSeparatorArray.Last() == 0x00) {
		stringSeparatorArray.RemoveAt(stringSeparatorArray.Num() - 1, 1, true);
	}

	if (messageWrapping == ESocketServerTCPSeparator::E_LengthSeparator && stringSeparatorArray.Num() == 0) {
		messageWrapping = ESocketServerTCPSeparator::E_None;
		UE_LOG(LogTemp, Warning, TEXT("Socket Sever Plugin: Separator mode is set to String but there is no String Separator. Mode changed to none."));
	}

	while (run) {

		//Wait a bit in case someone tries to send something right after the connection is established to avoid hitting a dead connection.
		if (waitForInit) {
			waitForInit = false;
			FPlatformProcess::Sleep(0.5);
		}

		if (socket == nullptr || tcpServer->clientSessions.Find(sessionID) == nullptr) {
			//UE_LOG(LogTemp, Error, TEXT("Socket not found."));
			return 0;
		}


		// try to connect to the server
		if (socket == nullptr || run == false) {
			//UE_LOG(LogTemp, Error, TEXT("Connection not exist."));
			//switch to gamethread
			if (messageQueue.IsEmpty() == false || byteArrayQueue.IsEmpty() == false) {
				AsyncTask(ENamedThreads::GameThread, [sessionID, serverID]() {
					USocketServerBPLibrary::socketServerBPLibrary->onsocketServerConnectionEventDelegate.Broadcast(EServerSocketConnectionEventType::E_Client, false, "Connection not exist", sessionID, serverID);
					});
			}
			return 0;
		}


		int32 sent = 0;
		if (socket != nullptr) {



			while (messageQueue.IsEmpty() == false) {
				FString m;
				messageQueue.Dequeue(m);
				FTCHARToUTF8 Convert(*m);
				sent = 0;

				TArray<uint8> byteCache;

				switch (messageWrapping)
				{
				case ESocketServerTCPSeparator::E_None:
					byteCache.Append((uint8*)Convert.Get(), Convert.Length());
					break;
				case ESocketServerTCPSeparator::E_ByteSeparator:
					byteCache.Append((uint8*)Convert.Get(), Convert.Length());
					byteCache.Add(byteSeparator);
					break;
				case ESocketServerTCPSeparator::E_StringSeparator:
				{
					m += stringSeparator;
					FTCHARToUTF8 ConvertWithSeparator(*m);
					byteCache.Append((uint8*)ConvertWithSeparator.Get(), ConvertWithSeparator.Length());
				}
				break;
				case ESocketServerTCPSeparator::E_LengthSeparator:
					if (FGenericPlatformProperties::IsLittleEndian()) {
						byteCache.Add(0x00);
					}
					else {
						byteCache.Add(0x01);
					}
					byteCache.SetNum(5);
					int32 dataLength = Convert.Length();
					FMemory::Memcpy(byteCache.GetData() + 1, &dataLength, 4);
					byteCache.Append((uint8*)Convert.Get(), Convert.Length());
					break;
				}

				socket->Send(byteCache.GetData(), byteCache.Num(), sent);
			}

			while (byteArrayQueue.IsEmpty() == false) {

				TArray<uint8> byteCache;
				byteArrayQueue.Dequeue(byteCache);
				sent = 0;

				switch (messageWrapping)
				{
				case ESocketServerTCPSeparator::E_ByteSeparator:
					byteCache.Add(byteSeparator);
					break;
				case ESocketServerTCPSeparator::E_StringSeparator:
				{
					FTCHARToUTF8 ConvertWithSeparator(*stringSeparator);
					byteCache.Append((uint8*)ConvertWithSeparator.Get(), ConvertWithSeparator.Length());
				}
				break;
				case ESocketServerTCPSeparator::E_LengthSeparator:
					byteCache.InsertZeroed(0, 5);
					if (FGenericPlatformProperties::IsLittleEndian() == false) {
						uint8 a = 0x01;
						FMemory::Memcpy(byteCache.GetData(), &a, 1);
					}
					int32 dataLength = byteCache.Num() - 5;
					FMemory::Memcpy(byteCache.GetData() + 1, &dataLength, 4);
					break;
				}

				socket->Send(byteCache.GetData(), byteCache.Num(), sent);

			}


		}
		else {
			//UE_LOG(LogTemp, Error, TEXT("Connection lost"));
			AsyncTask(ENamedThreads::GameThread, [sessionID, serverID]() {
				USocketServerBPLibrary::socketServerBPLibrary->onsocketServerConnectionEventDelegate.Broadcast(EServerSocketConnectionEventType::E_Client, false, "Connection lost", sessionID, serverID);
				});
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

FRunnableThread* FSocketServerTCPClientSendDataThread::getThread() {
	return thread;
}

void FSocketServerTCPClientSendDataThread::setThread(FRunnableThread* threadP) {
	thread = threadP;
}

void FSocketServerTCPClientSendDataThread::stopThread() {
	run = false;
	if (thread != nullptr) {
		pauseThread(false);
	}
	Stop();
}

bool FSocketServerTCPClientSendDataThread::isRun() {
	return run;
}

void FSocketServerTCPClientSendDataThread::setMessage(FString messageP, TArray<uint8> byteArrayP) {
	if (messageP.Len() > 0)
		messageQueue.Enqueue(messageP);
	if (byteArrayP.Num() > 0)
		byteArrayQueue.Enqueue(byteArrayP);
}

void FSocketServerTCPClientSendDataThread::sendMessage(FString messageP, TArray<uint8> byteArrayP) {
	if (messageP.Len() > 0)
		messageQueue.Enqueue(messageP);
	if (byteArrayP.Num() > 0)
		byteArrayQueue.Enqueue(byteArrayP);
	pauseThread(false);
}

void FSocketServerTCPClientSendDataThread::pauseThread(bool pause) {
	paused = pause;
	thread->Suspend(pause);
}