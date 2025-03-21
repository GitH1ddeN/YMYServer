// Fill out your copyright notice in the Description page of Project Settings.


#include "ToolkitFuncLib.h"
#include "SocketSubsystem.h"

FString UToolkitFuncLib::GetLocalIPAddress()
{
	FString IPAddress;

	if (ISocketSubsystem* SocketSubsystem = ISocketSubsystem::Get(PLATFORM_SOCKETSUBSYSTEM))
	{
		/*FString HostName;
		SocketSubsystem->GetHostName(HostName);*/

		TArray<TSharedPtr<FInternetAddr>> Address;
		SocketSubsystem->GetLocalAdapterAddresses(Address);

		for (const TSharedPtr<FInternetAddr>& Addr : Address)
		{
			IPAddress = Addr->ToString(false);
		}
	}

	return IPAddress;
}

bool UToolkitFuncLib::ReadFile(const FString& FilePath, FString& FileContent)
{
	return FFileHelper::LoadFileToString(FileContent, *FilePath);
}

bool UToolkitFuncLib::GetJsonFieldAndValue(const FString& JsonString, FString& JsonField, FString& JsonValue)
{
	TSharedRef<TJsonReader<>> JsonReader = TJsonReaderFactory<>::Create(JsonString);
	TSharedPtr<FJsonObject> JsonObject;

	bool bSuccess = FJsonSerializer::Deserialize(JsonReader, JsonObject);

	if (bSuccess)
	{
		for (auto Value : JsonObject->Values)
		{
			JsonField = Value.Key;
			JsonValue = Value.Value->AsString();
			break;
		}
	}

	return bSuccess;
}

int64 UToolkitFuncLib::Get13Timestamp()
{
	// 获取当前的时间点
	FDateTime CurrentTime = FDateTime::UtcNow();
	// 获取从1970年1月1日00:00:00 UTC到当前时间点所经过的总时间
	FDateTime Epoch(1970, 1, 1);
	FTimespan TimeSinceEpoch = CurrentTime - Epoch;
	// 将总时间转换为毫秒数
	return TimeSinceEpoch.GetTotalMilliseconds();
}
