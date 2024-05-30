#define WIN32_LEAN_AND_MEAN

#include <iostream>
#include <fstream>
#include <Windows.h>
#include <WinSock2.h>
#include <WS2tcpip.h>

class Server {
public:
	Server() {
		StartWSA();

		InitializeHints();

		TranslateFormHostnameToIPAddress();
	}
	bool Run() {

		ProcessListenSocket();
		while (true) {
			if (FileReceive() == false) {
				return false;
			}
		}
		return true;
	}

private:
	WSADATA wsaData;
	ADDRINFO hints;
	ADDRINFO *addrResult = NULL;
	SOCKET ClientSocket = INVALID_SOCKET;
	SOCKET ListenSocket = INVALID_SOCKET;
	PCSTR port_ = "777";

	const char *sendBuffer = "Hello from Server!";
	char recvBuffer[512];
	int result;

	bool StartWSA() {
		int result = WSAStartup(MAKEWORD(2, 2), &wsaData);
		if (result != 0) {
			std::cerr << "WSAStartup failed, result = " << result << std::endl;
			return false;
		}
		return true;
	}

	void InitializeHints() {
		ZeroMemory(&hints, sizeof(hints));
		hints.ai_family = AF_INET;
		hints.ai_socktype = SOCK_STREAM;
		hints.ai_protocol = IPPROTO_TCP;
		hints.ai_flags = AI_PASSIVE;
	}

	bool TranslateFormHostnameToIPAddress() {
		result = getaddrinfo(NULL, port_, &hints, &addrResult);
		if (result != 0) {
			std::cerr << "getaddrinfo failed, result = " << result << std::endl;
			WSACleanup();
			return 1;
		}
	}

	bool InitializeListenSocket() {
		ListenSocket = socket(addrResult->ai_family, addrResult->ai_socktype, addrResult->ai_protocol);
		if (ListenSocket == INVALID_SOCKET) {
			std::cerr << "socket creation failed" << std::endl;
			freeaddrinfo(addrResult);
			WSACleanup();
			return 1;
		}
	}

	void BindListenSocket() {
		result = bind(ListenSocket, addrResult->ai_addr, (int)addrResult->ai_addrlen);
		CheckIfSocketHasError("Binding socket failed, result = ", ListenSocket);
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

	void ProcessListenSocket() {
		InitializeListenSocket();

		BindListenSocket();

		result = listen(ListenSocket, SOMAXCONN);
		CheckIfSocketHasError("Listening socket failed,error: ", ListenSocket);

		ClientSocket = accept(ListenSocket, NULL, NULL);
		CheckIfSocketHasError("Accepting socket failed,error: ", ListenSocket);

		closesocket(ListenSocket);
	}

	bool ReceiveData() {
		ZeroMemory(recvBuffer, 512);
		do {
			result = recv(ClientSocket, recvBuffer, 512, 0);
			if (result > 0) {
				std::cout << "Received " << result << " bytes" << std::endl;
				std::cout << "Received data: " << recvBuffer << std::endl;
				send(ClientSocket, sendBuffer, (int)strlen(sendBuffer), 0);
				CheckIfSocketHasError("Send data failed,error: ", ClientSocket);
			} else if (result == 0) {
				std::cout << "Connection closing..." << std::endl;
			} else {
				std::cout << "recv failed with error " << result << std::endl;
				CloseSocket(ClientSocket);
				return 1;
			}
		} while (result > 0);

		result = shutdown(ClientSocket, SD_SEND);
		CheckIfSocketHasError("Shutdown client socket ", ClientSocket);

		CloseSocket(ClientSocket);
		return 0;
	}

	bool FileReceive() {
		std::ofstream out("screenshot.bmp", std::ios::binary | std::ios::trunc | std::ios::app);
		do {
			result = recv(ClientSocket, recvBuffer, (int)strlen(sendBuffer), 0);
			if (out.is_open()) {
				out.write(recvBuffer, (int)strlen(sendBuffer));
				ZeroMemory(&recvBuffer, (int)strlen(sendBuffer));
			}
		} while (result > 0);
		return true;
	}
};

int main() {
	Server server;
	server.Run();


}