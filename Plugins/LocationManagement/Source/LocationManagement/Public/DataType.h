// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "DataType.generated.h"

/**
 *
 */
class LOCATIONMANAGEMENT_API DataType
{
public:
	DataType();
	~DataType();
};

USTRUCT(BlueprintType)
struct FClientConfig
{
	GENERATED_BODY()
public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		FString Mac;
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		FString DeviceID;
};

USTRUCT(BlueprintType)
struct FClientState
{
	GENERATED_BODY()
public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		FString DeviceID;
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		int GameState;	///-1:Offline; 0:Idle; 1:Running
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		FString SessionID;
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		FString UserName;
};

USTRUCT(BlueprintType)
struct FClientSyncInfo
{
	GENERATED_BODY()
public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		FString DeviceID;
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		FVector ClientPosition;
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		FVector ClientRotation;
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		int EventID;
};

USTRUCT(BlueprintType)
struct FServerCommand
{
	GENERATED_BODY()
public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		FString ClientDeviceID;
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		int CommandID;
};

USTRUCT(BlueprintType)
struct FClientSessionInfo
{
	GENERATED_BODY()
public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		FString SessionID;
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		TArray<FString> ClientIDList;
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		FString HomeClientID;
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		int StateID;
};

USTRUCT(BlueprintType)
struct FWholeClientsLocations
{
	GENERATED_BODY()
public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		TArray<FString> ClientIDList;
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		TArray<FVector> ClientPositionList;
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		TArray<FVector> ClientRotationList;
};

USTRUCT(BlueprintType)
struct FCreateSessionInfo
{
	GENERATED_BODY()
public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		FString machineid;
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		FString session_location_topic;
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		FString session_event_topic;
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		FString session_event_system_topic;
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		FString exename;
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		FString clientname;
};

USTRUCT(BlueprintType)
struct FSessionSystemInfo
{
	GENERATED_BODY()
public:	
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		TArray<FString> ClientIDList;
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		TArray<FString> ClientNameList;
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		TArray<FString> ClientAvatarList;
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		int EventID;
};