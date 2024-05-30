#define WIN32_LEAN_AND_MEAN

#include <iostream>
#include <fstream>
#include <Windows.h>
#include <WinSock2.h>
#include <WS2tcpip.h>

class Client {
public:

	Client() {
		StartWSA();

		InitializeHints();

		TranslateFormHostnameToIPAddress();
	}

	bool Run() {

		InitializeConnectSocket();

		ConnectSocketTo();


		//SendData(sendBuffer);

		//ShutdownSocket();
		
		//ReceiveData();

		//CloseSocket(ConnectSocket);

		return 0;
	}

	void FileSend(const WCHAR *FilePath) {
		std::streampos file_size = 0;
		std::ifstream in(FilePath, std::ios::binary);
		char sendBuffer[512];
		ZeroMemory(&sendBuffer, (int)strlen(sendBuffer));

		if (in.is_open()) {
			while (1) {
				in.read(sendBuffer, (int)strlen(sendBuffer));
				if (in.eof()) {
					std::cerr << "End of File sending from Client" << std::endl;
					in.close();
					break;
				} else {
					result = send(ConnectSocket, sendBuffer, (int)strlen(sendBuffer), 0);
					std::cout << "Sent: " << result << " bytes" << std::endl;
					ZeroMemory(&sendBuffer, (int)strlen(sendBuffer));
				}
			}
		}
	}

	bool SendData(const char FAR *buffer) {
		result = send(ConnectSocket, buffer, (int)strlen(buffer), 0);
		CheckIfSocketHasError("send failed,error: ", ConnectSocket);
		std::cout << "Sent: " << result << " bytes" << std::endl;
		return true;
	}

private:
	WSADATA wsaData;
	ADDRINFO hints;
	ADDRINFO *addrResult = NULL;
	SOCKET ConnectSocket = INVALID_SOCKET;
	PCSTR port_ = "777";

	//const char *sendBuffer = "Hello from Client!";
	char recvBuffer[512];
	int result;

	bool StartWSA() {
		result = WSAStartup(MAKEWORD(2, 2), &wsaData);
		if (result != 0) {
			std::cerr << "WSAStartup failed, result = " << result << std::endl;
			return true;
		}
		return false;
	}

	void InitializeHints() {
		ZeroMemory(&hints, sizeof(hints));
		hints.ai_family = AF_INET;
		hints.ai_socktype = SOCK_STREAM;
		hints.ai_protocol = IPPROTO_TCP;
	}

	bool TranslateFormHostnameToIPAddress() {
		result = getaddrinfo("localhost", port_, &hints, &addrResult);
		if (result != 0) {
			std::cerr << "getaddrinfo failed, result = " << result << std::endl;
			WSACleanup();
			return 1;
		}
	}

	bool InitializeConnectSocket() {
		ConnectSocket = socket(addrResult->ai_family, addrResult->ai_socktype, addrResult->ai_protocol);
		if (ConnectSocket == INVALID_SOCKET) {
			std::cerr << "socket creation failed" << std::endl;
			freeaddrinfo(addrResult);
			WSACleanup();
			return 1;
		}
	}

	bool ConnectSocketTo() {
		result = connect(ConnectSocket, addrResult->ai_addr, (int)addrResult->ai_addrlen);
		if (result == SOCKET_ERROR) {
			std::cerr << "socket error. Unable connect to server, result = " << result << std::endl;
			CloseSocket(ConnectSocket);
			return 1;
		}
	}



	bool ShutdownSocket() {
		result = shutdown(ConnectSocket, SD_SEND);
		CheckIfSocketHasError("shutdown failed,error: ", ConnectSocket);
		return true;
	}


	


	void CloseSocket(SOCKET socket) {
		closesocket(socket);
		freeaddrinfo(addrResult);
		WSACleanup();
	}

	bool CheckIfSocketHasError(std::string error_message, SOCKET socket) {
		if (result == SOCKET_ERROR) {
			std::cerr << "Listening socket failed,error: " << result << std::endl;
			CloseSocket(socket);
			return false;
		}
		return true;
	}

	void ReceiveData() {
		ZeroMemory(recvBuffer, 512);
		do {
			result = recv(ConnectSocket, recvBuffer, 512, 0);
			if (result > 0) {
				std::cout << "Received " << result << " bytes" << std::endl;
				std::cout << "Received data: " << recvBuffer << std::endl;
			} else if (result == 0) {
				std::cout << "Connection closed" << std::endl;
			} else {
				std::cout << "recv failed with error " << result << std::endl;
			}
		} while (result > 0);
	}
};

BOOL WINAPI SaveBitmap(const WCHAR *wPath) {
	BITMAPFILEHEADER bfHeader;
	BITMAPINFOHEADER biHeader;
	BITMAPINFO bInfo;
	HGDIOBJ hTempBitmap;
	HBITMAP hBitmap;
	BITMAP bAllDesktops;
	HDC hDC, hMemDC;
	LONG lWidth, lHeight;
	BYTE *bBits = NULL;
	HANDLE hHeap = GetProcessHeap();
	DWORD cbBits, dwWritten = 0;
	HANDLE hFile;
	INT x = GetSystemMetrics(SM_XVIRTUALSCREEN);
	INT y = GetSystemMetrics(SM_YVIRTUALSCREEN);

	ZeroMemory(&bfHeader, sizeof(BITMAPFILEHEADER));
	ZeroMemory(&biHeader, sizeof(BITMAPINFOHEADER));
	ZeroMemory(&bInfo, sizeof(BITMAPINFO));
	ZeroMemory(&bAllDesktops, sizeof(BITMAP));

	hDC = GetDC(NULL);
	hTempBitmap = GetCurrentObject(hDC, OBJ_BITMAP);
	GetObjectW(hTempBitmap, sizeof(BITMAP), &bAllDesktops);

	lWidth = bAllDesktops.bmWidth;
	lHeight = bAllDesktops.bmHeight;

	DeleteObject(hTempBitmap);

	bfHeader.bfType = (WORD)('B' | ('M' << 8));
	bfHeader.bfOffBits = sizeof(BITMAPFILEHEADER) + sizeof(BITMAPINFOHEADER);
	biHeader.biSize = sizeof(BITMAPINFOHEADER);
	biHeader.biBitCount = 24;
	biHeader.biCompression = BI_RGB;
	biHeader.biPlanes = 1;
	biHeader.biWidth = lWidth;
	biHeader.biHeight = lHeight;

	bInfo.bmiHeader = biHeader;

	cbBits = (((24 * lWidth + 31) & ~31) / 8) * lHeight;

	hMemDC = CreateCompatibleDC(hDC);
	hBitmap = CreateDIBSection(hDC, &bInfo, DIB_RGB_COLORS, (VOID **)&bBits, NULL, 0);
	SelectObject(hMemDC, hBitmap);
	BitBlt(hMemDC, 0, 0, lWidth, lHeight, hDC, x, y, SRCCOPY);


	hFile = CreateFileW(wPath, GENERIC_WRITE | GENERIC_READ, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
	if (INVALID_HANDLE_VALUE == hFile) {
		DeleteDC(hMemDC);
		ReleaseDC(NULL, hDC);
		DeleteObject(hBitmap);

		return FALSE;
	}
	WriteFile(hFile, &bfHeader, sizeof(BITMAPFILEHEADER), &dwWritten, NULL);
	WriteFile(hFile, &biHeader, sizeof(BITMAPINFOHEADER), &dwWritten, NULL);
	WriteFile(hFile, bBits, cbBits, &dwWritten, NULL);
	FlushFileBuffers(hFile);
	CloseHandle(hFile);

	DeleteDC(hMemDC);
	ReleaseDC(NULL, hDC);
	DeleteObject(hBitmap);

	return TRUE;
}

int main() {

	Client client;
	client.Run();	
	std::string command;
	while (true) {
		std::cout << "You can send: screenshot[scr], hello[hi], or exit client[exit]\n";
		std::cin >> command;
		if (command == "scr") {
			const WCHAR* screenshot_name = L"screenshot";
			SaveBitmap(screenshot_name);
			HANDLE hFile = CreateFileW(screenshot_name, GENERIC_READ, 0, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
			client.FileSend(screenshot_name);
		} else if (command == "hi") {

		} else if (command == "exit") {

		} else {

		}
	}
	system("pause");
}