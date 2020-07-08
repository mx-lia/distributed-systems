// ClientU.cpp: определяет точку входа для консольного приложения.
//

#include "stdafx.h"
#include "Winsock2.h"
#include <iostream>
#include <string>
#include <ctime>
#pragma comment(lib, "WS2_32.lib")

using namespace std;

struct GETSINCHRO
{
	string cmd;
	int curvalue;
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
	case WSAEINVAL:		
		 msgText = "Ошибка в аргументе\n";				
		 break;
	default:				
		msgText = "Error\n";							
		break;
	};
	return msgText;
}

string SetErrorMsgText(string msgText, int code)
{
	return msgText + GetErrorMsgText(code);
};

int _tmain(int argc, _TCHAR* argv[])
{
	setlocale(LC_CTYPE, "Russian");

	string IP = "127.0.0.1";
	int Tc = 1000;
	int Cc = 0;

	SYSTEMTIME tm;
	GETSINCHRO getsincro, setsincro;

	getsincro.cmd = "SINC";
	getsincro.curvalue = 0;


	cout << "Клиент запущен" << endl;

	try
	{
		SOCKET cS;
		WSADATA wsaData;

		if (WSAStartup(MAKEWORD(2, 0), &wsaData) != 0)
			throw SetErrorMsgText("Startup: ", WSAGetLastError());

		if ((cS = socket(AF_INET, SOCK_DGRAM, NULL)) == INVALID_SOCKET)
			throw SetErrorMsgText("Socket: ", WSAGetLastError());

		SOCKADDR_IN serv;
		serv.sin_family = AF_INET;
		serv.sin_port = htons(2000);
		serv.sin_addr.s_addr = inet_addr(IP.c_str());
		int maxcor = 0;
		int mincor = INT_MAX;
		int avgcorr = 0;
		int lensockaddr = sizeof(serv);
		//sendto(cS, (char*)&getsincro, sizeof(getsincro), 0, (sockaddr*)&serv, sizeof(serv));

		//sendto(cS, (char *)&getsincro, sizeof(getsincro), 0, (sockaddr*)&serv, sizeof(serv));
		//recvfrom(cS, (char *)&setsincro, sizeof(setsincro), 0, (sockaddr*)&serv, &lensockaddr);
		//getsincro.curvalue += setsincro.curvalue;

		for (int i = 0; i < 10; i++)
		{
			GetSystemTime(&tm);
			sendto(cS, (char *)&getsincro, sizeof(getsincro), 0, (sockaddr*)&serv, sizeof(serv));
			recvfrom(cS, (char *)&setsincro, sizeof(setsincro), 0, (sockaddr*)&serv, &lensockaddr);
			maxcor = maxcor < setsincro.curvalue ? setsincro.curvalue : maxcor;
			mincor = mincor > setsincro.curvalue ? setsincro.curvalue : mincor;

			cout << "#" << i + 1 << " " << getsincro.curvalue << '\n' << "Timestamp: " << tm.wMonth << "." << tm.wDay << " " << tm.wHour + 3 << ":" << tm.wMinute << ":" << tm.wSecond << ":" << tm.wMilliseconds << '\n' << "Correction: " << setsincro.curvalue << " Min/max correction: " << mincor << "/" << maxcor << '\n';

			getsincro.curvalue += setsincro.curvalue + Tc;

			avgcorr += setsincro.curvalue;

			Sleep(Tc);
		}
		cout << "Average correction: " << avgcorr / 10 << endl;

		if (closesocket(cS) == SOCKET_ERROR)
			throw SetErrorMsgText("Closesocket: ", WSAGetLastError());

		if (WSACleanup() == SOCKET_ERROR)
			throw SetErrorMsgText("Cleanup: ", WSAGetLastError());
	}
	catch (string errorMsgText)
	{
		cout << endl << errorMsgText << endl;
	}

	return 0;
}