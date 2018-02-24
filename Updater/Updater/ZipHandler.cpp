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

#include <windows.h>
#include "contrib/minizip/unzip.h"

#include "ZipHandler.h"

ZipHandler::ZipHandler()
{

}

bool ZipHandler::UnZipFile(const wchar_t *pathOfZip, const wchar_t *destinationDir)
{
	wchar_t targetfilename[10003];
	wcscpy(targetfilename, destinationDir);
	size_t destDirLen = wcslen(targetfilename);
	if (destinationDir[destDirLen - 1] != '\\'){
		targetfilename[destDirLen] = '\\';
		destDirLen++;
	}
	wchar_t tmptargetfilename[10000];
	const int chunksize = 16384;
	char chunk[chunksize];
	unz_file_info64 pfile_info;
	char szFileName[10000];
	char extraField[10000];
	char szComment[10000];

	unzFile ufile = unzOpen64(pathOfZip);
	HANDLE hFile;
	DWORD ss;
	if (ufile){
		if (!unzGoToFirstFile(ufile)){
			int result = 1;
			while (result){
				unzGetCurrentFileInfo64(ufile, &pfile_info, szFileName, 10000, extraField, 10000, szComment, 10000);
				if (!ConvertToWchar(szFileName, &targetfilename[destDirLen])){
					log += "Cannot convert path ";
					log += szFileName;
					log += "\n";
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
								log += "Cannot create folder\n";
						}
						pch = wcschr(pch + 1, '\\');
					}
					hFile = CreateFileW(targetfilename, GENERIC_WRITE, FILE_SHARE_WRITE, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, 0);
					if (hFile == INVALID_HANDLE_VALUE){
						log += "Still cannot create file\n";
						continue;
					}
				}
				SetFilePointer(hFile, 0, NULL, FILE_BEGIN);
				if (unzOpenCurrentFile(ufile)){
					log += "Cannot open file in zip\n";
					continue;
				}
				int readresult = unzReadCurrentFile(ufile, chunk, chunksize);
				while (readresult > 1){

					WriteFile(hFile, chunk, readresult, &ss, NULL);
					if (readresult != ss){
						unzCloseCurrentFile(ufile);
						unzClose(ufile);
						CloseHandle(hFile);
						log += "There is no disc space or path is inaccessible\n";
						return false;
					}
					readresult = unzReadCurrentFile(ufile, &chunk, chunksize);
				}
				CloseHandle(hFile);
				unzCloseCurrentFile(ufile);
				result = !unzGoToNextFile(ufile);
			}
		}
		else{
			log += "Cannot step to first file\n";
		}
	}
	else{
		log += "Cannot open zip file\n";
	}
	unzClose(ufile);
	return true;
}

bool ZipHandler::ZipFolder(const wchar_t *destinationDir, const wchar_t *pathOfZip, const wchar_t **excludes, int numExcludes)
{
	zipFile zf = zipOpen64(pathOfZip, APPEND_STATUS_CREATE);
	if (zf == NULL)
		return false;

	size_t destinationDirLen = wcslen(destinationDir);
	for (int i = destinationDirLen - 2; i > 0; i--){
		if (destinationDir[i] == L'\\'){
			firstFolderStart = i+1;
			break;
		}
	}
	
	bool returnval = ZipFolderFiles(zf, destinationDir, pathOfZip, excludes, numExcludes);
	zipClose(zf, NULL);
	return returnval;
}

bool ZipHandler::ConvertToWchar(char *source, wchar_t *dest)
{
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

bool ZipHandler::ConvertToChar(const wchar_t *source, char **dest)
{
	int sourcelen = wcslen(source);
	int result = WideCharToMultiByte(CP_UTF8, 0, source, sourcelen, 0, 0, 0, 0);
	if (!result)
		return false;
	*dest = new char[result + 1];
	int copied = WideCharToMultiByte(CP_UTF8, 0, source, sourcelen, *dest, result, 0, 0);
	if (copied != result){
		delete[] (*dest);
		*dest = NULL;
		return false;
	}
	(*dest)[result] = 0;
	return true;
}

bool ZipHandler::CheckExcludes(wchar_t *path, const wchar_t **excludes, int numExcludes)
{
	for (int i = 0; i < numExcludes; i++){
		wchar_t *subpath = GetSubstring(path, wcslen(excludes[i]));
		if (subpath){
			if (!_wcsicmp(subpath, excludes[i])){
				delete[] subpath;
				return true;
			}
			delete[] subpath;
		}
	}
	return false;
}

wchar_t * ZipHandler::GetSubstring(wchar_t *string, int numChar)
{
	if (!numChar)
		return NULL;

	wchar_t * cpy = new wchar_t[numChar + 1];
	wcsncpy(cpy, string, numChar);
	cpy[numChar] = 0;
	return cpy;
}

bool ZipHandler::ZipFolderFiles(zipFile zf, const wchar_t *destinationDir, const wchar_t *pathOfZip, const wchar_t **excludes, int numExcludes)
{
	size_t destinationDirLen = wcslen(destinationDir);
	bool putBackSlash = destinationDir[destinationDirLen - 1] != L'\\';
	int filterSize = (putBackSlash) ? destinationDirLen + 7 : destinationDirLen + 6;
	wchar_t * filter = new wchar_t[filterSize];
	wcscpy(filter, L"\\\\?\\\0");
	wcscpy(&filter[4], destinationDir);
	wcscpy(&filter[destinationDirLen + 4], (putBackSlash) ? L"\\*\0" : L"*\0");
	_WIN32_FIND_DATAW data;
	HANDLE fileHandle = FindFirstFileW(filter, &data);
	if (fileHandle == INVALID_HANDLE_VALUE){
		log += "Cannot open directory to pack\n";
		delete[] filter;
		return false;
	}

	//To trzeba przerobiæ, folder pocz¹tkowy musi byæ zawsze taki sam.
	const wchar_t *firstFolderName = &destinationDir[firstFolderStart];
	size_t firstFolderSize = wcslen(firstFolderName);

	do
	{
		if (!wcscmp(data.cFileName, L".") || !wcscmp(data.cFileName, L".."))
			continue;

		if (CheckExcludes(data.cFileName, excludes, numExcludes))
			continue;

		size_t fsize = wcslen(data.cFileName);
		if (fsize>4 && !_wcsicmp(&data.cFileName[fsize-4], L".zip"))
			continue;

		size_t ffsize = destinationDirLen;
		size_t fullFilepathsize = putBackSlash ? fsize + destinationDirLen + 2 : fsize + destinationDirLen + 1;
		wchar_t * fullfilepath = new wchar_t[fullFilepathsize];
		wcscpy(fullfilepath, destinationDir);

		if (data.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY){
			if (putBackSlash){
				wcscpy(&fullfilepath[ffsize], L"\\");
				ffsize++;
			}
			wcscpy(&fullfilepath[ffsize], data.cFileName);
			ZipFolderFiles(zf, fullfilepath, pathOfZip, excludes, numExcludes);
			delete[] fullfilepath;
			continue;
		}
		
		
		size_t filepathsize = putBackSlash ? fsize + firstFolderSize + 2 : fsize + firstFolderSize + 1;
		
		wchar_t * filepath = new wchar_t[filepathsize];
		
		if (firstFolderSize)
			wcscpy(filepath, firstFolderName);
		size_t cffs = firstFolderSize;
		if (putBackSlash){
			wcscpy(&filepath[cffs], L"\\");
			cffs++;
			wcscpy(&fullfilepath[ffsize], L"\\");
			ffsize++;
		}
		wcscpy(&filepath[cffs], data.cFileName);
		wcscpy(&fullfilepath[ffsize], data.cFileName);
		//filepath[filepathsize - 1] = 0;

		ZipFile(zf, filepath, fullfilepath);
		delete[] filepath;
		delete[] fullfilepath;
	} while (FindNextFileW(fileHandle, &data) != 0);
	delete[] filter;
	FindClose(fileHandle);
	return true;
}

bool ZipHandler::ZipFile(zipFile zf, const wchar_t *name, const wchar_t *filepath)
{
	char *charname = NULL;
	if (!ConvertToChar(name, &charname)){
		log += "Cannot convert filename to ANSI\n";
		return false;
	}
	size_t size = 0;
	FILE *file = _wfopen(filepath, L"rb");
	if (!file){
		zip_fileinfo zfi = { 0 };
		size_t charnamesize = strlen(charname);
		char *pseudofile = new char[charnamesize + 2];
		strcpy(pseudofile, charname);
		strcpy(&pseudofile[charnamesize], "-");
		pseudofile[charnamesize + 1] = 0;
		delete[] charname;
		if (S_OK == zipOpenNewFileInZip(zf, pseudofile, &zfi, NULL, 0, NULL, 0, NULL, Z_DEFLATED, Z_DEFAULT_COMPRESSION)){
			if (zipCloseFileInZip(zf))
				log += "Cannot close file in zip\n";

			return true;
		}
		log += "Cannot open file in zip\n";
		return false;
	}
	else
	{
		zip_fileinfo zfi = { 0 };
		if (S_OK == zipOpenNewFileInZip(zf, charname, &zfi, NULL, 0, NULL, 0, NULL, Z_DEFLATED, Z_DEFAULT_COMPRESSION))
		{
			while (size = fread(buffer, 1, CHUNK, file))
			{

				if (zipWriteInFileInZip(zf, buffer, size))
					log += "Cannot write file in zip\n";

			}
			if (zipCloseFileInZip(zf))
				log += "Cannot close file in zip\n";
			fclose(file);
			delete[] charname;
			return true;
		}
		log += "Cannot open file in zip\n";
		delete[] charname;
		fclose(file);
	}
	return true;
}
