// Copyright 2017-2022 David Romanski (Socke). All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "EngineGlobals.h"
#include "Engine/Engine.h"
#include "EventBean.h"
#include "Sockets.h"
#include "SocketSubsystem.h"
#include "Interfaces/IPv4/IPv4Endpoint.h"
#include "IPAddressAsyncResolve.h"
#include "Common/UdpSocketReceiver.h"
#include "Common/UdpSocketBuilder.h"
#include "Async/Async.h"
#include "Containers/Queue.h"
#include "Misc/DateTime.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "Misc/FileHelper.h"
#include "Misc/Paths.h"
#include "HAL/PlatformFileManager.h"
#include "HAL/FileManager.h"
#include "Misc/Base64.h"
#include "Misc/SecureHash.h"
#include "Misc/AES.h"
#include "Modules/ModuleManager.h"
#include "SocketServer.generated.h"

class FSocketServerTCPClientSendDataThread;
class FSocketServerTCPClientReceiveDataThread;
class FSocketServerTCPFileHandlerThread;

class FUdpSocketReceiver;
class FSocketServerUDPThread;
class FSocketServerUDPClientSendDataThread;


UENUM(BlueprintType)
enum class EFileFunctionsSocketServerEncodingOptions : uint8
{
	E_AutoDetect	UMETA(DisplayName = "AutoDetect"),
	E_ForceAnsi 	UMETA(DisplayName = "ForceAnsi"),
	E_ForceUnicode	UMETA(DisplayName = "ForceUnicode"),
	E_ForceUTF8 	UMETA(DisplayName = "ForceUTF8"),
	E_ForceUTF8WithoutBOM	UMETA(DisplayName = "ForceUTF8WithoutBOM")
};

UENUM(BlueprintType)
enum class EFileFunctionsSocketServerDirectoryType : uint8
{
	E_gd	UMETA(DisplayName = "Game directory"),
	E_ad 	UMETA(DisplayName = "Absolute directory")
};

UENUM(BlueprintType)
enum class EServerSocketConnectionEventType : uint8
{
	E_Server 	UMETA(DisplayName = "Server"),
	E_Client	UMETA(DisplayName = "Client")

};

UENUM(BlueprintType)
enum class EServerSocketConnectionProtocol : uint8
{
	E_NotSet 	UMETA(DisplayName = "NotSet"),
	E_TCP 		UMETA(DisplayName = "TCP"),
	E_UDP		UMETA(DisplayName = "UDP")

};

UENUM(BlueprintType)
enum class EServerSocketConnectionCheckPortType : uint8
{
	E_TCP 		UMETA(DisplayName = "TCP"),
	E_UDP		UMETA(DisplayName = "UDP")

};

UENUM(BlueprintType)
enum class EReceiveFilterServer : uint8
{
	E_SAB 	UMETA(DisplayName = "Message And Bytes"),
	E_S		UMETA(DisplayName = "Message"),
	E_B		UMETA(DisplayName = "Bytes")

};

UENUM(BlueprintType)
enum class ESocketPlatformServer : uint8
{
	E_SSS_SYSTEM		UMETA(DisplayName = "System"),
	E_SSS_DEFAULT 		UMETA(DisplayName = "Auto"),
	E_SSS_WINDOWS		UMETA(DisplayName = "WINDOWS"),
	E_SSS_MAC			UMETA(DisplayName = "MAC"),
	E_SSS_IOS			UMETA(DisplayName = "IOS"),
	E_SSS_UNIX			UMETA(DisplayName = "UNIX"),
	E_SSS_ANDROID		UMETA(DisplayName = "ANDROID"),
	E_SSS_PS4			UMETA(DisplayName = "PS4"),
	E_SSS_XBOXONE		UMETA(DisplayName = "XBOXONE"),
	E_SSS_HTML5			UMETA(DisplayName = "HTML5"),
	E_SSS_SWITCH		UMETA(DisplayName = "SWITCH")

};

UENUM(BlueprintType)
enum class ESocketServerUDPSocketType : uint8
{
	E_SSS_SERVER 	UMETA(DisplayName = "Use Server Socket"),
	E_SSS_CLIENT	UMETA(DisplayName = "Use Client Socket")

};

UENUM(BlueprintType)
enum class ESocketServerTCPSeparator : uint8
{
	E_None 				UMETA(DisplayName = "None"),
	E_ByteSeparator		UMETA(DisplayName = "Separate via one Byte"),
	E_StringSeparator	UMETA(DisplayName = "Separate via String"),
	E_LengthSeparator	UMETA(DisplayName = "Separate by Length")

};


UENUM(BlueprintType)
enum class ERCONPasswordType : uint8
{
	E_parameter	UMETA(DisplayName = "As String Parameter"),
	E_gd		UMETA(DisplayName = "As File in Game directory"),
	E_ad 		UMETA(DisplayName = "As File in Absolute directory")
};

USTRUCT()
struct FSendUDPMessageStruct {

	GENERATED_USTRUCT_BODY()

	FString			ip;
	int32			port;
	FString			message;
	TArray<uint8>	bytes;
	FSocket* socketUDP = nullptr;
};

USTRUCT(BlueprintType)
struct FFileFunctionsSocketServerOpenFile
{
	GENERATED_USTRUCT_BODY()

		FArchive* writer = nullptr;

};

USTRUCT()
struct FIPandPortStruct {

	GENERATED_USTRUCT_BODY()

	bool			success = false;
	FString			ip = FString();
	FString			errorMessage = FString();
	int32			port = 0;
};

USTRUCT(BlueprintType)
struct FSocketServerPluginSession
{
	GENERATED_USTRUCT_BODY()

	FString ip = FString();
	int32	port = 0;
	int64 addToCleanerTime = 0;
	FString sessionID = FString();
	FString serverID = FString();
	FSocket* socket = nullptr;

	FSocketServerTCPClientSendDataThread* tcpSendThread = nullptr;
	FSocketServerTCPClientReceiveDataThread* tcpRecieverThread = nullptr;
	FSocketServerTCPFileHandlerThread* tcpFileHandlerThread = nullptr;

	FUdpSocketReceiver* udpSocketReceiver = nullptr;
	FSocketServerUDPThread* udpServerThread = nullptr;
	FSocketServerUDPClientSendDataThread* udpSendThread = nullptr;

	EServerSocketConnectionProtocol protocol;
};


USTRUCT(BlueprintType)
struct FSocketServerDownloadFileInfo
{
	GENERATED_USTRUCT_BODY()

	float size;
	float megaBytesReceived;
	float megaBytesLeft;
	float percentDownload;
	float megaBit;
	FString fileName;
	FString serverID;
};

//USTRUCT(BlueprintType)
//struct FSocketServerUploadFileInfo
//{
//	GENERATED_USTRUCT_BODY()
//
//	float size;
//	float megaBytesSend;
//	float megaBytesLeft;
//	float percentUpload;
//	float megaBit;
//	FString fileName;
//	FString serverID;
//};

USTRUCT(BlueprintType)
struct FSocketServerToken
{
	GENERATED_USTRUCT_BODY()

	FString token = FString();
	bool deleteAfterUse = false;
	EFileFunctionsSocketServerDirectoryType directoryType;
	FString fileDirectory = FString();
};

#ifndef __FileFunctionsSocketServer
#define __FileFunctionsSocketServer
#include "FileFunctionsSocketServer.h"
#endif

#ifndef __RCONServer
#define __RCONServer
#include "RCONServer.h"
#endif

#ifndef __SocketServerCleanerThread
#define __SocketServerCleanerThread
#include "SocketServerCleanerThread.h"
#endif


#ifndef __SocketServerBPLibrary
#define __SocketServerBPLibrary
#include "SocketServerBPLibrary.h"
#endif

#ifndef __SocketServerUDP
#define __SocketServerUDP
#include "SocketServerUDP.h"
#endif

#ifndef __SocketServerTCP
#define __SocketServerTCP
#include "SocketServerTCP.h"
#endif



class FSocketServerModule : public IModuleInterface
{
public:

	/** IModuleInterface implementation */
	virtual void StartupModule() override;
	virtual void ShutdownModule() override;
};