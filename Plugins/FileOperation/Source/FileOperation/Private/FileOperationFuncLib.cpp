// Fill out your copyright notice in the Description page of Project Settings.
#include "FileOperationFuncLib.h"
#include "Runtime/Core/Public/HAL/FileManagerGeneric.h"
#include "Runtime/Core/Public/Misc/FileHelper.h"
#include "Engine/TextureRenderTarget2D.h"
#include "Runtime/Core/Public/Misc/Paths.h"

//If this function cannot find or create the directory, returns false.
bool UFileOperationFuncLib::VerifyOrCreateDirectory(const FString& filePath)
{
	// Every function call, unless the function is inline, adds a small
	// overhead which we can avoid by creating a local variable like so.
	// But beware of making every function inline!
	IPlatformFile& PlatformFile = FPlatformFileManager::Get().GetPlatformFile();

	// Directory Exists?
	if (!PlatformFile.DirectoryExists(*filePath))
	{
		PlatformFile.CreateDirectory(*filePath);

		if (!PlatformFile.DirectoryExists(*filePath))
		{
			return false;
		}
	}
	return true;
}

bool UFileOperationFuncLib::VerifyDirectoryExists(const FString& filePath)
{
	IPlatformFile& PlatformFile = FPlatformFileManager::Get().GetPlatformFile();

	return PlatformFile.DirectoryExists(*filePath);
}

bool UFileOperationFuncLib::DeleteDirectory(const FString& filePath)
{
	IPlatformFile& PlatformFile = FPlatformFileManager::Get().GetPlatformFile();

	if (!PlatformFile.DirectoryExists(*filePath))
	{
		return false;
	}
	return PlatformFile.DeleteDirectory(*filePath);
}

bool UFileOperationFuncLib::CreateDirectoryTree(const FString& filePath)
{
	IPlatformFile& PlatformFile = FPlatformFileManager::Get().GetPlatformFile();

	return PlatformFile.CreateDirectoryTree(*filePath);
}

TArray<FString> UFileOperationFuncLib::FindFiles(const FString& Path, const FString& Filter)
{
	TArray<FString> FilePathList;
	FilePathList.Empty();
	FFileManagerGeneric::Get().FindFiles(FilePathList, *Path, *Filter);
	return FilePathList;
}

TArray<FString> UFileOperationFuncLib::FindFilesRecursive(const FString& Path, const FString& Filter, bool Files, bool Directory)
{
	TArray<FString> FilePathList;
	FilePathList.Empty();
	FFileManagerGeneric::Get().FindFilesRecursive(FilePathList, *Path, *Filter, Files, Directory);
	return FilePathList;
}

bool UFileOperationFuncLib::VerifyFileExists(const FString& filePath)
{
	IPlatformFile& PlatformFile = FPlatformFileManager::Get().GetPlatformFile();

	return PlatformFile.FileExists(*filePath);
}

bool UFileOperationFuncLib::CopyFileTo(const FString& to, const FString& from)
{
	int copied = IFileManager::Get().Copy(*to, *from, true);

	if (copied == 1)
		return true;

	return false;
}

bool UFileOperationFuncLib::MoveFileTo(const FString& to, const FString& from)
{
	return IFileManager::Get().Move(*to, *from);
}

bool UFileOperationFuncLib::DeleteFile(const FString& filePath)
{
	return IFileManager::Get().Delete(*filePath);
}

bool UFileOperationFuncLib::ReadFileToString(const FString& filePath, FString& result)
{
	return FFileHelper::LoadFileToString(result, *filePath);
}

bool UFileOperationFuncLib::ReadFileToStringArray(const FString& filePath, TArray<FString>& outArr)
{
	return FFileHelper::LoadFileToStringArray(outArr, *filePath);;
}

bool UFileOperationFuncLib::ReadFileToBytes(TArray<uint8>& bytes, const FString& filePath)
{
	return FFileHelper::LoadFileToArray(bytes, *filePath);
}

bool UFileOperationFuncLib::WriteStringToFile(const FString& filePath, const FString& content)
{
	return FFileHelper::SaveStringToFile(content, *filePath);
}

bool UFileOperationFuncLib::WriteStringArrayToFile(const FString& filePath, const TArray<FString>& lines)
{
	return FFileHelper::SaveStringArrayToFile(lines, *filePath);
}

bool UFileOperationFuncLib::WriteBytesToFile(const TArray<uint8>& bytes, const FString& fileName)
{
	return FFileHelper::SaveArrayToFile(bytes, *fileName);
}

FString UFileOperationFuncLib::GetRootDir()
{
	return FPaths::ConvertRelativePathToFull(FPaths::RootDir());
}

FString UFileOperationFuncLib::GetProjectDir()
{
	return FPaths::ConvertRelativePathToFull(FPaths::ProjectDir());
}

FString UFileOperationFuncLib::GetContentDir()
{
	return FPaths::ConvertRelativePathToFull(FPaths::ProjectContentDir());
}

FString UFileOperationFuncLib::GetSavedDir()
{
	return FPaths::ConvertRelativePathToFull(FPaths::ProjectSavedDir());
}

void UFileOperationFuncLib::SplitPath(const FString& InPath, FString& PathPart, FString& FilenamePart, FString& ExtensionPart)
{
	FPaths::Split(*InPath, PathPart, FilenamePart, ExtensionPart);
}

bool UFileOperationFuncLib::SaveRenderTargetToFile(UTextureRenderTarget2D* rt, const FString& fileDestination)
{
	FTextureRenderTargetResource* rtResource = rt->GameThread_GetRenderTargetResource();
	FReadSurfaceDataFlags readPixelFlags(RCM_UNorm);

	TArray<FColor> outBMP;
	outBMP.AddUninitialized(rt->GetSurfaceWidth() * rt->GetSurfaceHeight());
	rtResource->ReadPixels(outBMP, readPixelFlags);

	for (FColor& color : outBMP)
		color.A = 255;

	FIntPoint destSize(rt->GetSurfaceWidth(), rt->GetSurfaceHeight());
	TArray<uint8> CompressedBitmap;
	FImageUtils::CompressImageArray(destSize.X, destSize.Y, outBMP, CompressedBitmap);
	bool imageSavedOk = FFileHelper::SaveArrayToFile(CompressedBitmap, *fileDestination);
	return imageSavedOk;

	return true;
}

//
//FString UFileOperationFuncLib::Combine(TArray<FString> InPaths)
//{
//	return FPaths::Combine(InPaths);
//}
