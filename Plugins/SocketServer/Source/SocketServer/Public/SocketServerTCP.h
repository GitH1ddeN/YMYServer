// Copyright 2017-2020 David Romanski (Socke). All Rights Reserved.
#pragma once

#include "SocketServer.h"
#include "SocketServerTCPThread.h"
#include "SocketServerTCPClientSendDataThread.h"
#include "SocketServerTCPClientReceiveDataThread.h"
#include "SocketServerTCPClientSendFileToThread.h"
#include "SocketServerTCPFileHandlerThread.h"
#include "SocketServerTCP.generated.h"


class USocketServerBPLibrary;


UCLASS(Blueprintable, BlueprintType)
class SOCKETSERVER_API USocketServerTCP : public UObject
{
	GENERATED_UCLASS_BODY()

public:

	void startTCPServer(FIPandPortStruct ipStructP,FString IP, int32 port, EReceiveFilterServer receiveFilter, ESocketServerTCPSeparator messageWrapping, FString serverID, bool isFileServer, FString Aes256bitKey, bool resumeFiles);
	void stopTCPServer();
	void sendTCPMessage(TArray<FString> clientSessionIDs, FString message, TArray<uint8> byteArray, bool addLineBreak);
	void sendTCPMessageToClient(FString clientSessionID, FString message, TArray<uint8> byteArray, bool addLineBreak);

	FIPandPortStruct getServerIpAndPortStruct();
	FString getIP();
	int32 getPort();
	FString getServerID();
	bool hasResume();

	void initTCPClientThreads(FSocketServerPluginSession& session, EReceiveFilterServer receiveFilter);

	void addClientSession(FSocketServerPluginSession& session);
	FSocketServerPluginSession getClientSession(FString key);
	void removeClientSession(FString key);
	TMap<FString, FSocketServerPluginSession> getClientSessions();
	//EHTTPSocketServerFileDownloadResumeType getifFileExistThen();


	FString encryptMessage(FString message);
	FString decryptMessage(FString message);

	struct FSocketServerToken getTokenStruct(FString token);
	void removeTokenFromStruct(FString token);
	FString getCleanDir(EFileFunctionsSocketServerDirectoryType directoryType, FString fileDirectory);
	void getMD5FromFile(FString filePathP, bool& success, FString& MD5);
	void deleteFile(FString filePathP);
	int64 fileSize(FString filePathP);
	FString int64ToString(int64 num);
	void getTcpSeparator(FString& stringSeparator, uint8& byteSeparator, ESocketServerTCPSeparator& messageWrapping);
	bool isRun();

	TMap<FString, FSocketServerPluginSession> clientSessions;

private:
	bool run = true;
	FString serverID = FString();
	FIPandPortStruct ipAndPortStruct;
	FString serverIP = FString();
	int32  serverPort = -1;
	EReceiveFilterServer receiveFilter;
	bool fileServer = false;
	FString aesKey = FString();
	bool resumeFiles = false;
	//FString downloadDir;

	ESocketServerTCPSeparator messageWrapping;
	FString tcpStringSeparator = "(~{";
	uint8 tcpByteSeparator = 0x00;

};

