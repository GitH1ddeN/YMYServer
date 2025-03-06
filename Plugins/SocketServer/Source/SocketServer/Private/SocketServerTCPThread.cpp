// Copyright 2022 David Romanski (Socke). All Rights Reserved.

#include "SocketServerTCPThread.h"


FSocketServerTCPThread::FSocketServerTCPThread(USocketServerTCP* tcpServerP, EReceiveFilterServer receiveFilterP, bool& runP) :
	tcpServer(tcpServerP),
	receiveFilter(receiveFilterP),
	run(runP) {
}

void FSocketServerTCPThread::DoWork() {
	FIPandPortStruct ipAndPortStruct = tcpServer->getServerIpAndPortStruct();
	int32 port = tcpServer->getPort();
	FString serverID = tcpServer->getServerID();

	bool createServer = true;
	FSocket* listenerSocket = nullptr;
	ISocketSubsystem* socketSubSystem = USocketServerBPLibrary::getSocketSubSystem();
	if (socketSubSystem != nullptr) {

		TSharedRef<FInternetAddr> internetAddr = socketSubSystem->CreateInternetAddr();
		internetAddr->SetIp(*ipAndPortStruct.ip, ipAndPortStruct.success);
		internetAddr->SetPort(ipAndPortStruct.port);

		if (ipAndPortStruct.success) {
			listenerSocket = socketSubSystem->CreateSocket(NAME_Stream, *FString("USocketServerBPLibraryListenerSocket"), internetAddr->GetProtocolType());
		}

		if (listenerSocket != nullptr) {
			listenerSocket->SetLinger(false, 0);

			if (!listenerSocket->Bind(*internetAddr)) {
				if (listenerSocket != nullptr) {
					listenerSocket->Close();
					if (socketSubSystem != nullptr)
						socketSubSystem->DestroySocket(listenerSocket);
					listenerSocket = nullptr;
					UE_LOG(LogTemp, Error, TEXT("(211) TCP Server not started. Can't bind %s:%i. Please check IP,Port or your firewall."), *ipAndPortStruct.ip, port);
				}
				createServer = false;
			}


			if (createServer && !listenerSocket->Listen(8)) {
				if (listenerSocket != nullptr) {
					listenerSocket->Close();
					if (socketSubSystem != nullptr)
						socketSubSystem->DestroySocket(listenerSocket);
					listenerSocket = nullptr;
					UE_LOG(LogTemp, Error, TEXT("(212) TCP Server not started. Can't listen on %s:%i. Please check IP,Port or your firewall."), *ipAndPortStruct.ip, port);
				}
				createServer = false;
			}
		}

	}



	if (!createServer || listenerSocket == nullptr) {
		UE_LOG(LogTemp, Error, TEXT("(210) TCP Server not started on %s:%i. Please check IP,Port or your firewall."), *ipAndPortStruct.ip, port);
		AsyncTask(ENamedThreads::GameThread, [serverID]() {
			USocketServerBPLibrary::socketServerBPLibrary->onsocketServerConnectionEventDelegate.Broadcast(EServerSocketConnectionEventType::E_Server, false, "TCP Server not started. Please check IP,Port or your firewall.", TEXT(""), serverID);
			USocketServerBPLibrary::getSocketServerTarget()->stopTCPServer(serverID);
			});
	}
	else {
		//switch to gamethread
		AsyncTask(ENamedThreads::GameThread, [serverID]() {
			USocketServerBPLibrary::socketServerBPLibrary->onsocketServerConnectionEventDelegate.Broadcast(EServerSocketConnectionEventType::E_Server, true, "TCP Server started.", TEXT(""), serverID);
			});

		while (run) {
			bool pending;
			listenerSocket->WaitForPendingConnection(pending, FTimespan::FromSeconds(1));
			if (pending) {
				//UE_LOG(LogTemp, Display, TEXT("TCP Client: Pending connection"));

				FSocketServerPluginSession session = FSocketServerPluginSession();
				session.sessionID = FGuid::NewGuid().ToString();
				session.serverID = serverID;

				TSharedPtr<FInternetAddr> remoteAddress = USocketServerBPLibrary::getSocketSubSystem()->CreateInternetAddr();
				session.socket = listenerSocket->Accept(*remoteAddress, session.sessionID);

				/*FPlatformProcess::Sleep(0.1);
				session.socket->Close();
				USocketServerBPLibrary::getSocketSubSystem()->DestroySocket(session.socket);*/

				session.ip = remoteAddress->ToString(false);
				session.port = remoteAddress->GetPort();
				session.protocol = EServerSocketConnectionProtocol::E_TCP;
				tcpServer->initTCPClientThreads(session, receiveFilter);
				tcpServer->addClientSession(session);
			}
		}
	}

	////wait for client disconnects
	//FPlatformProcess::Sleep(2);

	if (listenerSocket && listenerSocket != nullptr) {
		listenerSocket->Close();
		if (socketSubSystem != nullptr)
			socketSubSystem->DestroySocket(listenerSocket);
	}

	listenerSocket = nullptr;

	//switch to gamethread
	AsyncTask(ENamedThreads::GameThread, [serverID]() {
		USocketServerBPLibrary::socketServerBPLibrary->onsocketServerConnectionEventDelegate.Broadcast(EServerSocketConnectionEventType::E_Server, false, "TCP Server stopped. Depending on the operating system it can take some time until the port is free again.", TEXT(""), serverID);
		});
	
}

void FSocketServerTCPThread::stopThread() {
	run = false;
}