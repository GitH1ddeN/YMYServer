// Copyright 2022 David Romanski (Socke). All Rights Reserved.

#include "SocketServerTCPClientSendFileToThread.h"


FSocketServerTCPClientSendFileToThread::FSocketServerTCPClientSendFileToThread(FSocket* socketP, FString filePathP, int64 fileSizeP, int64 startPositionP, FSocketServerPluginSession& sessionP) :
	socket(socketP),
	filePath(filePathP),
	fileSize(fileSizeP),
	startPosition(startPositionP),
	session(sessionP) {
	FString threadName = "FTCPClientSendFileToClientThread_" + FGuid::NewGuid().ToString();
	thread = FRunnableThread::Create(this, *threadName, 0, EThreadPriority::TPri_BelowNormal);
}

FSocketServerTCPClientSendFileToThread::~FSocketServerTCPClientSendFileToThread() {
	delete thread;
}

uint32 FSocketServerTCPClientSendFileToThread::Run() {

	if (socket == nullptr) {

		run = false;
		thread = nullptr;
		return 0;
	}

	FArchive* reader = IFileManager::Get().CreateFileReader(*filePath);
	if (reader == nullptr || reader->TotalSize() == 0) {
		if (reader != nullptr) {
			reader->Close();
		}
		delete reader;
		return false;
	}


	FString sessionID = session.sessionID;
	fileSize = reader->TotalSize();
	int64 readSize = 0;
	int64 lastPosition = startPosition;
	int64 bytesSentSinceLastTick = 0;

	int32 bufferSize = 1024 * 64;
	int32 dataSendBySocket = 0;

	float percent = 0.f;
	float mbit = 0.f;

	TArray<uint8> buffer;

	int64 lastTimeTicks = FDateTime::Now().GetTicks();

	if (bufferSize > fileSize) {
		bufferSize = fileSize;
	}

	if (lastPosition > 0) {
		reader->Seek(lastPosition);
	}

	while (run && lastPosition < fileSize) {

		if ((lastPosition + bufferSize) > fileSize) {
			bufferSize = fileSize - lastPosition;
		}

		//buffer.Reset(bufferSize);
		buffer.Empty();
		buffer.AddUninitialized(bufferSize);

		reader->Serialize(buffer.GetData(), buffer.Num());
		lastPosition += buffer.Num();

		//socket->Send((uint8*)((ANSICHAR*)Convert.Get()), Convert.Length(), dataSendBySocket);
		socket->Send(buffer.GetData(), buffer.Num(), dataSendBySocket);

		//slowdown for tests
		//FPlatformProcess::Sleep(0.01f);

		//fire event every second
		//one second = 10000000 ticks
		if (((FDateTime::Now().GetTicks()) - lastTimeTicks) >= 10000000) {
			mbit = ((float)lastPosition - (float)bytesSentSinceLastTick) / 1024 / 1024 * 8;
			lastTimeTicks = FDateTime::Now().GetTicks();
			bytesSentSinceLastTick = lastPosition;

			percent = ((float)lastPosition / fileSize) * 100;
			triggerFileOverTCPProgress(sessionID, filePath, percent, mbit, lastPosition, fileSize);
		}

	}

	if (lastPosition == fileSize) {

		mbit = ((float)lastPosition - (float)bytesSentSinceLastTick) / 1024 / 1024 * 8;
		percent = ((float)lastPosition / fileSize) * 100;

		triggerFileOverTCPProgress(sessionID, filePath, percent, mbit, lastPosition, fileSize);
	}
	else {
		triggerFileTransferOverTCPInfoEvent("Error while sending the file.", sessionID, filePath, false);
	}

	buffer.Empty();
	if (reader != nullptr) {
		reader->Close();
		reader = nullptr;
	}

	run = false;
	thread = nullptr;

	return 0;
}

void FSocketServerTCPClientSendFileToThread::triggerFileOverTCPProgress(FString sessionIDP, FString filePathP, float percentP, float mbitP, int64 bytesReceivedP, int64 fileSizeP) {
	AsyncTask(ENamedThreads::GameThread, [sessionIDP, filePathP, percentP, mbitP, bytesReceivedP, fileSizeP]() {
		USocketServerBPLibrary::socketServerBPLibrary->onfileTransferOverTCPProgressEventDelegate.Broadcast(sessionIDP, filePathP, percentP, mbitP, bytesReceivedP, fileSizeP);
		});
}

void FSocketServerTCPClientSendFileToThread::triggerFileTransferOverTCPInfoEvent(FString messageP, FString sessionIDP, FString filePathP, bool successP) {
	AsyncTask(ENamedThreads::GameThread, [messageP, sessionIDP, filePathP, successP]() {
		USocketServerBPLibrary::socketServerBPLibrary->onfileTransferOverTCPInfoEventDelegate.Broadcast(messageP, sessionIDP, filePathP, successP);
		});
}

void FSocketServerTCPClientSendFileToThread::stopThread() {
	run = false;
}

bool FSocketServerTCPClientSendFileToThread::isRun() {
	return run;
}