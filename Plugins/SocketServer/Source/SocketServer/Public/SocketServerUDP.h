// Copyright 2017 David Romanski (Socke). All Rights Reserved.
#pragma once

#include "SocketServer.h"
#include "SocketServerUDPThread.h"
#include "SocketServerUDPClientSendDataThread.h"
#include "SocketServerUDP.generated.h"


class USocketServerBPLibrary;
class FSocketServerUDPThread;
class FSocketServerUDPClientSendDataThread;


UCLASS(Blueprintable, BlueprintType)
class SOCKETSERVER_API USocketServerUDP : public UObject
{
	GENERATED_UCLASS_BODY()

public:

	void startUDPServer(FIPandPortStruct ipStruct,FString IP, int32 port, bool multicast, EReceiveFilterServer receiveFilter, FString serverID, int32 maxPacketSize);
	void stopUDPServer();
	void sendUDPMessage(TArray<FString> clientSessionIDs, FString message, TArray<uint8> byteArray, bool asynchronous, ESocketServerUDPSocketType socketType);
	void sendUDPMessageToClient(FString clientSessionID, FString message, TArray<uint8> byteArray, bool asynchronous, ESocketServerUDPSocketType socketType);
	void sendUDPMessageTo(FString ip, int32 port, FString message, TArray<uint8> byteArray, bool asynchronous);
	//do not work with ipv6
	//void UDPReceiver(const FArrayReaderPtr& ArrayReaderPtr, const FIPv4Endpoint& EndPt);
	void UDPReceiverSocketServerPlugin(FArrayReaderPtr& ArrayReaderPtr, TSharedRef<FInternetAddr> remoteAddress);

	FIPandPortStruct getServerIpAndPortStruct();
	FString getIP();
	int32 getPort();
	void setSocketReceiver(FUdpSocketReceiver* socketReceiverP, FSocket* socket);
	FUdpSocketReceiver* getSocketReceiver();
	FSocket* getSocket();
	FString getServerID();

	void addClientSession(FSocketServerPluginSession& session);
	FSocketServerPluginSession* getClientSession(FString key);
	void removeClientSession(FString key);
	TMap<FString, FSocketServerPluginSession> getClientSessions();
	void sendBytes(FSocket*& socket, TArray<uint8>& bytes, int32& sent, TSharedRef<FInternetAddr>& addr);

private:

	FString serverID;
	FIPandPortStruct ipAndPortStruct;
	FString serverIP;
	int32  serverPort = -1;
	int32 maxPacketSize = 65507;
	FSocket* socket= nullptr;
	FUdpSocketReceiver* socketReceiver = nullptr;
	EReceiveFilterServer receiveFilter;
	FSocketServerUDPThread* serverThread = nullptr;
	FSocketServerUDPClientSendDataThread* sendThread = nullptr;

	TMap<FString, FSocketServerPluginSession> clientSessions;
};