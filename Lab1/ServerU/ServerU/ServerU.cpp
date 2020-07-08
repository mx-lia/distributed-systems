// ServerU.cpp: определяет точку входа для консольного приложения.
//

#include "stdafx.h"
#include "iostream"
#include "string.h"
#include "locale"
#include "time.h"
#include "Winsock2.h"
#pragma comment(lib, "WS2_32.lib")

using namespace std;

struct SETSINCRO
{
	string cmd;
	int correction;
};

string GetErrorMsgText(int code)
{
	string msgText;

	switch (code)
	{
	case WSAEINTR:		
		msgText = "Работа функции прервана\n";			
		break;
	case WSAEACCES:				
		msgText = "Разрешение отвергнуто\n";						
		break;
	case WSAEFAULT:		
		msgText = "Ошибочный адрес\n";					
		break;
	default:				
		msgText = "Error\n";							
		break;
	};

	return msgText;
}

string SetErrorMsgText(string msgText, int code)
{
	return  msgText + GetErrorMsgText(code);
};

int setAverageCorrection(int averageCorrection[], int length)
{
	int value = 0;
	for (int i = 0; i < length; i++)
		value += averageCorrection[i];

	return value / length;
}

time_t ntpdate() {
	char* hostname = (char*)"88.147.254.232";

	int portno = 123;
	const int maxlen = 1024;
	int i;
	unsigned char msg[48] = { 010,0,0,0,0,0,0,0,0 };

	unsigned long buf[maxlen];
	struct protoent* proto;
	struct sockaddr_in server_addr;
	int s;
	time_t tmit;

	proto = getprotobyname("udp");
	s = socket(PF_INET, SOCK_DGRAM, proto->p_proto);

	memset(&server_addr, 0, sizeof(server_addr));
	server_addr.sin_family = AF_INET;
	server_addr.sin_addr.s_addr = inet_addr(hostname);
	server_addr.sin_port = htons(portno);

	i = sendto(s, (const char*)msg, sizeof(msg), 0, (struct sockaddr*) & server_addr, sizeof(server_addr));

	struct sockaddr saddr;
	int fromlen = sizeof(saddr);
	i = recvfrom(s, (char*)buf, 48, 0, &saddr, &fromlen);

	tmit = ntohl((time_t)buf[4]); //# get transmit time
	tmit -= 2208988800U;
	return tmit;
}

int _tmain(int argc, _TCHAR* argv[])
{
	setlocale(LC_ALL, "Russian");

	SETSINCRO setsincro, getsincro;
	setsincro.cmd = "SINCRO";
	setsincro.correction = 0;

	SYSTEMTIME tm;

	clock_t c;
	int averageCorrection[10];

	cout << "Сервер запущен" << endl;

	try
	{
		SOCKET sS;
		WSADATA wsaData;

		if (WSAStartup(MAKEWORD(2, 0), &wsaData) != 0)
			throw SetErrorMsgText("Startup: ", WSAGetLastError());

		if ((sS = socket(AF_INET, SOCK_DGRAM, NULL)) == INVALID_SOCKET)
			throw SetErrorMsgText("Socket: ", WSAGetLastError());

		SOCKADDR_IN serv;
		serv.sin_family = AF_INET;
		serv.sin_port = htons(2000);
		serv.sin_addr.s_addr = INADDR_ANY;

		if (bind(sS, (LPSOCKADDR)&serv, sizeof(serv)) == SOCKET_ERROR)
			throw SetErrorMsgText("Bind_Server: ", WSAGetLastError());

		int count = 1;
		SOCKADDR_IN client;
		int lc = sizeof(client);

		//recvfrom(sS, (char*)&getsincro, sizeof(getsincro), NULL, (sockaddr*)&client, &lc);

		while (count != 11)
		{
			int average = 0;
			GetSystemTime(&tm);
			recvfrom(sS, (char*)&getsincro, sizeof(getsincro), NULL, (sockaddr*)&client, &lc);
			c = clock(); //ntpdate();clock();
			setsincro.correction = c - getsincro.correction;
			averageCorrection[count - 1] = setsincro.correction;
			average = setAverageCorrection(averageCorrection, count);
			sendto(sS, (char*)&setsincro, sizeof(setsincro), 0, (sockaddr*)&client, sizeof(client));

			cout << "Request: " << count << '\n' \
				<< "Timestamp: " << tm.wMonth << "." << tm.wDay << " " << tm.wHour + 3 << ":" << tm.wMinute << ":" << tm.wSecond << ":" << tm.wMilliseconds << '\n' 
				<< "IP:" << inet_ntoa(client.sin_addr) << '\n'
				<< "Correction: " << setsincro.correction << '\n'
				<< "Avg correction: " << average << endl;
			count++;
		}

		if (closesocket(sS) == SOCKET_ERROR)
			throw SetErrorMsgText("close socket: ", WSAGetLastError());
		if (WSACleanup() == SOCKET_ERROR)
			throw SetErrorMsgText("Cleanup: ", WSAGetLastError());
	}
	catch (string errorMsgText)
	{
		cout << endl << errorMsgText;
	}

	return 0;
}