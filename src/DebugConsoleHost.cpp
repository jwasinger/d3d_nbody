#include "DebugConsoleHost.h"

DWORD WINAPI DebugConsoleHostThread(LPVOID lpvParam) //lpvoid is an array consisting of {pipeHandle, a pointer to a DebugConsoleHost instance}
{
	DebugConsoleHost *const host = (DebugConsoleHost *)lpvParam;
	char readBuff[256];
	DWORD bytesRead;
	
	if(!ReadFile(host->hConsolePipe, readBuff, sizeof(char) * 256, &bytesRead, NULL))
	{
		if(GetLastError() == ERROR_IO_PENDING)
			return;
		else
		{
			OutputDebugString("\nUnknown pipe error\n");
			return;
		}
	}

	/*******************
	
	while(true):
		if data is pending in the read-buffer:
			parse the data and execute the commands it contains

		if data is pending in the write-vector:
			while(write-vector is not empty):
				write data in write-vector[i] to the pipe
				remove write-data[i]

	*****************/
}

void DebugConsoleHost::WriteMessage(std::string &str)
{
	if(str.length > 255)
	{
		
	}
}

void DebugConsoleHost::Update(void)
{
	
}

DebugConsoleHost::DebugConsoleHost(void)
{
}


DebugConsoleHost::~DebugConsoleHost(void)
{
}

bool DebugConsoleHost::Init(void)
{
	this->hConsolePipe = CreateNamedPipe(
							"\\\\.\\pipe\\consolePipe", 
							PIPE_ACCESS_DUPLEX,
							PIPE_TYPE_BYTE | PIPE_READMODE_BYTE,
							1,
							256*sizeof(char), 
							256*sizeof(char),
							0, 
							NULL);

	if(!this->hConsolePipe)
	{
		OutputDebugString("\nfailed to create the console pipe\n");
		return false;
	}

	//run the client app and then hook it up 
	LPVOID threadParam = (LPVOID)this;
	CreateThread(NULL, 9, DebugConsoleHostThread,
}