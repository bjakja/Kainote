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
#include "contrib/minizip/unzip.h"
#include <vld.h>
//#include <WinInet.h>

//#pragma comment (lib, "Wininet.lib")

bool convert(char *source, wchar_t *dest){
	int sourcelen = strlen(source);
	int result = MultiByteToWideChar(CP_UTF8, 0, source, sourcelen, 0, 0);
	if (!result || result>10000)
		return false;
	//*dest = new wchar_t[result + 1];
	int copied = MultiByteToWideChar(CP_UTF8, 0, source, sourcelen, dest, result);
	if (copied != result){
		//delete[] (*dest);
		//*dest = NULL;
		return false;
	}
	dest[result] = 0;
	return true;
}

using namespace std;

int main(int argc, char **argv)
{
	//const wchar_t server[] = L"www.dropbox.com";//"github.com";
	//const wchar_t page[] = L"s/t8pkey94ruakyox/Kainote%20x64.zip?dl=1";//"bjakja/Kainote/blob/master/Kaiplayer/VersionKainote.h";
	//const wchar_t filename[] = L"C:\\testowy Kainote.zip";
	const char filename[] = "H:\\Kainote\\x64\\Release\\Kainote x64.zip\0";
	wchar_t targetfilename[10003] = L"I:\\";
	wchar_t tmptargetfilename[10000];
	const int chunksize = 16384;
	char chunk[chunksize];
	unz_file_info64 pfile_info;
	char szFileName[10000];
	char extraField[10000];
	char szComment[10000];

	unzFile ufile = unzOpen(filename);
	HANDLE hFile;
	DWORD ss;
	if (ufile){
		if (!unzGoToFirstFile(ufile)){
			int result = 1;
			while (result){
				unzGetCurrentFileInfo64(ufile, &pfile_info, szFileName, 10000, extraField, 10000, szComment, 10000);
				if (!convert(szFileName, &targetfilename[3])){ 
					cout << "nie mo¿na przekonwertowaæ œcie¿ki " << szFileName << ", wtf?\n";
					continue; 
				}
				hFile = CreateFileW(targetfilename, GENERIC_WRITE, FILE_SHARE_WRITE, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, 0);
				if (hFile == INVALID_HANDLE_VALUE){
					wchar_t *pch = wcschr(&targetfilename[4], '\\');
					while (pch){
						size_t newsize = pch - targetfilename + 1;
						wcsncpy(tmptargetfilename, targetfilename, newsize);
						tmptargetfilename[newsize] = 0;
						if (!CreateDirectoryW(tmptargetfilename, 0)){
							if (GetLastError() != ERROR_ALREADY_EXISTS)
								cout << "nie mo¿na utworzyæ folderu\n";
						}
						pch = wcschr(pch+1, '\\');
					}
					hFile = CreateFileW(targetfilename, GENERIC_WRITE, FILE_SHARE_WRITE, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, 0);
					if (hFile == INVALID_HANDLE_VALUE){
						cout << "ci¹gle nie mo¿na utworzyæ pliku\n";
						continue;
					}
				}
				SetFilePointer(hFile, 0, NULL, FILE_BEGIN);
				if (unzOpenCurrentFile(ufile)){
					cout << "nie mo¿na otworzyæ pliku?\n";
					continue;
				}
				int readresult = unzReadCurrentFile(ufile, chunk, chunksize);
				while (readresult > 1){
					
					WriteFile(hFile, chunk, readresult, &ss, NULL);
					if (readresult != ss){
						unzCloseCurrentFile(ufile);
						unzClose(ufile);
						CloseHandle(hFile);
						cout << "zabrak³o miejsca na dysku albo odwali³ numer\n";
						return 1;
					}
					readresult = unzReadCurrentFile(ufile, &chunk, chunksize);
				}
				CloseHandle(hFile);
				unzCloseCurrentFile(ufile);
				result = !unzGoToNextFile(ufile);
			}
		}
		else{
			cout << "nie mo¿na przejœæ do pierwszego pliku?\n";
		}
	}
	else{
		cout << "dupa blada, ktoœ ukrad³ wskaŸnik zipa\n";
	}
	unzClose(ufile);
	//system("pause");
	return 0;
}