// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "Runtime/Engine/Public/ImageUtils.h"
#include "FileOperationFuncLib.generated.h"

/**
 *
 */
UCLASS()
class FILEOPERATION_API UFileOperationFuncLib : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable, Category = "File operation")
		static bool VerifyOrCreateDirectory(const FString& filePath);

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "File operation")
		static bool VerifyDirectoryExists(const FString& filePath);

	UFUNCTION(BlueprintCallable, Category = "File operation")
		static bool DeleteDirectory(const FString& filePath);

	UFUNCTION(BlueprintCallable, Category = "File operation")
		static bool CreateDirectoryTree(const FString& filePath);

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "File operation")
		static TArray<FString> FindFiles(const FString& Path, const FString& Filter);

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "File operation")
		static TArray<FString> FindFilesRecursive(const FString& Path, const FString& Filter, bool Files, bool Directory);

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "File operation")
		static bool VerifyFileExists(const FString& filePath);

	UFUNCTION(BlueprintCallable, Category = "File operation")
		static bool MoveFileTo(const FString& to, const FString& from);

	UFUNCTION(BlueprintCallable, Category = "File operation")
		static bool CopyFileTo(const FString& to, const FString& from);

	UFUNCTION(BlueprintCallable, Category = "File operation")
		static bool DeleteFile(const FString& filePath);

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "File operation")
		static bool ReadFileToString(const FString& filePath, FString& result);

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "File operation")
		static bool ReadFileToStringArray(const FString& filePath,TArray<FString>& outArr);

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "File operation")
		static bool ReadFileToBytes(TArray<uint8>& bytes, const FString& filePath);

	UFUNCTION(BlueprintCallable, Category = "File operation")
		static bool WriteStringToFile(const FString& filePath, const FString& content);

	UFUNCTION(BlueprintCallable, Category = "File operation")
		static bool WriteStringArrayToFile(const FString& filePath, const TArray<FString>& lines);

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "File operation")
		static bool WriteBytesToFile(const TArray<uint8>& bytes, const FString& fileName);

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "File operation")
		static FString GetRootDir();

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "File operation")
		static FString GetProjectDir();

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "File operation")
		static FString GetContentDir();

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "File operation")
		static FString GetSavedDir();

	//分割路径,根据给定分隔符 ExtensionPart 将InPath 分割为两部分，分别赋值给参数PathPart和FileNamePart,可以理解为左右两部分
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "File operation")
		static void SplitPath(const FString& InPath, FString& PathPart, FString& FilenamePart, FString& ExtensionPart);

	//将输入的多个路径拼接成一个路径,可以拼接字符串,类似FString中的字符串拼接
	/*UFUNCTION(BlueprintCallable, Category = "File operation")
		static FString Combine(TArray<FString> InPaths);*/

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "File operation")
		static bool GetConfigInt(const FString& Category, const FString& Name, int32& value)
	{
		return GConfig->GetInt(*Category, *Name, value, GGameIni);
	}

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "File operation")
		static bool GetConfigBool(const FString& Category, const FString& Name, bool& value)
	{
		return GConfig->GetBool(*Category, *Name, value, GGameIni);
	}

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "File operation")
		static bool GetConfigFloat(const FString& Category, const FString& Name, float& value)
	{
		return GConfig->GetFloat(*Category, *Name, value, GGameIni);
	}

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "File operation")
		static bool GetConfigString(const FString& Category, const FString& Name, FString& value)
	{
		return GConfig->GetString(*Category, *Name, value, GGameIni);
	}

	UFUNCTION(BlueprintCallable, Category = "File operation")
		static void SetConfigInt(const FString& Category, const FString& Name, int32 value)
	{
		GConfig->SetInt(*Category, *Name, value, GGameIni);
	}

	UFUNCTION(BlueprintCallable, Category = "File operation")
		static void SetConfigBool(const FString& Category, const FString& Name, bool value)
	{
		GConfig->SetBool(*Category, *Name, value, GGameIni);
	}

	UFUNCTION(BlueprintCallable, Category = "File operation")
		static void SetConfigFloat(const FString& Category, const FString& Name, float value)
	{
		GConfig->SetFloat(*Category, *Name, value, GGameIni);
	}

	UFUNCTION(BlueprintCallable, Category = "File operation")
		static void SetConfigString(const FString& Category, const FString& Name, const FString& value)
	{
		GConfig->SetString(*Category, *Name, *value, GGameIni);
	}

	//保存UTextureRenderTarget2D到本地文件
	UFUNCTION(BlueprintCallable, meta = (DisplayName = "SaveRenderTargetToFile", Keywords = "SaveRenderTargetToFile"), Category = "SaveToFile")
		static bool SaveRenderTargetToFile(UTextureRenderTarget2D* rt, const FString& fileDestination);
};
