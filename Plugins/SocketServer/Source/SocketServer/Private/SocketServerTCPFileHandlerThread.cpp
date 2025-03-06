// Copyright 2022 David Romanski (Socke). All Rights Reserved.

#include "SocketServerTCPFileHandlerThread.h"

FSocketServerTCPFileHandlerThread::FSocketServerTCPFileHandlerThread(USocketServerTCP* tcpServerP, FSocketServerPluginSession& sessionP) :
	tcpServer(tcpServerP),
	session(sessionP) {
	FString threadName = "FTCPFileHandlerThread" + FGuid::NewGuid().ToString();
	thread = FRunnableThread::Create(this, *threadName, 0, EThreadPriority::TPri_BelowNormal);
}

FSocketServerTCPFileHandlerThread::~FSocketServerTCPFileHandlerThread() {
	delete thread;
}

uint32 FSocketServerTCPFileHandlerThread::Run() {
	FString serverID = tcpServer->getServerID();
	FSocket* clientSocket = session.socket;
	FString sessionID = session.sessionID;


	AsyncTask(ENamedThreads::GameThread, [sessionID, serverID]() {
		USocketServerBPLibrary::socketServerBPLibrary->onsocketServerConnectionEventDelegate.Broadcast(EServerSocketConnectionEventType::E_Client, true, "Client connected", sessionID, serverID);
		});

	float sleepTime = 0.1;
	int64 ticksDownload = FDateTime::Now().GetTicks();
	int64 lastByte = 0;
	int64 bytesDownloaded = 0;
	FArchive* writer = nullptr;
	uint32 DataSize;
	TArray<uint8> recvDataArray;
	int64 ticks1;
	int64 ticks2;
	FString md5Client = FString();
	FString fullFilePath = FString();
	FString token = FString();
	TArray<uint8> byteArray;
	FString recvMessage;
	bool isConnected = true;
	while (run && clientSocket != nullptr && tcpServer->clientSessions.Find(sessionID) != nullptr) {
		//ESocketConnectionState::SCS_Connected does not work https://issues.unrealengine.com/issue/UE-27542
		//Compare ticks is a workaround to get a disconnect. clientSocket->Wait() stop working after disconnect. (Another bug?)
		//If it doesn't wait any longer, ticks1 and ticks2 should be the same == disconnect.
		ticks1 = FDateTime::Now().GetTicks();
		clientSocket->Wait(ESocketWaitConditions::WaitForRead, FTimespan::FromSeconds(5));
		ticks2 = FDateTime::Now().GetTicks();
		bool hasData = clientSocket->HasPendingData(DataSize);
		if (!hasData) {
			if (ticks1 == ticks2) {
				break;//disconnect
			}
			else {
				if (sendFileThread == nullptr) {
					triggerFileTransferOverTCPInfoEvent("Timeout", sessionID, fullFilePath, false);
					break;//timeout
				}
			}
		}
		if (hasData) {
			int32 bytesRead = 0;

			switch (commandProgress)
			{
			case 0:
				recvDataArray.SetNumUninitialized(DataSize);
				if (clientSocket->Recv(recvDataArray.GetData(), recvDataArray.Num(), bytesRead)) {

					recvDataArray.Add(0x00);// null-terminator
					recvMessage = FString(UTF8_TO_TCHAR(recvDataArray.GetData()));
					recvMessage = (tcpServer->decryptMessage(recvMessage)).TrimStartAndEnd();
					/*UE_LOG(LogTemp, Display, TEXT("TCP End xxx: %s: "), *recvMessage);*/
					TArray<FString> lines;
					recvMessage.ParseIntoArray(lines, TEXT("_|_"), true);


					if (lines.Num() == 5) {
						//client send data to this server
						if (lines[0].Equals("SEND_FILE_TO_SERVER") && lines[1].Len() > 0 && lines[2].Len() > 0 && lines[3].Len() > 0 && lines[4].Len() > 0) {

							md5Client = lines[2];

							token = lines[1];
							struct FSocketServerToken tokenStruct = tcpServer->getTokenStruct(token);



							if (tokenStruct.token.IsEmpty() && tokenStruct.token.Equals(token) == false) {
								triggerFileTransferOverTCPInfoEvent("Token not found.", sessionID, fullFilePath, false);
								run = false;
								break;
							}

							if (tokenStruct.deleteAfterUse) {
								tcpServer->removeTokenFromStruct(token);
							}

							FString downloadDir = tcpServer->getCleanDir(tokenStruct.directoryType, tokenStruct.fileDirectory);
							FString fileName = lines[3];
							if (downloadDir.EndsWith("/")) {
								fullFilePath = downloadDir + fileName;
							}
							else {
								fullFilePath = downloadDir + "/" + fileName;
							}


							if (md5Client.IsEmpty()) {
								triggerFileTransferOverTCPInfoEvent("MD5 string from client is missing.", sessionID, fullFilePath, false);
								run = false;
								break;
							}





							if (FPaths::DirectoryExists(downloadDir) == false) {
								triggerFileTransferOverTCPInfoEvent("Directory not found.", sessionID, fullFilePath, false);
								run = false;
								break;
							}


							fileSize = FCString::Atoi64(*lines[4]);


							if (fileSize <= 0) {
								triggerFileTransferOverTCPInfoEvent("Client has reported a file size of 0 or lower.", sessionID, fullFilePath, false);
								run = false;
								break;
							}

							/*							if (tcpServer->hasResume() && lines[4].Equals("0") == false) {
															if (FPaths::FileExists(fullFilePath)) {
																lastByte = FCString::Atoi64(*lines[4]);
															}
														}*/

							if (tcpServer->hasResume()) {
								writer = IFileManager::Get().CreateFileWriter(*fullFilePath, EFileWrite::FILEWRITE_Append);
							}
							else {
								writer = IFileManager::Get().CreateFileWriter(*fullFilePath);
							}





							if (!writer) {
								triggerFileTransferOverTCPInfoEvent("Can't create file.", sessionID, fullFilePath, false);
								run = false;
								break;
							}



							bytesDownloaded = writer->TotalSize();


							if (bytesDownloaded > fileSize) {
								triggerFileTransferOverTCPInfoEvent("File on server bigger than on client. Cancel.", sessionID, fullFilePath, false);
								run = false;
								break;
							}

							if (bytesDownloaded == fileSize) {
								writer->Close();
								FPlatformProcess::Sleep(1);
								sendEndMessage(fullFilePath, token, md5Client, sessionID, clientSocket);
								run = false;
								break;
							}
							else {
								FPlatformProcess::Sleep(sleepTime);

								commandProgress = 1;
								FString response = "SEND_FILE_TO_SERVER_ACCEPTED_|_" + token + "_|_" + FString::FromInt(bytesDownloaded) + "\r\n";
								response = tcpServer->encryptMessage(response);

								FTCHARToUTF8 Convert(*response);
								int32 bytesSendOverSocket = 0;
								clientSocket->Send((uint8*)Convert.Get(), Convert.Length(), bytesSendOverSocket);

							}
						}
						else {
							triggerFileTransferOverTCPInfoEvent("File request incorrect (0).", sessionID, fullFilePath, false);

							recvMessage.Empty();
							recvDataArray.Empty();
							byteArray.Empty();

							run = false;
							break;
						}
					}
					else {
						if (lines.Num() == 2) {
							//client request file
							if (lines[0].Equals("REQUEST_FILE_FROM_SERVER") && lines[1].Len() > 0) {
								token = lines[1];
								struct FSocketServerToken tokenStruct = tcpServer->getTokenStruct(token);

								if (tokenStruct.token.IsEmpty() && tokenStruct.token.Equals(token) == false) {
									triggerFileTransferOverTCPInfoEvent("Token not found.", sessionID, fullFilePath, false);
									run = false;
									break;
								}

								fullFilePath = tcpServer->getCleanDir(tokenStruct.directoryType, tokenStruct.fileDirectory);

								if (FPaths::FileExists(fullFilePath) == false) {
									triggerFileTransferOverTCPInfoEvent("File not found,", sessionID, fullFilePath, false);
									run = false;
									break;
								}


								FPlatformProcess::Sleep(sleepTime);

								bool md5okay = false;
								FString md5Server = FString();
								tcpServer->getMD5FromFile(fullFilePath, md5okay, md5Server);

								FString response = "REQUEST_FILE_FROM_SERVER_ACCEPTED_|_" + token + "_|_" + md5Server + "_|_" + tcpServer->int64ToString(tcpServer->fileSize(fullFilePath)) + "_|_" + FPaths::GetCleanFilename(fullFilePath) + "\r\n";
								response = tcpServer->encryptMessage(response);

								FTCHARToUTF8 Convert(*response);
								int32 bytesSendOverSocket = 0;
								clientSocket->Send((uint8*)Convert.Get(), Convert.Length(), bytesSendOverSocket);



							}
							else {

								triggerFileTransferOverTCPInfoEvent("File request incorrect (1).", sessionID, fullFilePath, false);

								recvMessage.Empty();
								recvDataArray.Empty();
								byteArray.Empty();

								run = false;
								break;
							}
						}
						else {

							if (lines.Num() == 3) {
								//server send data to client
								if (lines[0].Equals("REQUEST_FILE_FROM_SERVER_ACCEPTED") && lines[1].Equals(token) && lines[2].Len() > 0) {
									int64 startPosition = FCString::Atoi64(*lines[2]);

									sendFileThread = new FSocketServerTCPClientSendFileToThread(clientSocket, fullFilePath, fileSize, startPosition, session);
									FPlatformProcess::Sleep(sleepTime);

									//if (!sendFile(fullFilePath, startPosition, clientSocket, sessionID)) {
									//	triggerFileTransferOverTCPInfoEvent("Error while sending the file.", sessionID, fullFilePath, false);

									//	recvMessage.Empty();
									//	recvDataArray.Empty();
									//	byteArray.Empty();

									//	run = false;
									//	break;
									//}
								}
								else {

									if (lines[0].Equals("REQUEST_FILE_FROM_SERVER_END") && lines[1].Equals(token) && lines[2].Equals("OKAY")) {
										triggerFileTransferOverTCPInfoEvent("File transfer successful.", sessionID, fullFilePath, true);
										run = false;
										break;
									}
									else {

										triggerFileTransferOverTCPInfoEvent("File transfer failed.", sessionID, fullFilePath, false);

										recvMessage.Empty();
										recvDataArray.Empty();
										byteArray.Empty();

										run = false;
										break;
									}
								}
							}
							else {

								triggerFileTransferOverTCPInfoEvent("File request incorrect (3).", sessionID, fullFilePath, false);

								recvMessage.Empty();
								recvDataArray.Empty();
								byteArray.Empty();

								run = false;
								break;
							}
						}
					}
				}
				break;
			case 1:
				//downlnoad
				bytesRead = 0;
				recvDataArray.Empty();
				recvDataArray.SetNumUninitialized(DataSize);
				if (clientSocket->Recv(recvDataArray.GetData(), recvDataArray.Num(), bytesRead)) {
					writer->Serialize(const_cast<uint8*>(recvDataArray.GetData()), recvDataArray.Num());

					//show progress each second
					if ((ticksDownload + 10000000) <= FDateTime::Now().GetTicks()) {
						writer->Flush();
						int64 bytesSendLastSecond = bytesDownloaded - lastByte;
						//float speed = ((float)bytesSendLastSecond) / 125000;
						float mbit = ((float)bytesSendLastSecond) / 1024 / 1024 * 8;
						float sent = ((float)bytesDownloaded) / 1048576;
						float left = 0;
						float percent = 0;
						if (fileSize > 0) {
							left = ((float)(fileSize - bytesDownloaded)) / 1048576;
							percent = ((float)bytesDownloaded / (float)fileSize * 100);
						}


						triggerFileOverTCPProgress(sessionID, fullFilePath, percent, mbit, lastByte, fileSize);

						ticksDownload = FDateTime::Now().GetTicks();
						lastByte = bytesDownloaded;
					}
					bytesDownloaded += bytesRead;
				}
				if (bytesDownloaded >= fileSize) {
					//receive file finish
					triggerFileOverTCPProgress(sessionID, fullFilePath, 100, 0, bytesDownloaded, fileSize);

					if (writer != nullptr) {
						writer->Close();
					}

					FPlatformProcess::Sleep(1);
					sendEndMessage(fullFilePath, token, md5Client, sessionID, clientSocket);

					run = false;
				}

				break;

			}
		}
	}

	recvDataArray.Empty();


	run = false;


	if (sendFileThread != nullptr) {
		sendFileThread->stopThread();
		sendFileThread = nullptr;
	}

	//UE_LOG(LogTemp, Display, TEXT("TCP Connected: %s:%i"), *session.ip, session.port);


	if (writer != nullptr) {
		writer->Close();
		delete writer;
	}

	FPlatformProcess::Sleep(3);
	if (clientSocket != nullptr) {
		clientSocket->Close();
	}

	USocketServerTCP* tcpServerGlobal = tcpServer;
	AsyncTask(ENamedThreads::GameThread, [sessionID, serverID, tcpServerGlobal]() {
		USocketServerBPLibrary::socketServerBPLibrary->onsocketServerConnectionEventDelegate.Broadcast(EServerSocketConnectionEventType::E_Client, false, "Client disconnected", sessionID, serverID);
		//clean up socket in main thread because race condition
		tcpServerGlobal->removeClientSession(sessionID);
		});

	return 0;
}

void FSocketServerTCPFileHandlerThread::triggerFileOverTCPProgress(FString sessionIDP, FString filePathP, float percentP, float mbitP, int64 bytesReceivedP, int64 fileSizeP) {
	AsyncTask(ENamedThreads::GameThread, [sessionIDP, filePathP, percentP, mbitP, bytesReceivedP, fileSizeP]() {
		USocketServerBPLibrary::socketServerBPLibrary->onfileTransferOverTCPProgressEventDelegate.Broadcast(sessionIDP, filePathP, percentP, mbitP, bytesReceivedP, fileSizeP);
		});
}

void FSocketServerTCPFileHandlerThread::triggerFileTransferOverTCPInfoEvent(FString messageP, FString sessionIDP, FString filePathP, bool successP) {
	AsyncTask(ENamedThreads::GameThread, [messageP, sessionIDP, filePathP, successP]() {
		USocketServerBPLibrary::socketServerBPLibrary->onfileTransferOverTCPInfoEventDelegate.Broadcast(messageP, sessionIDP, filePathP, successP);
		});
}

void FSocketServerTCPFileHandlerThread::sendEndMessage(FString fullFilePathP, FString tokenP, FString md5ClientP, FString sessionIDP, FSocket* clientSocketP) {
	bool md5okay = false;
	FString md5Server = FString();
	tcpServer->getMD5FromFile(fullFilePathP, md5okay, md5Server);

	FString response = "SEND_FILE_TO_SERVER_END_|_" + tokenP + "_|_";

	if (md5okay && md5ClientP.Equals(md5Server)) {
		triggerFileTransferOverTCPInfoEvent("File successfully received.", sessionIDP, fullFilePathP, true);

		response += "OKAY\r\n";
	}
	else {
		triggerFileTransferOverTCPInfoEvent("File received but MD5 does not match. Corrupted file will be deleted if resume is not disabled.", sessionIDP, fullFilePathP, false);
		response += "MD5ERROR\r\n";
		if (tcpServer->hasResume() == false) {
			tcpServer->deleteFile(fullFilePathP);
		}
	}

	response = tcpServer->encryptMessage(response);

	FTCHARToUTF8 Convert(*response);
	int32 bytesSendOverSocket = 0;
	clientSocketP->Send((uint8*)Convert.Get(), Convert.Length(), bytesSendOverSocket);
}

void FSocketServerTCPFileHandlerThread::stopThread() {
	run = false;
}
