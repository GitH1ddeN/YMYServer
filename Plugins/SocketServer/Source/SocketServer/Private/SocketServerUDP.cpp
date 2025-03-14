// Copyright 2017-2020 David Romanski (Socke). All Rights Reserved.

#include "SocketServerUDP.h"


USocketServerUDP::USocketServerUDP(const FObjectInitializer& ObjectInitializer) : Super(ObjectInitializer) {
	this->AddToRoot();
}


void USocketServerUDP::startUDPServer(FIPandPortStruct ipStructP,FString IPP, int32 portP, bool multicastP,
	EReceiveFilterServer receiveFilterP, FString serverIDP, int32 maxPacketSizeP) {

	ipAndPortStruct = ipStructP;
	serverIP = IPP;
	serverPort = portP;
	receiveFilter = receiveFilterP;
	serverID = serverIDP;
	maxPacketSize = maxPacketSizeP;
	if (maxPacketSize < 1 || maxPacketSize > 65507)
		maxPacketSize = 65507;
	serverThread = new FSocketServerUDPThread(this, multicastP);
	sendThread = new  FSocketServerUDPClientSendDataThread(this);

}

void USocketServerUDP::stopUDPServer() {

	TArray<FString> toRemoveSessionKeys;
	for (auto& element : getClientSessions()) {
		toRemoveSessionKeys.Add(element.Key);
	}
	for (int32 i = 0; i < toRemoveSessionKeys.Num(); i++) {
		removeClientSession(toRemoveSessionKeys[i]);
	}
	toRemoveSessionKeys.Empty();

	FSocketServerPluginSession session;

	if (sendThread != nullptr) {
		sendThread->stopThread();
		session.udpSendThread = sendThread;
	}

	if (serverThread != nullptr) {
		serverThread->stopThread();
		session.udpServerThread = serverThread;
	}

	if (socketReceiver != nullptr) {
		socketReceiver->Stop();
		socketReceiver->Exit();
		session.udpSocketReceiver = socketReceiver;
	}

	if (socket && socket != nullptr) {
		socket->Close();
	}
	USocketServerBPLibrary::socketServerBPLibrary->cleanConnection(session);
}

void USocketServerUDP::sendUDPMessage(TArray<FString> clientSessionIDs, FString message, TArray<uint8> byteArray, bool asynchronous, ESocketServerUDPSocketType socketType) {
	for (auto& sessionID : clientSessionIDs) {
		FSocketServerPluginSession* sessionPointer = clientSessions.Find(sessionID);
		if (sessionPointer != nullptr) {
			FSocketServerPluginSession& session = *sessionPointer;
			if (session.protocol == EServerSocketConnectionProtocol::E_UDP) {


				FSocket* socketUDP = socket;
				if (socketUDP == nullptr || socketType == ESocketServerUDPSocketType::E_SSS_CLIENT)
					socketUDP = session.socket;

				if (asynchronous) {
					sendThread->sendMessage(session.ip, session.port, message, byteArray, socketUDP);
					return;
				}

				int32 sent = 0;

				TSharedRef<FInternetAddr> addr = USocketServerBPLibrary::getSocketSubSystem()->CreateInternetAddr();
				bool bIsValid;
				addr->SetIp(*session.ip, bIsValid);
				addr->SetPort(session.port);
				if (bIsValid) {
					if (byteArray.Num() > 0) {
						sendBytes(socketUDP, byteArray, sent, addr);
					}

					if (message.Len() > 0) {
						FTCHARToUTF8 Convert(*message);
						byteArray.Append((uint8*)Convert.Get(), Convert.Length());
						sendBytes(socketUDP, byteArray, sent, addr);
					}
				}
				else {
					UE_LOG(LogTemp, Warning, TEXT("SimpleSocketServer Plugin: Can't send data. Wrong adress."));
				}

			}
		}
		else {
			FString serverIDGlobal = serverID;
			AsyncTask(ENamedThreads::GameThread, [sessionID, serverIDGlobal]() {
				USocketServerBPLibrary::socketServerBPLibrary->onsocketServerUDPConnectionEventDelegate.Broadcast(false, "Can't send message. SessionID not found on this server: " + sessionID, serverIDGlobal);
			});
		}
	}
}


void USocketServerUDP::sendUDPMessageToClient(FString clientSessionID, FString message, TArray<uint8> byteArray, bool asynchronous, ESocketServerUDPSocketType socketType) {

	FSocketServerPluginSession* sessionPointer = clientSessions.Find(clientSessionID);
	if (sessionPointer != nullptr) {
		FSocketServerPluginSession& session = *sessionPointer;
		if (session.protocol == EServerSocketConnectionProtocol::E_UDP) {
			
			FSocket* socketUDP = socket;
			if (socketUDP == nullptr || socketType == ESocketServerUDPSocketType::E_SSS_CLIENT)
				socketUDP = session.socket;

			if (asynchronous) {
				sendThread->sendMessage(session.ip, session.port, message, byteArray, socketUDP);
				return;
			}

			int32 sent = 0;

			TSharedRef<FInternetAddr> addr = USocketServerBPLibrary::getSocketSubSystem()->CreateInternetAddr();
			bool bIsValid;
			addr->SetIp(*session.ip, bIsValid);
			addr->SetPort(session.port);
			if (bIsValid) {
				if (byteArray.Num() > 0) {
					sendBytes(socketUDP, byteArray, sent, addr);
				}

				if (message.Len() > 0) {
					FTCHARToUTF8 Convert(*message);
					byteArray.Append((uint8*)Convert.Get(), Convert.Length());
					sendBytes(socketUDP, byteArray, sent, addr);
				}

				
			}
			else {
				UE_LOG(LogTemp, Warning, TEXT("SimpleSocketServer Plugin: Can't send data. Wrong adress."));
			}

		}
	}
	else {
		FString serverIDGlobal = serverID;
		AsyncTask(ENamedThreads::GameThread, [clientSessionID, serverIDGlobal]() {
			USocketServerBPLibrary::socketServerBPLibrary->onsocketServerUDPConnectionEventDelegate.Broadcast(false, "Can't send message. SessionID not found on this server: " + clientSessionID, serverIDGlobal);
		});
	}

}


void USocketServerUDP::sendUDPMessageTo(FString ip, int32 port, FString message, TArray<uint8> byteArray, bool asynchronous) {
	int32 sent = 0;

	FSocket* socketUDP = socket;
	if (socketUDP == nullptr) {
		UE_LOG(LogTemp, Warning, TEXT("SimpleSocketServer Plugin: Can't send data. Connection broken. Restart your UDP server %s"),*serverID);
		return;
	}

	if (asynchronous) {
		sendThread->sendMessage(ip,port,message, byteArray,socketUDP);
		return;
	}

	TSharedRef<FInternetAddr> addr = USocketServerBPLibrary::getSocketSubSystem()->CreateInternetAddr();
	bool bIsValid;
	addr->SetIp(*ip, bIsValid);
	addr->SetPort(port);
	if (bIsValid) {

		if (byteArray.Num() > 0) {
			sendBytes(socketUDP, byteArray, sent, addr);
		}

		if (message.Len() > 0) {
			FTCHARToUTF8 Convert(*message);
			byteArray.Append((uint8*)Convert.Get(), Convert.Length());
			sendBytes(socketUDP, byteArray, sent, addr);
		}
		
	}
	else {
		UE_LOG(LogTemp, Warning, TEXT("SimpleSocketServer Plugin: Can't send data. Wrong adress."));
	}
}



void USocketServerUDP::UDPReceiverSocketServerPlugin(FArrayReaderPtr& ArrayReaderPtr, TSharedRef<FInternetAddr> remoteAddress) {

	FString sessionID = remoteAddress.Get().ToString(true);
	FSocketServerPluginSession* sessionPointer = clientSessions.Find(sessionID);
	if (sessionPointer == nullptr) {

		//FString socketName;
		//FSocket* receiverSocket = FUdpSocketBuilder(*socketName);
		ISocketSubsystem* socketSubsystem = USocketServerBPLibrary::getSocketSubSystem();
		FSocket* receiverSocket = socketSubsystem->CreateSocket(NAME_DGram, *sessionID, remoteAddress->GetProtocolType());


		//create and save session
		FSocketServerPluginSession session;
		session.sessionID = sessionID;
		session.serverID = serverID;
		session.ip = remoteAddress.Get().ToString(false);
		session.port = remoteAddress.Get().GetPort();
		session.socket = receiverSocket;
		session.protocol = EServerSocketConnectionProtocol::E_UDP;
		addClientSession(session);
	}
	TArray<uint8> byteArray;
	if (receiveFilter == EReceiveFilterServer::E_SAB || receiveFilter == EReceiveFilterServer::E_B) {
		byteArray.Append(ArrayReaderPtr->GetData(), ArrayReaderPtr->Num());
	}

	FString recvMessage;
	if (receiveFilter == EReceiveFilterServer::E_SAB || receiveFilter == EReceiveFilterServer::E_S) {
		ArrayReaderPtr->Add(0x00);// null-terminator
		char* Data = (char*)ArrayReaderPtr->GetData();
		recvMessage = FString(UTF8_TO_TCHAR(Data));
	}

	FString serverIDGlobal = serverID;
	//switch to gamethread
	AsyncTask(ENamedThreads::GameThread, [recvMessage, sessionID, byteArray, serverIDGlobal]() {
		USocketServerBPLibrary::socketServerBPLibrary->onserverReceiveUDPMessageEventDelegate.Broadcast(sessionID, recvMessage, byteArray, serverIDGlobal);
		if (USocketServerBPLibrary::socketServerBPLibrary->getResiteredClientEvent(sessionID) != nullptr) {
			USocketServerBPLibrary::socketServerBPLibrary->getResiteredClientEvent(sessionID)->onregisteredEventDelegate.Broadcast(recvMessage, byteArray);
		}
		});
	byteArray.Empty();
	recvMessage.Empty();
}

//do not work with ipv6
//void USocketServerPluginUDPServer::UDPReceiver(const FArrayReaderPtr& ArrayReaderPtr, const FIPv4Endpoint& EndPt) {
//
//	FString sessionID = EndPt.ToString();
//	FClientSocketSession* sessionPointer = clientSessions.Find(sessionID);
//	if (sessionPointer == nullptr) {
//		TSharedRef<FInternetAddr> remoteAddress = EndPt.ToInternetAddr();
//
//		FString socketName;
//		FSocket* receiverSocket = FUdpSocketBuilder(*socketName);
//
//		//create and save session
//		FClientSocketSession session;
//		session.sessionID = sessionID;
//		session.serverID = serverID;
//		session.ip = remoteAddress.Get().ToString(false);
//		session.port = remoteAddress.Get().GetPort();
//		session.socket = receiverSocket;
//		session.protocol = EServerSocketConnectionProtocol::E_UDP;
//		addClientSession(session);
//	}
//	TArray<uint8> byteArray;
//	if (receiveFilter == EReceiveFilterServer::E_SAB || receiveFilter == EReceiveFilterServer::E_B) {
//		byteArray.Append(ArrayReaderPtr->GetData(), ArrayReaderPtr->Num());
//	}
//	
//	FString recvMessage;
//	if (receiveFilter == EReceiveFilterServer::E_SAB || receiveFilter == EReceiveFilterServer::E_S) {
//		ArrayReaderPtr->Add(0x00);// null-terminator
//		char* Data = (char*)ArrayReaderPtr->GetData();
//		recvMessage = FString(UTF8_TO_TCHAR(Data));
//	}
//
//	FString serverIDGlobal = serverID;
//	//switch to gamethread
//	AsyncTask(ENamedThreads::GameThread, [recvMessage, sessionID, byteArray, serverIDGlobal]() {
//		USocketServerBPLibrary::socketServerBPLibrary->onserverReceiveUDPMessageEventDelegate.Broadcast(sessionID, recvMessage, byteArray, serverIDGlobal);
//		if (USocketServerBPLibrary::socketServerBPLibrary->getResiteredClientEvent(sessionID) != nullptr) {
//			USocketServerBPLibrary::socketServerBPLibrary->getResiteredClientEvent(sessionID)->onregisteredEventDelegate.Broadcast(recvMessage, byteArray);
//		}
//	});
//	byteArray.Empty();
//	recvMessage.Empty();
//}


FIPandPortStruct USocketServerUDP::getServerIpAndPortStruct() {
	return ipAndPortStruct;
}

FString USocketServerUDP::getIP(){
	return serverIP;
}

int32 USocketServerUDP::getPort() {
	return serverPort;
}

void USocketServerUDP::setSocketReceiver(FUdpSocketReceiver* socketReceiverP, FSocket* socketP) {
	socketReceiver = socketReceiverP;
	socket = socketP;
}

FUdpSocketReceiver* USocketServerUDP::getSocketReceiver() {
	return socketReceiver;
}

FSocket* USocketServerUDP::getSocket() {
	return socket;
}

FString USocketServerUDP::getServerID(){
	return serverID;
}

void USocketServerUDP::addClientSession(FSocketServerPluginSession& session){
	if (session.sessionID.IsEmpty() == false) {
		//UE_LOG(LogTemp, Warning, TEXT("ADD Session:%s"), *session.sessionID);
		clientSessions.Add(session.sessionID, session);
	}
}

FSocketServerPluginSession* USocketServerUDP::getClientSession(FString key){
	return clientSessions.Find(key);
}

void USocketServerUDP::removeClientSession(FString key){
	if (clientSessions.Remove(key)) {
		//UE_LOG(LogTemp, Warning, TEXT("Remove Session:%s"), *key);
		USocketServerBPLibrary::socketServerBPLibrary->unregisterClientEvent(key);
	}
}

TMap<FString, FSocketServerPluginSession> USocketServerUDP::getClientSessions(){
	return clientSessions;
}

void USocketServerUDP::sendBytes(FSocket*& socketP, TArray<uint8>& byteArray, int32& sent, TSharedRef<FInternetAddr>& addr){
	if (byteArray.Num() > maxPacketSize) {
		TArray<uint8> byteArrayTemp;
		for (int32 i = 0; i < byteArray.Num(); i++) {
			byteArrayTemp.Add(byteArray[i]);
			if (byteArrayTemp.Num() == maxPacketSize) {
				sent = 0;
				socketP->SendTo(byteArrayTemp.GetData(), byteArrayTemp.Num(), sent, *addr);
				byteArrayTemp.Empty();
			}
		}
		if (byteArrayTemp.Num() > 0) {
			sent = 0;
			socketP->SendTo(byteArrayTemp.GetData(), byteArrayTemp.Num(), sent, *addr);
			byteArrayTemp.Empty();
		}
	}
	else {
		sent = 0;
		socketP->SendTo(byteArray.GetData(), byteArray.Num(), sent, *addr);
	}

	byteArray.Empty();
}