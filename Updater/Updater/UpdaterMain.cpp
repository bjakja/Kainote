﻿//  Copyright (c) 2018, Marcin Drob

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
#include "ZipHandler.h"

//#include <vld.h>
//#include <WinInet.h>

//#pragma comment (lib, "Wininet.lib")

using namespace std;

int main(int argc, char **argv)
{
	//const wchar_t server[] = L"www.dropbox.com";//"github.com";
	//const wchar_t page[] = L"s/t8pkey94ruakyox/Kainote%20x64.zip?dl=1";//"bjakja/Kainote/blob/master/Kaiplayer/VersionKainote.h";
	//const wchar_t filename[] = L"C:\\testowy Kainote.zip";
	//wchar_t filename[] = L"H:\Kainote\x64\Release\\Kainote x64.zip";
	//wchar_t targetfilename[]=L"I:\\レジェンドオブ・デュオ";
	//wchar_t targetfilename1[3000];
	// phrase[3000];
	wchar_t targetfilename1[] = L"J:\\Easeus 11 22_24\\Nowy(F)\\Lost Files\\#2 Lost Partition(NTFS)\\Anime";
	//wchar_t targetfilename2[] = L"H:\\";
	//wchar_t targetfilename3[] = L"F:\\";
	//wchar_t filesDir[] = L"H:\\Kainote\\x64\\Release";
	//wchar_t zipPatch[] = L"H:\\Kainote\\x64\\Release\\Kainote x64 b909.zip";
	//const wchar_t *excludes[] = { L"audiocache", L"indices", L"subs" };
	wcout << L"Podaj sciezke for zero size seeking: ";
	wcout << targetfilename1;
	//wcin >> targetfilename1;
	//while (wcslen(targetfilename1) == 0)
	//wcin >> targetfilename1;
	/*wcout << L"Podaj fraze: ";
	wcin >> phrase;
	while (wcslen(phrase) == 0)
	wcin >> phrase;*/
	
	ZipHandler zh;
	//zh.ZipFolder(filesDir, zipPatch, excludes, 3);
	////zh.UnZipFile(zipPatch, targetfilename);
	size_t dirs=0;
	size_t files=0;
	zh.CheckFiles(targetfilename1, &dirs, &files, NULL);
	wcout << L"location: " << targetfilename1 << L"\n";
	wcout << L"dirs: " << dirs<<L"\n";
	wcout << L"files: " << files << L"\n";
	if (zh.found.size()){
		wcout << L"found:\n";
		wcout << zh.found;
	}
	system("pause");
	return 0;
}