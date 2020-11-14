//  Copyright (c) 2018, Marcin Drob

//  Kainote is free software: you can redistribute it and/or modify
//  it under the terms of the GNU General Public License as published by
//  the Free Software Foundation, either version 3 of the License, or
//  (at your option) any later version.

//  Kainote is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU General Public License for more details.

//  You should have received a copy of the GNU General Public License
//  along with Kainote.  If not, see <http://www.gnu.org/licenses/>.

#include <string>
#include <stdio.h>
#include <iostream>
#include <windows.h>
#include <WinInet.h>

#pragma comment (lib, "Wininet.lib")

using namespace std;

int Downloader(const wchar_t *server, const wchar_t *page, const wchar_t *filename, string *output)
{
	char szData[1024];
	bool writeToFile = filename != NULL;
	if (!output && !writeToFile)
		return 5;
	
	HANDLE hFile = INVALID_HANDLE_VALUE;
	if (writeToFile){
		hFile = CreateFileW(filename, GENERIC_WRITE, FILE_SHARE_WRITE, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, 0);
		SetFilePointer(hFile, 0, NULL, FILE_BEGIN);
	}
	DWORD ss;
	// initialize WinInet
	HINTERNET hInternet = InternetOpen(TEXT("Kainote updater"), INTERNET_OPEN_TYPE_PRECONFIG, NULL, NULL, 0);
	if (hInternet != NULL)
	{
		// open HTTP session
		HINTERNET hConnect = InternetConnectW(hInternet, server, INTERNET_DEFAULT_HTTPS_PORT, NULL, NULL, INTERNET_SERVICE_HTTP, 0, 1);
		if (hConnect != NULL)
		{
			//wstring request = page;

			// open request
			HINTERNET hRequest = HttpOpenRequestW(hConnect, L"GET", page, NULL, NULL, 0, INTERNET_FLAG_KEEP_CONNECTION | INTERNET_FLAG_SECURE, 1);
			if (hRequest != NULL)
			{
				// send request
				BOOL isSend = HttpSendRequestW(hRequest, NULL, 0, NULL, 0);

				if (isSend)
				{
					for (;;)
					{
						// reading data
						DWORD dwByteRead;
						BOOL isRead = InternetReadFile(hRequest, szData, sizeof(szData), &dwByteRead);

						// break cycle if error or end
						if (isRead == FALSE || dwByteRead == 0)
							break;

						// saving result
						if (writeToFile){
							WriteFile(hFile, szData, dwByteRead, &ss, NULL);
							if (dwByteRead != ss){
								return 1;
							}
						}
						else{
							(*output) += string(szData, dwByteRead);
						}
					}
				}
				else{ return 4; }

				// close request
				InternetCloseHandle(hRequest);
			}
			else{ return 3; }
			// close session
			InternetCloseHandle(hConnect);
		}
		else{ return 2; }
		// close WinInet
		InternetCloseHandle(hInternet);
	}
	if (writeToFile){
		CloseHandle(hFile);
	}
	//cout << output;
	return 0;
}