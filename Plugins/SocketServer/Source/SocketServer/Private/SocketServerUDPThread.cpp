// Copyright 2022 David Romanski (Socke). All Rights Reserved.

#include "SocketServerUDPThread.h"

FSocketServerUDPThread::FSocketServerUDPThread(USocketServerUDP* udpServerP, bool multicastP) :
	udpServer(udpServerP),
	multicast(multicastP) {
	FString threadName = "FServerUDPThread_" + FGuid::NewGuid().ToString();
	thread = FRunnableThread::Create(this, *threadName, 0, EThreadPriority::TPri_BelowNormal);
}

FSocketServerUDPThread::~FSocketServerUDPThread() {
	delete thread;
}

uint32 FSocketServerUDPThread::Run() {

	FIPandPortStruct ipAndPortStruct = udpServer->getServerIpAndPortStruct();
	FSocket* listenerSocket = udpServer->getSocket();
	FUdpSocketReceiver* udpSocketReceiver = nullptr;


	FString ip = ipAndPortStruct.ip;
	FString adress = ip + ":" + FString::FromInt(udpServer->getPort());
	FString serverID = udpServer->getServerID();

	// create the socket
	FString socketName;
	ISocketSubsystem* socketSubsystem = USocketServerBPLibrary::getSocketSubSystem();


	if (multicast) {
		TSharedPtr<class FInternetAddr> addr = socketSubsystem->CreateInternetAddr();
		addr->SetAnyAddress();
		addr->SetPort(udpServer->getPort());


		listenerSocket = socketSubsystem->CreateSocket(NAME_DGram, *socketName, addr->GetProtocolType());
		if (listenerSocket == nullptr || listenerSocket == NULL) {
			const TCHAR* SocketErr = socketSubsystem->GetSocketError(SE_GET_LAST_ERROR_CODE);
			AsyncTask(ENamedThreads::GameThread, [adress, SocketErr, serverID]() {
				USocketServerBPLibrary::socketServerBPLibrary->onsocketServerUDPConnectionEventDelegate.Broadcast(true, "Unable to open UDP Server: " + adress + " | " + SocketErr, serverID);
				});
			thread = nullptr;
			return 0;
		}


		if (!listenerSocket->Bind(*addr)) {
			UE_LOG(LogTemp, Error, TEXT("Unable to open UDP Server"));
			const TCHAR* SocketErr = socketSubsystem->GetSocketError(SE_GET_LAST_ERROR_CODE);
			AsyncTask(ENamedThreads::GameThread, [adress, SocketErr, serverID]() {
				USocketServerBPLibrary::socketServerBPLibrary->onsocketServerUDPConnectionEventDelegate.Broadcast(true, "Unable to open UDP Server: " + adress + " | " + SocketErr, serverID);
				});
			thread = nullptr;
			return 0;
		}

		if (!listenerSocket->SetBroadcast(true)) {
			UE_LOG(LogTemp, Error, TEXT("Unable to set Broadcast"));
			const TCHAR* SocketErr = socketSubsystem->GetSocketError(SE_GET_LAST_ERROR_CODE);
			AsyncTask(ENamedThreads::GameThread, [adress, SocketErr, serverID]() {
				USocketServerBPLibrary::socketServerBPLibrary->onsocketServerUDPConnectionEventDelegate.Broadcast(true, "Unable to set Broadcast: " + adress + " | " + SocketErr, serverID);
				});
			thread = nullptr;
			return 0;
		}

		if (!listenerSocket->SetMulticastLoopback(true)) {
			UE_LOG(LogTemp, Error, TEXT("Unable to set Multicast Loopback"));
			const TCHAR* SocketErr = socketSubsystem->GetSocketError(SE_GET_LAST_ERROR_CODE);
			AsyncTask(ENamedThreads::GameThread, [adress, SocketErr, serverID]() {
				USocketServerBPLibrary::socketServerBPLibrary->onsocketServerUDPConnectionEventDelegate.Broadcast(true, "Unable to set Multicast Loopback: " + adress + " | " + SocketErr, serverID);
				});
			thread = nullptr;
			return 0;
		}
		bool validIP = true;
		addr->SetIp(*ip, validIP);

		if (!validIP) {
			UE_LOG(LogTemp, Error, TEXT("SocketServer UDP. Can't set ip"));
			AsyncTask(ENamedThreads::GameThread, [adress, serverID]() {
				USocketServerBPLibrary::socketServerBPLibrary->onsocketServerUDPConnectionEventDelegate.Broadcast(true, "Unable to open UDP Server: " + adress + " | Can't set ip.", serverID);
				});
			thread = nullptr;
			return 0;
		}


		if (!listenerSocket->JoinMulticastGroup(*addr)) {
			UE_LOG(LogTemp, Error, TEXT("Unable to join Multicast Group"));
			const TCHAR* SocketErr = socketSubsystem->GetSocketError(SE_GET_LAST_ERROR_CODE);
			AsyncTask(ENamedThreads::GameThread, [adress, SocketErr, serverID]() {
				USocketServerBPLibrary::socketServerBPLibrary->onsocketServerUDPConnectionEventDelegate.Broadcast(true, "Unable to join Multicast Group: " + adress + " | " + SocketErr, serverID);
				});
			thread = nullptr;
			return 0;
		}
	}
	else {

		TSharedPtr<class FInternetAddr> addr = socketSubsystem->CreateInternetAddr();
		bool validIP = true;
		addr->SetPort(udpServer->getPort());
		addr->SetIp(*ip, validIP);


		listenerSocket = socketSubsystem->CreateSocket(NAME_DGram, *socketName, addr->GetProtocolType());
		if (listenerSocket == nullptr || listenerSocket == NULL) {
			const TCHAR* SocketErr = socketSubsystem->GetSocketError(SE_GET_LAST_ERROR_CODE);
			AsyncTask(ENamedThreads::GameThread, [adress, SocketErr, serverID]() {
				USocketServerBPLibrary::socketServerBPLibrary->onsocketServerUDPConnectionEventDelegate.Broadcast(true, "Unable to open UDP Server: " + adress + " | " + SocketErr, serverID);
				});
			thread = nullptr;
			return 0;
		}

		if (!validIP) {
			UE_LOG(LogTemp, Error, TEXT("SocketServer UDP. Can't set ip"));
			AsyncTask(ENamedThreads::GameThread, [adress, serverID]() {
				USocketServerBPLibrary::socketServerBPLibrary->onsocketServerUDPConnectionEventDelegate.Broadcast(true, "Unable to open UDP Server: " + adress + " | Can't set ip.", serverID);
				});
			thread = nullptr;
			return 0;
		}

		listenerSocket->SetReuseAddr();
		listenerSocket->SetNonBlocking();
		if (!listenerSocket->Bind(*addr)) {
			UE_LOG(LogTemp, Error, TEXT("Unable to open UDP Server"));
			const TCHAR* SocketErr = socketSubsystem->GetSocketError(SE_GET_LAST_ERROR_CODE);
			AsyncTask(ENamedThreads::GameThread, [adress, SocketErr, serverID]() {
				USocketServerBPLibrary::socketServerBPLibrary->onsocketServerUDPConnectionEventDelegate.Broadcast(true, "Unable to open UDP Server: " + adress + " | " + SocketErr, serverID);
				});
			thread = nullptr;
			return 0;
		}
	}

	//do not work with ipv6
	FTimespan ThreadWaitTime = FTimespan::FromMilliseconds(100);
	//FString threadName = "SocketServerBPLibUDPReceiverThread_" + FString::FromInt(FDateTime::Now().GetTicks());

	//udpSocketReceiver = new FUdpSocketReceiver(listenerSocket, ThreadWaitTime, *threadName);
	//udpSocketReceiver->OnDataReceived().BindUObject(udpServer, &USocketServerPluginUDPServer::UDPReceiver);
	//udpSocketReceiver->Start();
	udpServer->setSocketReceiver(udpSocketReceiver, listenerSocket);

	//udpServer->initUDPClientThreads(listenerSocket);

	AsyncTask(ENamedThreads::GameThread, [adress, serverID]() {
		USocketServerBPLibrary::socketServerBPLibrary->onsocketServerUDPConnectionEventDelegate.Broadcast(true, "UDP Server started: " + adress, serverID);
		});


	//copy of FUdpSocketReceiver.h to get IPv6 working
	while (run) {
		if (!listenerSocket->Wait(ESocketWaitConditions::WaitForRead, ThreadWaitTime)) {
			continue;
		}

		TSharedRef<FInternetAddr> Sender = socketSubsystem->CreateInternetAddr();
		uint32 Size;

		while (listenerSocket->HasPendingData(Size)) {
			FArrayReaderPtr Reader = MakeShared<FArrayReader, ESPMode::ThreadSafe>(true);
			Reader->SetNumUninitialized(FMath::Min(Size, 65507u));

			int32 Read = 0;

			if (listenerSocket->RecvFrom(Reader->GetData(), Reader->Num(), Read, *Sender))
			{
				Reader->RemoveAt(Read, Reader->Num() - Read, false);
				udpServer->UDPReceiverSocketServerPlugin(Reader, Sender);
				//	UE_LOG(LogTemp, Error, TEXT("%s_%s"), *Sender.Get().ToString(true), *Sender->GetProtocolType().ToString());

			}
		}
	}

	if (listenerSocket != nullptr) {
		listenerSocket->Close();
		socketSubsystem->DestroySocket(listenerSocket);
		listenerSocket = nullptr;
	}

	return 0;
}

void FSocketServerUDPThread::stopThread() {
	run = false;
}