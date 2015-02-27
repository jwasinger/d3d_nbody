//#include "common_include.h"
//#include <vector>
//
//#ifndef DEBUG_CONSOLE_HOST_H
//#define DEBUG_CONSOLE_HOST_H
//
//DWORD WINAPI DebugConsoleHostThread(LPVOID lpvParam);
//
//class DebugConsoleHost
//{
//	friend DWORD WINAPI DebugConsoleHostThread(LPVOID lpvParam);
//
//private:
//	bool clientConnected;
//	bool writePending;
//	byte writeData[256];
//
//
//	std::vector<byte*> messageQueue;
//
//	HANDLE hConsolePipe;
//
//public:
//	DebugConsoleHost(void);
//	~DebugConsoleHost(void);
//
//	bool Init(void);
//	void Update(void);
//	void WriteMessage(std::string &str);
//
//private:
//	void onReceiveMessage(byte *data);
//};
//
//#endif