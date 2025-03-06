// Copyright 2017-2020 David Romanski (Socke). All Rights Reserved.

#include "SocketServerBPLibrary.h"

USocketServerBPLibrary* USocketServerBPLibrary::socketServerBPLibrary;

USocketServerBPLibrary::USocketServerBPLibrary(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer) {
	socketServerBPLibrary = this;

	if (socketServerPluginCleanerThread == nullptr)
		socketServerPluginCleanerThread = new FSocketServerCleanerThread();

	//Delegates examples
	/*onsocketServerConnectionEventDelegate.AddDynamic(this, &USocketServerBPLibrary::socketServerConnectionEventDelegate);
	onserverReceiveTCPMessageEventDelegate.AddDynamic(this, &USocketServerBPLibrary::serverReceiveTCPMessageEventDelegate);
	onsocketServerUDPConnectionEventDelegate.AddDynamic(this, &USocketServerBPLibrary::socketServerUDPConnectionEventDelegate);
	onserverReceiveUDPMessageEventDelegate.AddDynamic(this, &USocketServerBPLibrary::serverReceiveUDPMessageEventDelegate);*/
}


/*Delegate functions*/
void USocketServerBPLibrary::socketServerConnectionEventDelegate(const EServerSocketConnectionEventType type, const bool success, const FString message, const FString sessionID, const FString serverID) {}
void USocketServerBPLibrary::serverReceiveTCPMessageEventDelegate(const FString sessionID, const FString message, const TArray<uint8>& byteArray, const FString serverID) {}
void USocketServerBPLibrary::fileTransferOverTCPProgressEventDelegate(const FString sessionID, const FString filePathP, const float percent, const float mbit, const int64 bytesTransferred, const int64 fileSize){}
void USocketServerBPLibrary::fileTransferOverTCPInfoEventDelegate(const FString message, const FString sessionID, const FString filePathP, const bool success){}
void USocketServerBPLibrary::socketServerUDPConnectionEventDelegate(const bool success, const FString message, const FString serverID) {}
void USocketServerBPLibrary::serverReceiveUDPMessageEventDelegate(const FString sessionID, const FString message, const TArray<uint8>& byteArray, const FString serverID) {}
void USocketServerBPLibrary::readBytesFromFileInPartsEventDelegate(const int64 fileSize, const int64 position, const bool end, const TArray<uint8>& byteArray) {}
void USocketServerBPLibrary::receiveRCONRequestEventDelegate(const FString sessionID, const FString serverID, const int32 requestID, const FString request) {}



USocketServerBPLibrary * USocketServerBPLibrary::getSocketServerTarget() {
	return socketServerBPLibrary;
}


//void USocketServerBPLibrary::breakDownloadFileInfo(FSocketServerDownloadFileInfo downloadFileInfo, float& size, float& megaBytesLeft, float& megaBytesReceived, float& percentDownload, float& megaBit, FString& fileName, FString& serverID){
//	size = downloadFileInfo.size;
//	megaBytesLeft = downloadFileInfo.megaBytesLeft;
//	megaBytesReceived = downloadFileInfo.megaBytesReceived;
//	percentDownload = downloadFileInfo.percentDownload;
//	megaBit = downloadFileInfo.megaBit;
//	fileName = downloadFileInfo.fileName;
//	serverID = downloadFileInfo.serverID;
//}
//
//void USocketServerBPLibrary::breakUploadFileInfo(FSocketServerUploadFileInfo fileUploadInfo, float& size, float& megaBytesLeft, float& megabytesTransferred, float& percentUpload, float& megaBit, FString& fileName, FString& serverID) {
//	size = fileUploadInfo.size;
//	megaBytesLeft = fileUploadInfo.megaBytesLeft;
//	megaBytesSend = fileUploadInfo.megaBytesSend;
//	percentUpload = fileUploadInfo.percentUpload;
//	megaBit = fileUploadInfo.megaBit;
//	fileName = fileUploadInfo.fileName;
//	serverID = fileUploadInfo.serverID;
//}

void USocketServerBPLibrary::serverPluginGetSocketSessionIds(const FString serverID, TArray<FString>& sessionIDs) {
	USocketServerBPLibrary::getSocketServerTarget()->serverPluginGetSocketSessionIdsNonStatic(serverID, sessionIDs);
}

void USocketServerBPLibrary::serverPluginGetSocketSessionIdsNonStatic(const FString optionalServerID, TArray<FString>& sessionIDs){
	if (optionalServerID.IsEmpty()) {
		if (udpServers.Find(lastUDPServerID) != nullptr) {
			(*udpServers.Find(lastUDPServerID))->getClientSessions().GetKeys(sessionIDs);
			return;
		}
	}
	else {
		if (udpServers.Find(optionalServerID) != nullptr) {
			(*udpServers.Find(optionalServerID))->getClientSessions().GetKeys(sessionIDs);
			return;
		}
	}

	if (optionalServerID.IsEmpty()) {
		if (tcpServers.Find(lastTCPServerID) != nullptr) {
			(*tcpServers.Find(lastTCPServerID))->getClientSessions().GetKeys(sessionIDs);
		}
	}
	else {
		if (tcpServers.Find(optionalServerID) != nullptr) {
			(*tcpServers.Find(optionalServerID))->getClientSessions().GetKeys(sessionIDs);
		}
	}
}

void USocketServerBPLibrary::serverPluginGetSocketSessionInfoByServerID(const FString serverID, const FString sessionID, bool& sessionFound, FString& IP, int32& port, EServerSocketConnectionProtocol& connectionProtocol) {
	USocketServerBPLibrary::getSocketServerTarget()->serverPluginGetSocketSessionInfoByServerIDNonStatic(serverID, sessionID,sessionFound, IP, port, connectionProtocol);
}

void USocketServerBPLibrary::serverPluginGetSocketSessionInfoByServerIDNonStatic(const FString serverID, const FString sessionID, bool& sessionFound, FString& IP, int32& port, EServerSocketConnectionProtocol& connectionProtocol){
	if (sessionID.IsEmpty()) {
		sessionFound = false;
		return;
	}

	FSocketServerPluginSession* sessionPointer = nullptr;

	if (udpServers.Find(serverID) != nullptr) {
		sessionPointer = (*udpServers.Find(serverID))->getClientSession(sessionID);
	}
	else {
		sessionPointer = (*tcpServers.Find(serverID))->clientSessions.Find(sessionID);
	}

	if (sessionPointer != nullptr) {
		FSocketServerPluginSession& session = *sessionPointer;
		IP = session.ip;
		port = session.port;
		connectionProtocol = session.protocol;
		sessionFound = true;
	}
	else {
		sessionFound = false;
		IP = "";
		port = -1;
		connectionProtocol = EServerSocketConnectionProtocol::E_NotSet;
	}
}


void USocketServerBPLibrary::serverPluginGetSocketSessionInfo(const FString sessionID, bool &sessionFound, FString &IP, int32 &port, EServerSocketConnectionProtocol &connectionProtocol, FString& serverID) {
	USocketServerBPLibrary::getSocketServerTarget()->serverPluginGetSocketSessionInfoNonStatic(sessionID, sessionFound, IP, port, connectionProtocol, serverID);
}

void USocketServerBPLibrary::serverPluginGetSocketSessionInfoNonStatic(const FString sessionID, bool& sessionFound, FString& IP, int32& port, EServerSocketConnectionProtocol& connectionProtocol, FString& serverID){
	if (sessionID.IsEmpty()) {
		sessionFound = false;
		return;
	}

	sessionFound = false;
	IP = "";
	port = -1;
	connectionProtocol = EServerSocketConnectionProtocol::E_NotSet;

	//udp
	for (auto& element : udpServers) {
		serverPluginGetSocketSessionInfoByServerID(element.Key, sessionID, sessionFound, IP, port, connectionProtocol);
		if (sessionFound) {
			serverID = element.Key;
			break;
		}
	}

	//tcp
	if (sessionFound == false) {
		for (auto& element : tcpServers) {
			serverPluginGetSocketSessionInfoByServerID(element.Key, sessionID, sessionFound, IP, port, connectionProtocol);
			if (sessionFound) {
				serverID = element.Key;
				break;
			}
		}
	}
}

void USocketServerBPLibrary::removeSessionAndCloseConnection(FString sessionId, FString optionalServerID) {
	USocketServerBPLibrary::getSocketServerTarget()->removeSessionAndCloseConnectionNonStatic(sessionId, optionalServerID);
}

void USocketServerBPLibrary::removeSessionAndCloseConnectionNonStatic(FString sessionId, FString optionalServerID){
	if (optionalServerID.IsEmpty()) {
		if (lastUDPServerID.IsEmpty() == false && udpServers.Find(lastUDPServerID) != nullptr) {
			(*udpServers.Find(lastUDPServerID))->removeClientSession(sessionId);
		}
		if (lastTCPServerID.IsEmpty() == false && tcpServers.Find(lastTCPServerID) != nullptr) {
			(*tcpServers.Find(lastTCPServerID))->removeClientSession(sessionId);
		}
	}
	else {
		if (udpServers.Find(optionalServerID) != nullptr) {
			(*udpServers.Find(optionalServerID))->removeClientSession(sessionId);
		}
		if (optionalServerID.IsEmpty() == false && tcpServers.Find(optionalServerID) != nullptr) {
			(*tcpServers.Find(optionalServerID))->removeClientSession(sessionId);
		}
	}
}

FIPandPortStruct USocketServerBPLibrary::checkIpAndPort(FString IP, int32 port) {

	FIPandPortStruct ipAndPortStruct;
	ipAndPortStruct.ip = IP;
	ipAndPortStruct.port = port;

	ISocketSubsystem* socketSubSystem = getSocketSubSystem();
	if (socketSubSystem != nullptr) {
		TSharedRef<FInternetAddr> internetAddr = socketSubSystem->CreateInternetAddr();
		internetAddr->SetIp(*IP, ipAndPortStruct.success);
		if (ipAndPortStruct.success == false) {
			UE_LOG(LogTemp, Error, TEXT("Invalid IP:%s"), *IP);
			ipAndPortStruct.errorMessage = "Invalid IP: " + IP;
			return ipAndPortStruct;
		}
	}

	//simple port check
	if (port < 0 || port > 65535) {
		UE_LOG(LogTemp, Error, TEXT("Invalid port:%i"), port);
		ipAndPortStruct.errorMessage = "Invalid port: " + FString::FromInt(port);
		ipAndPortStruct.success = false;
		return ipAndPortStruct;
	}

	
	
	return ipAndPortStruct;
}




void USocketServerBPLibrary::stopUDPServer(FString optionalServerID) {
	USocketServerBPLibrary::getSocketServerTarget()->stopUDPServerNonStatic(optionalServerID);
}

void USocketServerBPLibrary::stopUDPServerNonStatic(FString optionalServerID){
	if (optionalServerID.IsEmpty()) {
		optionalServerID = lastUDPServerID;
	}

	if (optionalServerID.IsEmpty() || udpServers.Find(optionalServerID) == nullptr) {
		UE_LOG(LogTemp, Warning, TEXT("ServerID not found: %s"), *optionalServerID);
		return;
	}

	USocketServerUDP* udpServer = *udpServers.Find(optionalServerID);
	if (udpServer != nullptr) {
		udpServer->stopUDPServer();
	}
	udpServers.Remove(optionalServerID);

	AsyncTask(ENamedThreads::GameThread, [optionalServerID]() {
		USocketServerBPLibrary::socketServerBPLibrary->onsocketServerUDPConnectionEventDelegate.Broadcast(false, "UDP Server stopped.", optionalServerID);
		});
}

void USocketServerBPLibrary::stopAllUDPServers() {
	USocketServerBPLibrary::getSocketServerTarget()->stopAllUDPServersNonStatic();
}

void USocketServerBPLibrary::stopAllUDPServersNonStatic(){
	TArray<FString> toRemoveServerKeys;
	for (auto& element : udpServers) {
		toRemoveServerKeys.Add(element.Key);
	}

	for (auto& element : toRemoveServerKeys) {
		stopUDPServer(element);
	}
}

void USocketServerBPLibrary::startUDPServer(FString& serverID, FString IP, int32 port ,bool multicast, EReceiveFilterServer receiveFilter, FString optionalServerID, int32 maxPacketSize) {
	USocketServerBPLibrary::getSocketServerTarget()->startUDPServerNonStatic(serverID, IP, port, multicast, receiveFilter, optionalServerID, maxPacketSize);
}

void USocketServerBPLibrary::startUDPServerNonStatic(FString& serverID, FString IP, int32 port, bool multicast, EReceiveFilterServer receiveFilter, FString optionalServerID, int32 maxPacketSize){
	serverID = optionalServerID;

	if (serverID.IsEmpty()) {
		serverID = FGuid::NewGuid().ToString();
		UE_LOG(LogTemp, Display, TEXT("SimpleSocketServer Plugin: ServerID not set. Generate automatically one :%s"), *serverID);
	}

	if (udpServers.Find(serverID) != nullptr) {
		AsyncTask(ENamedThreads::GameThread, [serverID]() {
			USocketServerBPLibrary::socketServerBPLibrary->onsocketServerUDPConnectionEventDelegate.Broadcast(false, "An server with this serverID is already started.", serverID);
			});
		return;
	}

	FIPandPortStruct ipStruct = checkIpAndPort(IP, port);
	if (ipStruct.success == false) {
		AsyncTask(ENamedThreads::GameThread, [IP, port, serverID]() {
			USocketServerBPLibrary::socketServerBPLibrary->onsocketServerUDPConnectionEventDelegate.Broadcast(false, "Can't start server on: " + IP + ":" + FString::FromInt(port), serverID);
			});
		return;
	}

	//check used port
	for (auto& element : udpServers) {
		USocketServerUDP* udpServerTmp = element.Value;
		if (udpServerTmp != nullptr) {
			if (udpServerTmp->getIP().Equals(IP) && udpServerTmp->getPort() == port) {
				FString tmpServerID = udpServerTmp->getServerID();
				AsyncTask(ENamedThreads::GameThread, [IP, port, serverID, tmpServerID]() {
					USocketServerBPLibrary::socketServerBPLibrary->onsocketServerUDPConnectionEventDelegate.Broadcast(false, "Can't start server. Address is already used by: " + tmpServerID, serverID);
					});
				return;
			}
		}
	}

	USocketServerUDP* udpServer = NewObject<USocketServerUDP>(USocketServerUDP::StaticClass());
	udpServers.Add(serverID, udpServer);
	udpServer->startUDPServer(ipStruct, IP, port, multicast, receiveFilter, serverID, maxPacketSize);

	lastUDPServerID = serverID;
}


void USocketServerBPLibrary::socketServerSendUDPMessage(TArray<FString> clientSessionIDs, FString message, TArray<uint8> byteArray, bool addLineBreak, bool asynchronous, ESocketServerUDPSocketType socketType, FString optionalServerID) {
	USocketServerBPLibrary::getSocketServerTarget()->socketServerSendUDPMessageNonStatic(clientSessionIDs, message, byteArray, addLineBreak, asynchronous, socketType, optionalServerID);
}

void USocketServerBPLibrary::socketServerSendUDPMessageNonStatic(TArray<FString> clientSessionIDs, FString message, TArray<uint8> byteArray, bool addLineBreak, bool asynchronous, ESocketServerUDPSocketType socketType, FString optionalServerID){
	if (message.Len() == 0 && byteArray.Num() == 0)
		return;

	if (optionalServerID.IsEmpty()) {
		optionalServerID = lastUDPServerID;
	}

	if (optionalServerID.IsEmpty() || udpServers.Find(optionalServerID) == nullptr) {
		UE_LOG(LogTemp, Warning, TEXT("SimpleSocketServer Plugin: ServerID not found: %s"), *optionalServerID);
		return;
	}

	if (message.Len() > 0 && addLineBreak) {
		message.Append("\r\n");
	}

	USocketServerUDP* udpServer = *udpServers.Find(optionalServerID);
	if (udpServer != nullptr) {
		udpServer->sendUDPMessage(clientSessionIDs, message, byteArray, asynchronous, socketType);
	}
	else {
		UE_LOG(LogTemp, Warning, TEXT("SimpleSocketServer Plugin: Server not found or not started: %s"), *optionalServerID);
		return;
	}
}


void USocketServerBPLibrary::socketServerSendUDPMessageToClient(FString clientSessionID, FString message, TArray<uint8> byteArray, bool addLineBreak, bool asynchronous, ESocketServerUDPSocketType socketType, FString optionalServerID) {
	USocketServerBPLibrary::getSocketServerTarget()->socketServerSendUDPMessageToClientNonStatic(clientSessionID, message, byteArray, addLineBreak, asynchronous,  socketType, optionalServerID);
}

void USocketServerBPLibrary::socketServerSendUDPMessageToClientNonStatic(FString clientSessionID, FString message, TArray<uint8> byteArray, bool addLineBreak, bool asynchronous, ESocketServerUDPSocketType socketType, FString optionalServerID){
	if (message.Len() == 0 && byteArray.Num() == 0)
		return;

	if (optionalServerID.IsEmpty()) {
		optionalServerID = lastUDPServerID;
	}

	if (optionalServerID.IsEmpty() || udpServers.Find(optionalServerID) == nullptr) {
		UE_LOG(LogTemp, Warning, TEXT("SimpleSocketServer Plugin: ServerID not found: %s"), *optionalServerID);
		return;
	}

	if (message.Len() > 0 && addLineBreak) {
		message.Append("\r\n");
	}

	USocketServerUDP* udpServer = *udpServers.Find(optionalServerID);
	if (udpServer != nullptr) {
		udpServer->sendUDPMessageToClient(clientSessionID, message, byteArray, asynchronous, socketType);
	}
	else {
		UE_LOG(LogTemp, Warning, TEXT("SimpleSocketServer Plugin: Server not found or not started: %s"), *optionalServerID);
		return;
	}
}

void USocketServerBPLibrary::socketServerSendUDPMessageTo(FString ip, int32 port, FString message, TArray<uint8> byteArray, bool addLineBreak, bool asynchronous, FString optionalServerID) {
	USocketServerBPLibrary::getSocketServerTarget()->socketServerSendUDPMessageToNonStatic(ip, port, message, byteArray, addLineBreak,  asynchronous, optionalServerID);
}

void USocketServerBPLibrary::socketServerSendUDPMessageToNonStatic(FString ip, int32 port, FString message, TArray<uint8> byteArray, bool addLineBreak, bool asynchronous, FString optionalServerID){
	if (message.Len() == 0 && byteArray.Num() == 0)
		return;

	if (optionalServerID.IsEmpty()) {
		optionalServerID = lastUDPServerID;
	}

	if (optionalServerID.IsEmpty() || udpServers.Find(optionalServerID) == nullptr) {
		UE_LOG(LogTemp, Warning, TEXT("SimpleSocketServer Plugin: ServerID not found: %s"), *optionalServerID);
		return;
	}

	if (message.Len() > 0 && addLineBreak) {
		message.Append("\r\n");
	}

	USocketServerUDP* udpServer = *udpServers.Find(optionalServerID);
	if (udpServer != nullptr) {
		udpServer->sendUDPMessageTo(ip, port, message, byteArray, asynchronous);
	}
	else {
		UE_LOG(LogTemp, Warning, TEXT("SimpleSocketServer Plugin: Server not found or not started: %s"), *optionalServerID);
		return;
	}
}



//TCP
void USocketServerBPLibrary::startTCPServerNontStatic(FString& serverID, FString IP, int32 port, EReceiveFilterServer receiveFilter,
	ESocketServerTCPSeparator messageSeparatorP,
	FString optionalServerID, bool isFileServer, FString Aes256bitKey, bool resumeFiles) {

	serverID = optionalServerID;

	if (serverID.IsEmpty()) {
		serverID = FGuid::NewGuid().ToString();
		UE_LOG(LogTemp, Display, TEXT("SimpleSocketServer Plugin: ServerID not set. Generate automatically one :%s"), *serverID);
	}

	if (tcpServers.Find(serverID) != nullptr) {
		AsyncTask(ENamedThreads::GameThread, [serverID]() {
			USocketServerBPLibrary::socketServerBPLibrary->onsocketServerConnectionEventDelegate.Broadcast(EServerSocketConnectionEventType::E_Server, false, "An server with this serverID is already started.","",serverID);
		});
		return;
	}

	FIPandPortStruct ipStruct = checkIpAndPort(IP, port);
	if (ipStruct.success == false) {
		AsyncTask(ENamedThreads::GameThread, [IP, port, serverID]() {
			USocketServerBPLibrary::socketServerBPLibrary->onsocketServerConnectionEventDelegate.Broadcast(EServerSocketConnectionEventType::E_Server, false, "Can't start server on: " + IP + ":" + FString::FromInt(port), "", serverID);
		});
		return;
	}

	//check used port
	for (auto& element : tcpServers) {
		USocketServerTCP* tcpServerTmp = element.Value;
		if (tcpServerTmp != nullptr) {
			if (tcpServerTmp->getIP().Equals(IP) && tcpServerTmp->getPort() == port) {
				FString tmpServerID = tcpServerTmp->getServerID();
				AsyncTask(ENamedThreads::GameThread, [IP, port, serverID, tmpServerID]() {
					USocketServerBPLibrary::socketServerBPLibrary->onsocketServerConnectionEventDelegate.Broadcast(EServerSocketConnectionEventType::E_Server, false, "Can't start server. Address is already used by: " + tmpServerID, "", serverID);
				});
				return;
			}
		}
	}

	USocketServerTCP* tcpServer = NewObject<USocketServerTCP>(USocketServerTCP::StaticClass());
	if (serverID.IsEmpty()) {
		serverID = FGuid::NewGuid().ToString();
		UE_LOG(LogTemp, Warning, TEXT("SimpleSocketServer Plugin: ServerID not set. Generate automatically one :%s"), *serverID);
	}
	tcpServers.Add(serverID, tcpServer);
	tcpServer->startTCPServer(ipStruct, IP, port, receiveFilter, messageSeparatorP, serverID,isFileServer,Aes256bitKey,resumeFiles);

	lastTCPServerID = serverID;
}

void USocketServerBPLibrary::startTCPServer(FString& serverID, FString IP, int32 port, EReceiveFilterServer receiveFilter, ESocketServerTCPSeparator messageSeparatorP,
	FString optionalServerID) {
	USocketServerBPLibrary::getSocketServerTarget()->startTCPServerNontStatic(serverID, IP, port, receiveFilter, messageSeparatorP, optionalServerID, false, "", false);
}

void USocketServerBPLibrary::startTCPFileServer(FString& serverID, FString IP, int32 port, FString customServerID, FString Aes256bitKey, bool resumeFiles){
	USocketServerBPLibrary::getSocketServerTarget()->startTCPServerNontStatic(serverID, IP, port, EReceiveFilterServer::E_SAB, ESocketServerTCPSeparator::E_None,
		customServerID, true, Aes256bitKey, resumeFiles);
}


void USocketServerBPLibrary::addFileToken(FString token, bool deleteAfterUse, EFileFunctionsSocketServerDirectoryType directoryType, FString fileDirectory){
	if (token.IsEmpty())
		return;
	struct FSocketServerToken fst;
	fst.token = token;
	fst.deleteAfterUse = deleteAfterUse;
	fst.directoryType = directoryType;
	fst.fileDirectory = fileDirectory;


	USocketServerBPLibrary::getSocketServerTarget()->fileTokenMap.Add(token, fst);
}

void USocketServerBPLibrary::addFileTokens(TMap<FString, FString> fileTokens, bool deleteAfterUse, EFileFunctionsSocketServerDirectoryType directoryType){
	for (auto& element : fileTokens) {
		addFileToken(element.Key, deleteAfterUse, directoryType, element.Value);
	}
}

void USocketServerBPLibrary::removeFileToken(FString token){
	USocketServerBPLibrary::getSocketServerTarget()->fileTokenMap.Remove(token);
}


void USocketServerBPLibrary::stopTCPServer(FString serverID) {
	USocketServerBPLibrary::getSocketServerTarget()->stopTCPServerNonSTatic(serverID);
}

void USocketServerBPLibrary::stopTCPServerNonSTatic(FString serverID){
	if (serverID.IsEmpty()) {
		serverID = lastTCPServerID;
	}

	if (rconServers.Find(serverID) != nullptr) {
		unregiserRCONServer(serverID);
	}

	if (serverID.IsEmpty() || tcpServers.Find(serverID) == nullptr) {
		UE_LOG(LogTemp, Warning, TEXT("ServerID not found: %s"), *serverID);
		return;
	}

	USocketServerTCP* tcpServer = *tcpServers.Find(serverID);
	if (tcpServer != nullptr) {
		tcpServer->stopTCPServer();
	}
	tcpServers.Remove(serverID);
}

void USocketServerBPLibrary::stopAllTCPServers() {
	USocketServerBPLibrary::getSocketServerTarget()->stopAllTCPServersNonStatic();
}

void USocketServerBPLibrary::stopAllTCPServersNonStatic(){
	TArray<FString> toRemoveServerKeys;
	for (auto& element : tcpServers) {
		toRemoveServerKeys.Add(element.Key);
	}

	for (auto& element : toRemoveServerKeys) {
		stopTCPServer(element);
	}
}

void USocketServerBPLibrary::socketServerSendTCPMessage(TArray<FString> clientSessionIDs, FString message, TArray<uint8> byteArray, bool addLineBreak,FString optionalServerID) {
	USocketServerBPLibrary::getSocketServerTarget()->socketServerSendTCPMessageNonStatic( clientSessionIDs, message,  byteArray, addLineBreak, optionalServerID);
}

void USocketServerBPLibrary::socketServerSendTCPMessageNonStatic(TArray<FString> clientSessionIDs, FString message, TArray<uint8> byteArray, bool addLineBreak, FString optionalServerID){
	if (message.Len() == 0 && byteArray.Num() == 0)
		return;

	if (optionalServerID.IsEmpty()) {
		optionalServerID = lastTCPServerID;
	}

	if (optionalServerID.IsEmpty() || tcpServers.Find(optionalServerID) == nullptr) {
		UE_LOG(LogTemp, Warning, TEXT("SimpleSocketServer Plugin: ServerID not found: %s"), *optionalServerID);
		return;
	}

	USocketServerTCP* tcpServer = *tcpServers.Find(optionalServerID);
	if (tcpServer != nullptr) {
		tcpServer->sendTCPMessage(clientSessionIDs, message, byteArray, addLineBreak);
	}
	else {
		UE_LOG(LogTemp, Warning, TEXT("SimpleSocketServer Plugin: Server not found or not started: %s"), *optionalServerID);
		return;
	}
}

void USocketServerBPLibrary::socketServerSendTCPMessageToClient(FString clientSessionID, FString message, TArray<uint8> byteArray, bool addLineBreak, FString optionalServerID) {
	USocketServerBPLibrary::getSocketServerTarget()->socketServerSendTCPMessageToClientNonStatic(clientSessionID, message,  byteArray, addLineBreak, optionalServerID);
}

void USocketServerBPLibrary::socketServerSendTCPMessageToClientNonStatic(FString clientSessionID, FString message, TArray<uint8> byteArray, bool addLineBreak, FString optionalServerID){
	if (message.Len() == 0 && byteArray.Num() == 0)
		return;

	if (optionalServerID.IsEmpty()) {
		optionalServerID = lastTCPServerID;
	}

	if (optionalServerID.IsEmpty() || tcpServers.Find(optionalServerID) == nullptr) {
		UE_LOG(LogTemp, Warning, TEXT("SimpleSocketServer Plugin: ServerID not found: %s"), *optionalServerID);
		return;
	}

	USocketServerTCP* tcpServer = *tcpServers.Find(optionalServerID);
	if (tcpServer != nullptr) {
		tcpServer->sendTCPMessageToClient(clientSessionID, message, byteArray, addLineBreak);
	}
	else {
		UE_LOG(LogTemp, Warning, TEXT("SimpleSocketServer Plugin: Server not found or not started: %s"), *optionalServerID);
		return;
	}
}

void USocketServerBPLibrary::changeTCPSeparatorStringOnServer(FString separator) {
	USocketServerBPLibrary::getSocketServerTarget()->changeTCPSeparatorStringOnServerNonStatic(separator);
}

void USocketServerBPLibrary::changeTCPSeparatorStringOnServerNonStatic(FString separator){
	tcpStringSeparator = separator;
}

void USocketServerBPLibrary::changeTCPSeparatorByteOnServer(uint8 separator) {
	USocketServerBPLibrary::getSocketServerTarget()->changeTCPSeparatorByteOnServerNonStatic(separator);
}

void USocketServerBPLibrary::changeTCPSeparatorByteOnServerNonStatic(uint8 separator) {
	tcpByteSeparator = separator;
}


void USocketServerBPLibrary::getTcpSeparator(uint8& byteSeparator, FString& stringSeparator) {
	stringSeparator = tcpStringSeparator;
	byteSeparator = tcpByteSeparator;
}

void USocketServerBPLibrary::cleanConnection(FSocketServerPluginSession& session) {
	socketServerPluginCleanerThread->addSession(session);
}

TMap<FString, USocketServerTCP*> USocketServerBPLibrary::getTcpServerMap(){
	return tcpServers;
}



FString USocketServerBPLibrary::generateUniqueID(){
	return FGuid::NewGuid().ToString();
}

UDNSClientSocketServer* USocketServerBPLibrary::resolveDomain(FString domain, bool useDNSCache, FString dnsIP){
	return USocketServerBPLibrary::getSocketServerTarget()->resolveDomainNonStatic(domain, useDNSCache, dnsIP);
}

UDNSClientSocketServer* USocketServerBPLibrary::resolveDomainNonStatic(FString domain, bool useDNSCache, FString dnsIP){
	UDNSClientSocketServer* dnsClient = NewObject<UDNSClientSocketServer>(UDNSClientSocketServer::StaticClass());
	dnsClient->resolveDomain(USocketServerBPLibrary::getSocketSubSystem(), domain, useDNSCache, dnsIP);
	return dnsClient;
}

void USocketServerBPLibrary::changeSocketPlatform(ESocketPlatformServer platform){
		USocketServerBPLibrary::getSocketServerTarget()->systemSocketPlatform = platform;
}

bool USocketServerBPLibrary::checkPort(EServerSocketConnectionCheckPortType type, FString ip, int32 port){
	return USocketServerBPLibrary::getSocketServerTarget()->checkPortNonStatic(type, ip, port);
}

bool USocketServerBPLibrary::checkPortNonStatic(EServerSocketConnectionCheckPortType type, FString ip, int32 port){
	if (type == EServerSocketConnectionCheckPortType::E_TCP) {
		FIPandPortStruct ipAndPort = checkIpAndPort(ip, port);
		if (!ipAndPort.success) {
			return false;
		}
		FSocket* listenerSocket = nullptr;
		ISocketSubsystem* socketSubSystem = USocketServerBPLibrary::getSocketSubSystem();
		if (socketSubSystem != nullptr) {

			if (socketSubSystem != nullptr) {
				TSharedRef<FInternetAddr> internetAddr = socketSubSystem->CreateInternetAddr();
				bool validIP = false;
				internetAddr->SetIp(*ip, validIP);
				internetAddr->SetPort(port);

				if (validIP) {
					listenerSocket = socketSubSystem->CreateSocket(NAME_Stream, *FString("USocketServerBPLibraryListenerCheckTCPPort"), internetAddr->GetProtocolType());
				}

				if (listenerSocket != nullptr) {
					listenerSocket->SetLinger(false, 0);

					if (!listenerSocket->Bind(*internetAddr)) {
						if (listenerSocket != nullptr) {
							listenerSocket->Close();
							if (socketSubSystem != nullptr)
								socketSubSystem->DestroySocket(listenerSocket);
							listenerSocket = nullptr;
						}
						return false;
					}


					if (!listenerSocket->Listen(1)) {
						if (listenerSocket != nullptr) {
							listenerSocket->Close();
							if (socketSubSystem != nullptr)
								socketSubSystem->DestroySocket(listenerSocket);
							listenerSocket = nullptr;
						}
						return false;
					}
				}
			}
		}
		if (listenerSocket == nullptr) {
			return false;
		}
		else {
			listenerSocket->Close();
			if (socketSubSystem != nullptr)
				socketSubSystem->DestroySocket(listenerSocket);
			listenerSocket = nullptr;
			return true;
		}
	}
	else {
		ISocketSubsystem* socketSubSystem = USocketServerBPLibrary::getSocketSubSystem();
		FSocket* listenerSocket = nullptr;
		if (socketSubSystem != nullptr) {
			TSharedPtr<class FInternetAddr> addr = socketSubSystem->CreateInternetAddr();
			bool validIP = true;
			addr->SetPort(port);
			addr->SetIp(*ip, validIP);

			if (!validIP) {
				return false;
			}
			FString socketName;
			listenerSocket = socketSubSystem->CreateSocket(NAME_DGram, *socketName, addr->GetProtocolType());
			if (listenerSocket == nullptr) {
				return false;
			}
			listenerSocket->SetReuseAddr();
			listenerSocket->SetNonBlocking();
			if (!listenerSocket->Bind(*addr)) {
				return false;
			}
		}

		if (listenerSocket == nullptr) {
			return false;
		}
		else {
			listenerSocket->Close();
			if (socketSubSystem != nullptr)
				socketSubSystem->DestroySocket(listenerSocket);
			listenerSocket = nullptr;
			return true;
		}
	}
	return false;
}

int32 USocketServerBPLibrary::getRandomPort(EServerSocketConnectionCheckPortType type, FString ip){
	return USocketServerBPLibrary::getSocketServerTarget()->getRandomPortNonStatic(type, ip);
}

int32 USocketServerBPLibrary::getRandomPortNonStatic(EServerSocketConnectionCheckPortType type, FString ip){
	int32 port = 0;
	if (type == EServerSocketConnectionCheckPortType::E_TCP) {
		FIPandPortStruct ipAndPort = checkIpAndPort(ip, port);
		if (!ipAndPort.success) {
			return 0;
		}
		FSocket* listenerSocket = nullptr;
		ISocketSubsystem* socketSubSystem = USocketServerBPLibrary::getSocketSubSystem();
		if (socketSubSystem != nullptr) {

			if (socketSubSystem != nullptr) {
				TSharedRef<FInternetAddr> internetAddr = socketSubSystem->CreateInternetAddr();
				bool validIP = false;
				internetAddr->SetIp(*ip, validIP);
				internetAddr->SetPort(port);

				if (validIP) {
					listenerSocket = socketSubSystem->CreateSocket(NAME_Stream, *FString("USocketServerBPLibraryListenerCheckTCPPort"), internetAddr->GetProtocolType());
				}

				if (listenerSocket != nullptr) {
					listenerSocket->SetLinger(false, 0);

					if (!listenerSocket->Bind(*internetAddr)) {
						if (listenerSocket != nullptr) {
							listenerSocket->Close();
							if (socketSubSystem != nullptr)
								socketSubSystem->DestroySocket(listenerSocket);
							listenerSocket = nullptr;
						}
						return 0;
					}


					if (!listenerSocket->Listen(1)) {
						if (listenerSocket != nullptr) {
							listenerSocket->Close();
							if (socketSubSystem != nullptr)
								socketSubSystem->DestroySocket(listenerSocket);
							listenerSocket = nullptr;
						}
						return 0;
					}
				}
			}
		}
		if (listenerSocket == nullptr) {
			return 0;
		}
		else {
			port = listenerSocket->GetPortNo();
			listenerSocket->Close();
			if (socketSubSystem != nullptr)
				socketSubSystem->DestroySocket(listenerSocket);
			listenerSocket = nullptr;
			return port;
		}
	}
	else {
		ISocketSubsystem* socketSubSystem = USocketServerBPLibrary::getSocketSubSystem();
		FSocket* listenerSocket = nullptr;
		if (socketSubSystem != nullptr) {
			TSharedPtr<class FInternetAddr> addr = socketSubSystem->CreateInternetAddr();
			bool validIP = true;
			addr->SetPort(port);
			addr->SetIp(*ip, validIP);

			if (!validIP) {
				return 0;
			}
			FString socketName;
			listenerSocket = socketSubSystem->CreateSocket(NAME_DGram, *socketName, addr->GetProtocolType());
			if (listenerSocket == nullptr) {
				return 0;
			}
			listenerSocket->SetReuseAddr();
			listenerSocket->SetNonBlocking();
			if (!listenerSocket->Bind(*addr)) {
				return 0;
			}
		}

		if (listenerSocket == nullptr) {
			return 0;
		}
		else {
			port = listenerSocket->GetPortNo();
			listenerSocket->Close();
			if (socketSubSystem != nullptr)
				socketSubSystem->DestroySocket(listenerSocket);
			listenerSocket = nullptr;
			return port;
		}
	}
	return 0;
}

void USocketServerBPLibrary::registerClientEvent(FString sessionID, UEventBean*& event){
	USocketServerBPLibrary::getSocketServerTarget()->registerClientEventNonStatic(sessionID, event);
}

void USocketServerBPLibrary::registerClientEventNonStatic(FString sessionID, UEventBean*& event){
	UPROPERTY()
		UEventBean* eventBean = NewObject<UEventBean>(UEventBean::StaticClass());
	eventBean->AddToRoot();
	messageEvents.Add(sessionID, eventBean);
	event = eventBean;
}

void USocketServerBPLibrary::unregisterClientEvent(FString sessionID){
	USocketServerBPLibrary::getSocketServerTarget()->unregisterClientEventNonStatic(sessionID);
}

void USocketServerBPLibrary::unregisterClientEventNonStatic(FString sessionID){
	if (sessionID.IsEmpty())
		return;
	if (messageEvents.Find(sessionID) != nullptr) {
		UEventBean* event = *messageEvents.Find(sessionID);
		if (event != nullptr) {
			event->RemoveFromRoot();
		}
		messageEvents.Remove(sessionID);
	}
}

UEventBean* USocketServerBPLibrary::getResiteredClientEvent(FString sessionID){
	if (sessionID.IsEmpty())
		return nullptr;
	if (messageEvents.Find(sessionID) != nullptr) {
		return *messageEvents.Find(sessionID);
	}
	return nullptr;
}

void USocketServerBPLibrary::registerRCONServer(FString serverID, ERCONPasswordType passwordType, FString passwordOrFile, bool& success, FString& errorMessage){
	URCONServer* rconServer = NewObject<URCONServer>(URCONServer::StaticClass());
	rconServer->AddToRoot();
	rconServer->startRCONServer(serverID, passwordType, passwordOrFile, success, errorMessage);

	if (success) {
		USocketServerBPLibrary::getSocketServerTarget()->onserverReceiveTCPMessageEventDelegate.AddDynamic(rconServer, &URCONServer::receiveTCPMessageEvent);
		USocketServerBPLibrary::getSocketServerTarget()->rconServers.Add(serverID, rconServer);
	}
}

void USocketServerBPLibrary::unregiserRCONServer(FString serverID){
	if (USocketServerBPLibrary::getSocketServerTarget()->rconServers.Find(serverID) != nullptr) {
		URCONServer* rconServer = *USocketServerBPLibrary::getSocketServerTarget()->rconServers.Find(serverID);
		USocketServerBPLibrary::getSocketServerTarget()->onserverReceiveTCPMessageEventDelegate.RemoveDynamic(rconServer, &URCONServer::receiveTCPMessageEvent);
		USocketServerBPLibrary::getSocketServerTarget()->rconServers.Remove(serverID);
		rconServer->RemoveFromRoot();
		rconServer = nullptr;
	}
}

void USocketServerBPLibrary::sendRCONResponse(FString sessionID, FString serverID, int32 requestID, FString response){
	if (USocketServerBPLibrary::getSocketServerTarget()->rconServers.Find(serverID) == nullptr) {
		UE_LOG(LogTemp, Warning, TEXT("SimpleSocketServer Plugin (RCON): Server noch registered. %s"),*serverID);
		return;
	}
	URCONServer* rconServer = *USocketServerBPLibrary::getSocketServerTarget()->rconServers.Find(serverID);
	rconServer->sendResponse(sessionID, serverID,requestID, 0, response);
}


void USocketServerBPLibrary::changeCleanerThreadSettingsOnServer(bool showLogs, int32 minLiveTimeInSeconds) {
	if (USocketServerBPLibrary::getSocketServerTarget()->socketServerPluginCleanerThread != nullptr) {
		USocketServerBPLibrary::getSocketServerTarget()->socketServerPluginCleanerThread->changeSettings(showLogs, minLiveTimeInSeconds);
	}
}

void USocketServerBPLibrary::parseBytesToFloat(TArray<uint8> bytes, float& value) {

	if (bytes.Num() != 4) {
		UE_LOG(LogTemp, Error, TEXT("ParseBytesToFloat: Cannot convert bytes to float. Array must contain 4 bytes but has %i bytes."), bytes.Num());
		return;
	}

	FMemory::Memcpy(&value, bytes.GetData(), 4);

}

void USocketServerBPLibrary::parseBytesToInteger(TArray<uint8> bytes, int32& value) {


	if (bytes.Num() != 4) {
		UE_LOG(LogTemp, Error, TEXT("ParseBytesToInteger: Cannot convert bytes to integer. Array must contain 4 bytes but has %i bytes."), bytes.Num());
		return;
	}

	FMemory::Memcpy(&value, bytes.GetData(), 4);
}

void USocketServerBPLibrary::parseBytesToInteger64(TArray<uint8> bytes, int64& value) {


	if (bytes.Num() != 8) {
		UE_LOG(LogTemp, Error, TEXT("ParseBytesToInteger64: Cannot convert bytes to integer. Array must contain 8 bytes but has %i bytes."), bytes.Num());
		return;
	}

	FMemory::Memcpy(&value, bytes.GetData(), 8);
}

void USocketServerBPLibrary::parseBytesToFloatPure(TArray<uint8> bytes, float& value){
	parseBytesToFloat(bytes, value);
}

void USocketServerBPLibrary::parseBytesToIntegerPure(TArray<uint8> bytes, int32& value){
	parseBytesToInteger(bytes, value);
}

void USocketServerBPLibrary::parseBytesToInteger64Pure(TArray<uint8> bytes, int64& value){
	parseBytesToInteger64(bytes, value);
}

void USocketServerBPLibrary::parseBytesToFloatEndian(TArray<uint8> bytes, float& littleEndian, float& bigEndian) {
	littleEndian = 0.f;
	bigEndian = 0.f;

	if (bytes.Num() != 4) {
		UE_LOG(LogTemp, Error, TEXT("ParseBytesToFloat: Cannot convert bytes to float. Array must contain 4 bytes but has %i bytes."), bytes.Num());
		return;
	}

	uint8 littleEndianChar[] = { bytes[0], bytes[1], bytes[2], bytes[3] };
	FMemory::Memcpy(&littleEndian, &littleEndianChar, sizeof(littleEndian));

	uint8 bigEndianChar[] = { bytes[3], bytes[2], bytes[1], bytes[0] };
	FMemory::Memcpy(&bigEndian, &bigEndianChar, sizeof(bigEndian));
}

void USocketServerBPLibrary::parseBytesToIntegerEndian(TArray<uint8> bytes, int32& littleEndian, int32& bigEndian){
	littleEndian = 0;
	bigEndian = 0;

	if (bytes.Num() != 4){
		UE_LOG(LogTemp, Error, TEXT("ParseBytesToInteger: Cannot convert bytes to integer. Array must contain 4 bytes but has %i bytes."), bytes.Num());
		return;
	}
		

	uint8 littleEndianChar[] = { bytes[0], bytes[1], bytes[2], bytes[3] };
	FMemory::Memcpy(&littleEndian, &littleEndianChar, sizeof(littleEndian));

	uint8 bigEndianChar[] = { bytes[3], bytes[2], bytes[1], bytes[0] };
	FMemory::Memcpy(&bigEndian, &bigEndianChar, sizeof(bigEndian));
}

void USocketServerBPLibrary::parseBytesToInteger64Endian(TArray<uint8> bytes, int64& littleEndian, int64& bigEndian) {
	littleEndian = 0;
	bigEndian = 0;

	if (bytes.Num() != 8) {
		UE_LOG(LogTemp, Error, TEXT("ParseBytesToInteger64: Cannot convert bytes to integer. Array must contain 8 bytes but has %i bytes."), bytes.Num());
		return;
	}return;

	uint8 littleEndianChar[] = { bytes[0], bytes[1], bytes[2], bytes[3],bytes[4], bytes[5], bytes[6], bytes[7] };
	FMemory::Memcpy(&littleEndian, &littleEndianChar, sizeof(littleEndian));

	uint8 bigEndianChar[] = { bytes[7], bytes[6], bytes[5], bytes[4], bytes[3], bytes[2], bytes[1], bytes[0] };
	FMemory::Memcpy(&bigEndian, &bigEndianChar, sizeof(bigEndian));
}

void USocketServerBPLibrary::parseFloatToBytes(TArray<uint8>& byteArray, float value, bool switchByteOrder){
	byteArray.Empty();
	union {
		float tmpVal;
		uint8 tmpArray[4];
	} u;
	u.tmpVal = value;
	byteArray.AddUninitialized(4);

	FMemory::Memcpy(byteArray.GetData(), u.tmpArray, 4);

	if (switchByteOrder) {
		byteArray.SwapMemory(3, 0);
		byteArray.SwapMemory(2, 1);
	}
}

void USocketServerBPLibrary::parseIntegerToBytes(TArray<uint8>& byteArray, int32 value, bool switchByteOrder){
	byteArray.Empty();
	union {
		int32 tmpVal;
		uint8 tmpArray[4];
	} u;
	u.tmpVal = value;
	byteArray.AddUninitialized(4);

	FMemory::Memcpy(byteArray.GetData(), u.tmpArray, 4);

	if (switchByteOrder) {
		byteArray.SwapMemory(3, 0);
		byteArray.SwapMemory(2, 1);
	}
}

void USocketServerBPLibrary::parseInteger64ToBytes(TArray<uint8>& byteArray, int64 value, bool switchByteOrder){
	byteArray.Empty();
	union {
		int64 tmpVal;
		uint8 tmpArray[8];
	} u;
	u.tmpVal = value;
	byteArray.AddUninitialized(8);

	FMemory::Memcpy(byteArray.GetData(), u.tmpArray, 8);

	if (switchByteOrder) {
		byteArray.SwapMemory(7, 0);
		byteArray.SwapMemory(6, 1);
		byteArray.SwapMemory(5, 2);
		byteArray.SwapMemory(4, 3);
	}
}

void USocketServerBPLibrary::parseFloatToBytesPure(TArray<uint8>& byteArray, float value, bool switchByteOrder){
	byteArray.Empty();
	parseFloatToBytes(byteArray, value, switchByteOrder);
}

void USocketServerBPLibrary::parseIntegerToBytesPure(TArray<uint8>& byteArray, int32 value, bool switchByteOrder){
	byteArray.Empty();
	parseIntegerToBytes(byteArray, value, switchByteOrder);
}

void USocketServerBPLibrary::parseInteger64ToBytesPure(TArray<uint8>& byteArray, int64 value, bool switchByteOrder){
	byteArray.Empty();
	parseInteger64ToBytes(byteArray, value, switchByteOrder);
}

void USocketServerBPLibrary::parseBytesToFloatArrayPure(TArray<float>& value, TArray<uint8> bytes){
	if (bytes.Num() == 0 && bytes.Num()%4 != 0)
		return;
	value.Empty();
	value.AddUninitialized(bytes.Num()/4);
	FMemory::Memcpy(value.GetData(), bytes.GetData(), bytes.Num());
}

void USocketServerBPLibrary::parseBytesToIntegerArrayPure(TArray<int32>& value, TArray<uint8> bytes){
	if (bytes.Num() == 0 && bytes.Num() % 4 != 0)
		return;
	value.Empty();
	value.AddUninitialized(bytes.Num() / 4);
	FMemory::Memcpy(value.GetData(), bytes.GetData(), bytes.Num());
}

void USocketServerBPLibrary::parseBytesToInteger64ArrayPure(TArray<int64>& value, TArray<uint8> bytes){
	if (bytes.Num() == 0 && bytes.Num() % 8 != 0)
		return;
	value.Empty();
	value.AddUninitialized(bytes.Num() / 8);
	FMemory::Memcpy(value.GetData(), bytes.GetData(), bytes.Num());
}

void USocketServerBPLibrary::parseFloatArrayToBytesPure(TArray<uint8>& byteArray, TArray<float> value){
	byteArray.Empty();
	byteArray.AddUninitialized(value.Num() * 4);
	FMemory::Memcpy(byteArray.GetData(), value.GetData(), value.Num()*4);
}

void USocketServerBPLibrary::parseIntegerArrayToBytesPure(TArray<uint8>& byteArray, TArray<int32> value){
	byteArray.Empty();
	byteArray.AddUninitialized(value.Num() * 4);
	FMemory::Memcpy(byteArray.GetData(), value.GetData(), value.Num() * 4);
}

void USocketServerBPLibrary::parseInteger64ArrayToBytesPure(TArray<uint8>& byteArray, TArray<int64> value){
	byteArray.Empty();
	byteArray.AddUninitialized(value.Num() * 8);
	FMemory::Memcpy(byteArray.GetData(), value.GetData(), value.Num() * 8);
}

bool USocketServerBPLibrary::startUEGameHost(UObject* worldContextObject, FString Protocol, FString Host, FString Map, FString RedirectURL, TArray<FString> Op, FString Portal, int32 Port) {
	if (GEngine) {
		FURL url;
		url.Protocol = Protocol;
		url.Host = Host;
		url.Port = Port;
		url.Map = Map;
		url.RedirectURL = RedirectURL;
		url.Op = Op;
		url.Portal = Portal;
		UWorld* world = GEngine->GetWorldFromContextObjectChecked(worldContextObject);
		if (world) {
			return world->Listen(url);
		}
	}
	return false;
}

//void USocketServerBPLibrary::stopUEGameHost(UObject* worldContextObject) {
//	//It looks like the listen function checks if a server is already running and then closes it if it is running. 
//	//If you now call listen again with port 0 then the server is stopped but new one can't be created on port 0.
//	startUEGameHost(worldContextObject, 0);
//}

ISocketSubsystem * USocketServerBPLibrary::getSocketSubSystem(){
	switch (USocketServerBPLibrary::getSocketServerTarget()->systemSocketPlatform)
	{
	case ESocketPlatformServer::E_SSS_SYSTEM:
		//UE_LOG(LogTemp, Display, TEXT("E_SSS_SYSTEM"));
		return ISocketSubsystem::Get(PLATFORM_SOCKETSUBSYSTEM);
	case ESocketPlatformServer::E_SSS_WINDOWS:
		//UE_LOG(LogTemp, Display, TEXT("E_SSS_WINDOWS"));
		return ISocketSubsystem::Get(FName(TEXT("WINDOWS")));
	case ESocketPlatformServer::E_SSS_MAC:
		//UE_LOG(LogTemp, Display, TEXT("E_SSS_MAC"));
		return ISocketSubsystem::Get(FName(TEXT("MAC")));
	case ESocketPlatformServer::E_SSS_IOS:
		//UE_LOG(LogTemp, Display, TEXT("E_SSS_IOS"));
		return ISocketSubsystem::Get(FName(TEXT("IOS")));
	case ESocketPlatformServer::E_SSS_UNIX:
		//UE_LOG(LogTemp, Display, TEXT("E_SSS_UNIX"));
		return ISocketSubsystem::Get(FName(TEXT("UNIX")));
	case ESocketPlatformServer::E_SSS_ANDROID:
		//UE_LOG(LogTemp, Display, TEXT("E_SSS_ANDROID"));
		return ISocketSubsystem::Get(FName(TEXT("ANDROID")));
	case ESocketPlatformServer::E_SSS_PS4:
		//UE_LOG(LogTemp, Display, TEXT("E_SSS_PS4"));
		return ISocketSubsystem::Get(FName(TEXT("PS4")));
	case ESocketPlatformServer::E_SSS_XBOXONE:
		//UE_LOG(LogTemp, Display, TEXT("E_SSS_XBOXONE"));
		return ISocketSubsystem::Get(FName(TEXT("XBOXONE")));
	case ESocketPlatformServer::E_SSS_HTML5:
		//UE_LOG(LogTemp, Display, TEXT("E_SSS_HTML5"));
		return ISocketSubsystem::Get(FName(TEXT("HTML5")));
	case ESocketPlatformServer::E_SSS_SWITCH:
		//UE_LOG(LogTemp, Display, TEXT("E_SSS_SWITCH"));
		return ISocketSubsystem::Get(FName(TEXT("SWITCH")));
	case ESocketPlatformServer::E_SSS_DEFAULT:
		//UE_LOG(LogTemp, Display, TEXT("E_SSS_DEFAULT"));
		return ISocketSubsystem::Get();
	default:
		//pUE_LOG(LogTemp, Display, TEXT("DEFAULT"));
		return ISocketSubsystem::Get(PLATFORM_SOCKETSUBSYSTEM);
	}	
}

//void USocketServerBPLibrary::addClientSession(FString key, FClientSocketSession& session) {
//	clientSessions.Add(key, session);
//}
//
//void USocketServerBPLibrary::removeClientSession(FString key) {
//	FClientSocketSession* sessionPointer = clientSessions.Find(key);	
//	if (sessionPointer != nullptr) {
//		FClientSocketSession& session = *sessionPointer;
//		
//		if (session.protocol == EServerSocketConnectionProtocol::E_TCP) {
//			if (session.TCPClientSendDataToServerThread != nullptr && session.TCPClientSendDataToServerThread->isRun()) {
//				session.TCPClientSendDataToServerThread->stopThread();
//			}
//			FSocket * socket = session.socket;
//			if (socket != nullptr) {
//				socket->Close();
//			}
//		}
//		clientSessions.Remove(key);
//	}
//	
//}

//void USocketServerBPLibrary::startTCPClientHandler(FClientSocketSession& session, EReceiveFilterServer receiveFilter) {
//	new FClientConnectionThread(session, receiveFilter);
//}
//
//bool USocketServerBPLibrary::isTCPServerRun() {
//	return tcpServerRun;
//}
//
//void USocketServerBPLibrary::setTCPServerRun(bool run) {
//	tcpServerRun = run;
//}
