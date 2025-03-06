// Copyright 2022 David Romanski (Socke). All Rights Reserved.

#include "SocketServerTCPClientReceiveDataThread.h"


FSocketServerTCPClientReceiveDataThread::FSocketServerTCPClientReceiveDataThread(USocketServerTCP* tcpServerP,
	FSocketServerPluginSession& sessionP,
	EReceiveFilterServer receiveFilterP) :
	tcpServer(tcpServerP),
	session(sessionP),
	receiveFilter(receiveFilterP){
	FString threadName = "FTCPClientReceiveDataFromServerThread" + FGuid::NewGuid().ToString();
	thread = FRunnableThread::Create(this, *threadName, 0, EThreadPriority::TPri_BelowNormal);
}

FSocketServerTCPClientReceiveDataThread::~FSocketServerTCPClientReceiveDataThread() {
	delete thread;
	//thread = nullptr;
}

uint32 FSocketServerTCPClientReceiveDataThread::Run() {

	//FPlatformProcess::Sleep(0.1);
	//tcpServer->removeClientSession(session.sessionID);

	FString serverID = tcpServer->getServerID();
	FSocket* clientSocket = session.socket;
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


	//switch to gamethread
	AsyncTask(ENamedThreads::GameThread, [sessionID, serverID]() {
		USocketServerBPLibrary::socketServerBPLibrary->onsocketServerConnectionEventDelegate.Broadcast(EServerSocketConnectionEventType::E_Client, true, "Client connected", sessionID, serverID);
		});

	uint32 DataSize = 0;
	//FArrayReaderPtr Datagram = MakeShareable(new FArrayReader(true));
	TArray<uint8> dataFromSocket;
	int64 ticks1;
	int64 ticks2;
	TArray<uint8> byteDataArray;
	TArray<uint8> byteDataArrayCache;
	bool hasData = false;
	int32 lastDataLengthFromHeader = 0;

	while (run && clientSocket != nullptr && tcpServer->isRun()) {
		//ESocketConnectionState::SCS_Connected does not work https://issues.unrealengine.com/issue/UE-27542
		//Compare ticks is a workaround to get a disconnect. clientSocket->Wait() stop working after disconnect. (Another bug?)
		//If it doesn't wait any longer, ticks1 and ticks2 should be the same == disconnect.
		ticks1 = FDateTime::Now().GetTicks();
		clientSocket->Wait(ESocketWaitConditions::WaitForRead, FTimespan::FromSeconds(1));
		ticks2 = FDateTime::Now().GetTicks();


		hasData = clientSocket->HasPendingData(DataSize);;

	/*	if (tcpServer->isRun()) {
			hasData = clientSocket->HasPendingData(DataSize);
		}
		else {
			deathConnection = true;
		}*/

		if (!hasData && ticks1 == ticks2) {
			deathConnection = true;
			//UE_LOG(LogTemp, Display, TEXT("TCP End xxx: %s:%i"), *session.ip, session.port);
			break;
		}
		if (hasData) {

			dataFromSocket.SetNumUninitialized(DataSize);
			int32 BytesRead = 0;
			if (clientSocket->Recv(dataFromSocket.GetData(), dataFromSocket.Num(), BytesRead)) {

				switch (messageWrapping)
				{
				case ESocketServerTCPSeparator::E_None:
					triggerMessageEvent(dataFromSocket, sessionID, serverID);
					break;

				case ESocketServerTCPSeparator::E_ByteSeparator:

					for (int32 i = 0; i < dataFromSocket.Num(); i++) {
						byteDataArrayCache.Add(dataFromSocket[i]);
						if (dataFromSocket[i] == byteSeparator) {
							triggerMessageEvent(byteDataArrayCache, sessionID, serverID, false);
							byteDataArrayCache.Empty();
						}
					}

					break;

				case ESocketServerTCPSeparator::E_StringSeparator:
					for (int32 i = 0; i < dataFromSocket.Num(); i++) {
						;
						if ((TCHAR)dataFromSocket[i] == stringSeparatorArray[0]) {
							bool found = true;
							for (int32 j = 1; j < stringSeparatorArray.Num(); j++) {
								i++;
								if (dataFromSocket.Num() <= (i)) {
									found = false;
									break;
								}
								if ((TCHAR)dataFromSocket[i] != stringSeparatorArray[j]) {
									found = false;
									break;
								}
							}
							if (found) {
								triggerMessageEvent(byteDataArrayCache, sessionID, serverID, true);
								byteDataArrayCache.Empty();
							}
						}
						else {
							byteDataArrayCache.Add(dataFromSocket[i]);
						}
					}
					break;

				case ESocketServerTCPSeparator::E_LengthSeparator:
					if (lastDataLengthFromHeader == 0 && dataFromSocket.Num() >= 5) {

						readDataLength(dataFromSocket, lastDataLengthFromHeader);

						if (dataFromSocket.Num() == 5) {
							dataFromSocket.Empty();
							continue;
						}

						byteDataArrayCache.Append(dataFromSocket.GetData() + 5, dataFromSocket.Num() - 5);
						dataFromSocket.Empty();
					}
					else {
						byteDataArrayCache.Append(dataFromSocket.GetData(), dataFromSocket.Num());
					}

					int32 maxLoops = 1000;//to prevent endless loop

					while (byteDataArrayCache.Num() > 0 && byteDataArrayCache.Num() >= lastDataLengthFromHeader && maxLoops > 0) {
						maxLoops--;

						byteDataArray.Append(byteDataArrayCache.GetData(), lastDataLengthFromHeader);
						byteDataArrayCache.RemoveAt(0, lastDataLengthFromHeader, true);


						triggerMessageEvent(byteDataArray, sessionID, serverID);
						//UE_LOG(LogTemp, Display, TEXT("%s"), *mainMessage);
						byteDataArray.Empty();

						if (byteDataArrayCache.Num() == 0) {
							lastDataLengthFromHeader = 0;
							break;
						}

						if (byteDataArrayCache.Num() > 5) {
							readDataLength(byteDataArrayCache, lastDataLengthFromHeader);
							byteDataArrayCache.RemoveAt(0, 5, true);
						}

					}
					break;
				}
			}
			dataFromSocket.Empty();
		}
	}

	if (!deathConnection && clientSocket != nullptr) {
		//UE_LOG(LogTemp, Display, TEXT("TCP Close"));
		clientSocket->Close();
	}
	//else {
	//	if (deathConnection) {
	//		UE_LOG(LogTemp, Display, TEXT("TCP2 Close true"));
	//	}
	//	else {
	//		UE_LOG(LogTemp, Display, TEXT("TCP2 Close false"));
	//	}
	//}

	//UE_LOG(LogTemp, Display, TEXT("TCP Connected: %s:%i"), *session.ip, session.port);

	//switch to gamethread
	USocketServerTCP* tcpServerGlobal = tcpServer;
	AsyncTask(ENamedThreads::GameThread, [sessionID, serverID, tcpServerGlobal]() {
		USocketServerBPLibrary::socketServerBPLibrary->onsocketServerConnectionEventDelegate.Broadcast(EServerSocketConnectionEventType::E_Client, false, "Client disconnected", sessionID, serverID);
		//clean up socket in main thread because race condition
		tcpServerGlobal->removeClientSession(sessionID);
		});

	dataFromSocket.Empty();
	byteDataArray.Empty();
	byteDataArrayCache.Empty();

	return 0;
}

void FSocketServerTCPClientReceiveDataThread::triggerMessageEvent(TArray<uint8>& byteDataArray, FString& sessionID, FString& serverID, bool addNullTerminator) {
	//if (receiveFilter == EReceiveFilterServer::E_SAB || receiveFilter == EReceiveFilterServer::E_B) {
	//	byteDataArray.Append(dataFromSocket.GetData(), dataFromSocket.Num());
	//}

	FString mainMessage = FString();
	if (receiveFilter == EReceiveFilterServer::E_SAB || receiveFilter == EReceiveFilterServer::E_S) {
		if (addNullTerminator)
			byteDataArray.Add(0x00);// null-terminator
		mainMessage = FString(UTF8_TO_TCHAR((char*)byteDataArray.GetData()));
		if (receiveFilter == EReceiveFilterServer::E_S) {
			byteDataArray.Empty();
		}
	}



	//switch to gamethread
	AsyncTask(ENamedThreads::GameThread, [mainMessage, sessionID, byteDataArray, serverID]() {
		//UE_LOG(LogTemp, Display, TEXT("TCP:%s"), *recvMessage);
		USocketServerBPLibrary::socketServerBPLibrary->onserverReceiveTCPMessageEventDelegate.Broadcast(sessionID, mainMessage, byteDataArray, serverID);
		if (USocketServerBPLibrary::socketServerBPLibrary->getResiteredClientEvent(sessionID) != nullptr) {
			USocketServerBPLibrary::socketServerBPLibrary->getResiteredClientEvent(sessionID)->onregisteredEventDelegate.Broadcast(mainMessage, byteDataArray);
		}
		});
	mainMessage.Empty();
}

void FSocketServerTCPClientReceiveDataThread::readDataLength(TArray<uint8>& byteDataArray, int32& byteLenght) {
	if (FGenericPlatformProperties::IsLittleEndian() && byteDataArray[0] == 0x00) {
		FMemory::Memcpy(&byteLenght, byteDataArray.GetData() + 1, 4);
	}
	else {
		//endian fits not. swap bytes that contains the length
		byteDataArray.SwapMemory(1, 4);
		byteDataArray.SwapMemory(2, 3);
		FMemory::Memcpy(&byteLenght, byteDataArray.GetData() + 1, 4);
	}
}

void FSocketServerTCPClientReceiveDataThread::stopThread() {
	run = false;
	Stop();
}