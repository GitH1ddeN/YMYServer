// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "Json.h"
#include "JsonUtilities.h"
#include "DataType.h"
#include"JsonObjectConverter.h"
#include "ToolkitFuncLib.generated.h"

/**
 *
 */
UCLASS()
class LOCATIONMANAGEMENT_API UToolkitFuncLib : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()
public:

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "CustomData")
		static FString FClientConfigToJson(FClientConfig data)
	{
		FString str = "";
		FJsonObjectConverter::UStructToJsonObjectString(data, str, 0, 0);
		return str;
	}

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "CustomData")
		static FClientConfig JsonToFClientConfig(FString str)
	{

		FClientConfig result;
		TSharedRef< TJsonReader<> >Reader = TJsonReaderFactory<>::Create(str);

		TSharedPtr<FJsonObject>Object;
		bool bSuccessful = FJsonSerializer::Deserialize(Reader, Object);
		if (bSuccessful)
		{
			FClientConfig data;
			bool cSuccess = FJsonObjectConverter::JsonObjectStringToUStruct(str, &data, 0, 0);
			if (cSuccess)
			{
				result = data;
			}
		}
		return result;
	}

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "CustomData")
		static FString FClientStateToJson(FClientState data)
	{
		FString str = "";
		FJsonObjectConverter::UStructToJsonObjectString(data, str, 0, 0);
		return str;
	}

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "CustomData")
		static FClientState JsonToFClientState(FString str)
	{

		FClientState result;
		TSharedRef< TJsonReader<> >Reader = TJsonReaderFactory<>::Create(str);

		TSharedPtr<FJsonObject>Object;
		bool bSuccessful = FJsonSerializer::Deserialize(Reader, Object);
		if (bSuccessful)
		{
			FClientState data;
			bool cSuccess = FJsonObjectConverter::JsonObjectStringToUStruct(str, &data, 0, 0);
			if (cSuccess)
			{
				result = data;
			}
		}
		return result;
	}

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "CustomData")
		static FString FClientSyncInfoToJson(FClientSyncInfo data)
	{
		FString str = "";
		FJsonObjectConverter::UStructToJsonObjectString(data, str, 0, 0);
		return str;
	}

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "CustomData")
		static FClientSyncInfo JsonToFClientSyncInfo(FString str)
	{

		FClientSyncInfo result;
		TSharedRef< TJsonReader<> >Reader = TJsonReaderFactory<>::Create(str);

		TSharedPtr<FJsonObject>Object;
		bool bSuccessful = FJsonSerializer::Deserialize(Reader, Object);
		if (bSuccessful)
		{
			FClientSyncInfo data;
			bool cSuccess = FJsonObjectConverter::JsonObjectStringToUStruct(str, &data, 0, 0);
			if (cSuccess)
			{
				result = data;
			}
		}
		return result;
	}


	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "CustomData")
		static FString FServerCommandToJson(FServerCommand data)
	{
		FString str = "";
		FJsonObjectConverter::UStructToJsonObjectString(data, str, 0, 0);
		return str;
	}

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "CustomData")
		static FServerCommand JsonToFServerCommand(FString str)
	{

		FServerCommand result;
		TSharedRef< TJsonReader<> >Reader = TJsonReaderFactory<>::Create(str);

		TSharedPtr<FJsonObject>Object;
		bool bSuccessful = FJsonSerializer::Deserialize(Reader, Object);
		if (bSuccessful)
		{
			FServerCommand data;
			bool cSuccess = FJsonObjectConverter::JsonObjectStringToUStruct(str, &data, 0, 0);
			if (cSuccess)
			{
				result = data;
			}
		}
		return result;
	}


	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "CustomData")
		static FString FClientSessionInfoToJson(FClientSessionInfo data)
	{
		FString str = "";
		FJsonObjectConverter::UStructToJsonObjectString(data, str, 0, 0);
		return str;
	}

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "CustomData")
		static FClientSessionInfo JsonToFClientSessionInfo(FString str)
	{

		FClientSessionInfo result;
		TSharedRef< TJsonReader<> >Reader = TJsonReaderFactory<>::Create(str);

		TSharedPtr<FJsonObject>Object;
		bool bSuccessful = FJsonSerializer::Deserialize(Reader, Object);
		if (bSuccessful)
		{
			FClientSessionInfo data;
			bool cSuccess = FJsonObjectConverter::JsonObjectStringToUStruct(str, &data, 0, 0);
			if (cSuccess)
			{
				result = data;
			}
		}
		return result;
	}

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "CustomData")
		static FString FCreateSessionInfoToJson(FCreateSessionInfo data)
	{
		FString str = "";
		FJsonObjectConverter::UStructToJsonObjectString(data, str, 0, 0);
		return str;
	}

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "CustomData")
		static FCreateSessionInfo JsonToFCreateSessionInfo(FString str)
	{

		FCreateSessionInfo result;
		TSharedRef< TJsonReader<> >Reader = TJsonReaderFactory<>::Create(str);

		TSharedPtr<FJsonObject>Object;
		bool bSuccessful = FJsonSerializer::Deserialize(Reader, Object);
		if (bSuccessful)
		{
			FCreateSessionInfo data;
			bool cSuccess = FJsonObjectConverter::JsonObjectStringToUStruct(str, &data, 0, 0);
			if (cSuccess)
			{
				result = data;
			}
		}
		return result;
	}

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "CustomData")
		static FString FSessionSystemInfoToJson(FSessionSystemInfo data)
	{
		FString str = "";
		FJsonObjectConverter::UStructToJsonObjectString(data, str, 0, 0);
		return str;
	}

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "CustomData")
		static FSessionSystemInfo JsonToFSessionSystemInfo(FString str)
	{

		FSessionSystemInfo result;
		TSharedRef< TJsonReader<> >Reader = TJsonReaderFactory<>::Create(str);

		TSharedPtr<FJsonObject>Object;
		bool bSuccessful = FJsonSerializer::Deserialize(Reader, Object);
		if (bSuccessful)
		{
			FSessionSystemInfo data;
			bool cSuccess = FJsonObjectConverter::JsonObjectStringToUStruct(str, &data, 0, 0);
			if (cSuccess)
			{
				result = data;
			}
		}
		return result;
	}

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "CustomData")
		static FString FWholeClientsLocationsToJson(FWholeClientsLocations data)
	{
		FString str = "";
		FJsonObjectConverter::UStructToJsonObjectString(data, str, 0, 0);
		return str;
	}

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "CustomData")
		static FWholeClientsLocations JsonToFWholeClientsLocations(FString str)
	{

		FWholeClientsLocations result;
		TSharedRef< TJsonReader<> >Reader = TJsonReaderFactory<>::Create(str);

		TSharedPtr<FJsonObject>Object;
		bool bSuccessful = FJsonSerializer::Deserialize(Reader, Object);
		if (bSuccessful)
		{
			FWholeClientsLocations data;
			bool cSuccess = FJsonObjectConverter::JsonObjectStringToUStruct(str, &data, 0, 0);
			if (cSuccess)
			{
				result = data;
			}
		}
		return result;
	}

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Random String")
		static FString GenerateRandStrWithTime(int32 Length)
	{
		const FString Characters = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789";
		FString RandomString;

		for (int32 i = 0; i < Length; ++i)
		{
			int32 RandomIndex = FMath::RandRange(0, Characters.Len() - 1);
			RandomString.AppendChar(Characters[RandomIndex]);
		}

		// Get current time
		FDateTime CurrentTime = FDateTime::UtcNow();
		FString Timestamp = CurrentTime.ToString(TEXT("%y%m%d%H%M%S"));

		// Combine random string with timestamp
		FString Result = RandomString + "_" + Timestamp;

		return Result;
	}

	UFUNCTION(BlueprintPure, meta = (DisplayName = "ToRotator", CompactNodeTitle = "->", BlueprintAutocast), Category = "Utilities|Transform")
		static FRotator Conv_VectorToRotator(FVector InData)
	{
		return FRotator(InData.Y, InData.Z, InData.X);
	}

	UFUNCTION(BlueprintPure, meta = (DisplayName = "ToVector", CompactNodeTitle = "->", BlueprintAutocast), Category = "Utilities|Transform")
		static FVector Conv_RotatorToVector(FRotator InData)
	{
		return FVector(InData.Roll, InData.Pitch, InData.Yaw);
	}

	UFUNCTION(BlueprintCallable, BlueprintPure, Category ="Utilities|Sockets")
	static FString GetLocalIPAddress();

	/**
	* 读取文件内容
	*
	* @param		FilePath		文件路径
	* @param		FileContent		文本内容
	* @return						返回读取是否成功
	*/
	UFUNCTION(BlueprintCallable, Category = "FileOperation")
	static bool ReadFile(const FString& FilePath, FString& FileContent);

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "FileOperation")
	static bool GetJsonFieldAndValue(const FString& JsonString, FString& JsonField, FString& JsonValue);
};
