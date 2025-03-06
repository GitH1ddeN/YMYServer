// Copyright 2017-2020 David Romanski (Socke). All Rights Reserved.

#include "SocketServerTCP.h"


USocketServerTCP::USocketServerTCP(const FObjectInitializer& ObjectInitializer) : Super(ObjectInitializer) {
	this->AddToRoot();
	/*UE_LOG(LogTemp, Warning, TEXT("USocketServerPluginTCPServer 1"));*/
}


void USocketServerTCP::startTCPServer(FIPandPortStruct ipStructP,FString IPPP, int32 portP, EReceiveFilterServer receiveFilterP, ESocketServerTCPSeparator messageWrappingP, FString serverIDP, bool isFileServer, FString Aes256bitKeyP, bool resumeFilesP) {
	ipAndPortStruct = ipStructP;
	serverPort = portP;
	receiveFilter = receiveFilterP;
	serverID = serverIDP;
	fileServer = isFileServer;
	aesKey = Aes256bitKeyP;
	resumeFiles = resumeFilesP;
	messageWrapping = messageWrappingP;
	USocketServerBPLibrary::socketServerBPLibrary->getTcpSeparator(tcpByteSeparator, tcpStringSeparator);
	(new FAutoDeleteAsyncTask<FSocketServerTCPThread>(this, receiveFilter,run))->StartBackgroundTask();
}


void USocketServerTCP::stopTCPServer() {



	TArray<FString> toRemoveSessionKeys;
	for (auto& element : getClientSessions()) {
		toRemoveSessionKeys.Add(element.Key);
	}
	for (int32 i = 0; i < toRemoveSessionKeys.Num(); i++) {
		removeClientSession(toRemoveSessionKeys[i]);
	}
	toRemoveSessionKeys.Empty();

	run = false;

}

void USocketServerTCP::sendTCPMessage(TArray<FString> clientSessionIDs, FString message, TArray<uint8> byteArray, bool addLineBreak){

	if (message.Len() > 0) {
		//if (messageWrapping == ESocketServerTCPSeparator::E_StringSeparator) {
		//	message = tcpMessageHeader + message + tcpMessageFooter;
		//}
		if (addLineBreak) {
			message.Append("\r\n");
		}
	}


	for (auto& sessionID : clientSessionIDs) {
		if (clientSessions.Find(sessionID) != nullptr) {
			FSocketServerPluginSession& session = *clientSessions.Find(sessionID);
			if (session.protocol == EServerSocketConnectionProtocol::E_TCP) {

				if (session.tcpSendThread == nullptr) {
					UE_LOG(LogTemp, Warning, TEXT("SimpleSocketServer Plugin: The thread for sending data has not yet been initialized. Data is not sent."));
				}
				else {
					session.tcpSendThread->sendMessage(message, byteArray);
				}
			}
		}
	}
}

void USocketServerTCP::sendTCPMessageToClient(FString clientSessionID, FString message, TArray<uint8> byteArray, bool addLineBreak) {

	if (message.Len() > 0) {
		//if (messageWrapping == ESocketServerTCPSeparator::E_StringSeparator) {
		//	message = tcpMessageHeader + message + tcpMessageFooter;
		//}
		if (addLineBreak) {
			message.Append("\r\n");
		}
	}

	if (clientSessions.Find(clientSessionID) != nullptr) {
		FSocketServerPluginSession& session = *clientSessions.Find(clientSessionID);
		if (session.protocol == EServerSocketConnectionProtocol::E_TCP) {

			if (session.tcpSendThread == nullptr) {
				UE_LOG(LogTemp, Warning, TEXT("SimpleSocketServer Plugin: The thread for sending data has not yet been initialized. Data is not sent."));
			}
			else {
				session.tcpSendThread->sendMessage(message, byteArray);
			}
		}
	}
	else {
		UE_LOG(LogTemp, Warning, TEXT("SimpleSocketServer Plugin: Session not found: %s"),*clientSessionID);
	}
}


FIPandPortStruct USocketServerTCP::getServerIpAndPortStruct() {
	return ipAndPortStruct;
}

FString USocketServerTCP::getIP() {
	return serverIP;
}

int32 USocketServerTCP::getPort() {
	return serverPort;
}

FString USocketServerTCP::getServerID() {
	return serverID;
}

bool USocketServerTCP::hasResume() {
	return resumeFiles;
}


void USocketServerTCP::initTCPClientThreads(FSocketServerPluginSession& sessionP, EReceiveFilterServer receiveFilterP){
	if (fileServer) {
		sessionP.tcpFileHandlerThread = new FSocketServerTCPFileHandlerThread(this, sessionP);
	}
	else {
		sessionP.tcpRecieverThread = new FSocketServerTCPClientReceiveDataThread(this, sessionP, receiveFilterP);
		sessionP.tcpSendThread = new FSocketServerTCPClientSendDataThread(this, sessionP);
	}

}

void USocketServerTCP::addClientSession(FSocketServerPluginSession& session) {
	if (session.sessionID.IsEmpty() == false) {
		//UE_LOG(LogTemp, Warning, TEXT("ADD Session:%s"), *session.sessionID);
		clientSessions.Add(session.sessionID, session);
	}
}

//FClientSocketSession USocketServerPluginTCPServer::getClientSession(FString key) {
//	if (clientSessions.Find(key) == nullptr)
//		return FClientSocketSession();
//	return *clientSessions.Find(key);
//}

void USocketServerTCP::removeClientSession(FString key) {
	//close client socket
	if (clientSessions.Find(key) != nullptr) {
		FSocketServerPluginSession& session = *clientSessions.Find(key);

		if (session.tcpRecieverThread != nullptr) {
			session.tcpRecieverThread->stopThread();
		}

		if (session.tcpSendThread != nullptr) {
			session.tcpSendThread->stopThread();
		}

		if (session.tcpFileHandlerThread != nullptr) {
			session.tcpFileHandlerThread->stopThread();
		}


		USocketServerBPLibrary::socketServerBPLibrary->cleanConnection(session);
	}
	if (clientSessions.Remove(key)) {
		//UE_LOG(LogTemp, Warning, TEXT("Remove3 Session:%s"), *key);
		USocketServerBPLibrary::socketServerBPLibrary->unregisterClientEvent(key);
	}
}

TMap<FString, FSocketServerPluginSession> USocketServerTCP::getClientSessions(){
	return clientSessions;
}

//EHTTPSocketServerFileDownloadResumeType USocketServerPluginTCPServer::getifFileExistThen(){
//	return ifFileExistThen;
//}



FString USocketServerTCP::encryptMessage(FString message) {
	return UFileFunctionsSocketServer::encryptMessageWithAES(message, aesKey);
}

FString USocketServerTCP::decryptMessage(FString message) {
	return UFileFunctionsSocketServer::decryptMessageWithAES(message, aesKey);
}

struct FSocketServerToken USocketServerTCP::getTokenStruct(FString token){
	FSocketServerToken sst;
	if (USocketServerBPLibrary::socketServerBPLibrary->fileTokenMap.Find(token) != nullptr) {
		sst = *USocketServerBPLibrary::socketServerBPLibrary->fileTokenMap.Find(token);
	}
	return sst;
}

void USocketServerTCP::removeTokenFromStruct(FString token) {
	if (USocketServerBPLibrary::socketServerBPLibrary->fileTokenMap.Find(token) != nullptr) {
		USocketServerBPLibrary::socketServerBPLibrary->fileTokenMap.Remove(token);
	}
}

FString USocketServerTCP::getCleanDir(EFileFunctionsSocketServerDirectoryType directoryType, FString fileDirectory) {
	return UFileFunctionsSocketServer::getCleanDirectory(directoryType, fileDirectory);
}

void USocketServerTCP::getMD5FromFile(FString filePathP, bool& success, FString& MD5) {
	UFileFunctionsSocketServer::getMD5FromFileAbsolutePath(filePathP, success, MD5);
}

void USocketServerTCP::deleteFile(FString filePathP) {
	UFileFunctionsSocketServer::deleteFileAbsolutePath(filePathP);
}

bool USocketServerTCP::isRun(){
	return run;
}

int64 USocketServerTCP::fileSize(FString filePathP) {
	return UFileFunctionsSocketServer::fileSizeAbsolutePath(filePathP);
}

FString USocketServerTCP::int64ToString(int64 num) {
	return UFileFunctionsSocketServer::int64ToString(num);
}

void USocketServerTCP::getTcpSeparator(FString& stringSeparator, uint8& byteSeparator, ESocketServerTCPSeparator& messageWrappingP) {
	messageWrappingP = messageWrapping;
	stringSeparator = tcpStringSeparator;
	tcpByteSeparator = byteSeparator;
}