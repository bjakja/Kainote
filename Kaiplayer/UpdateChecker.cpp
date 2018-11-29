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
#include <windows.h>
#include <WinInet.h>
#include "UpdateChecker.h"
#include "Config.h"
#include "KaiMessageBox.h"
#include "VersionKainote.h"
#include "KainoteApp.h"
#include <wx/regex.h>

#pragma comment (lib, "Wininet.lib")

using namespace std;

//#ifndef SAFE_DELETE
//#define SAFE_DELETE(x) if (x !=NULL) { delete x; x = NULL; }
//#endif

UpdateChecker::UpdateChecker()
{
	//0 on every launch, 1 after two days, 2 after three days, 3 after week, 4 after two weeks, 5 after month, 6 never
	checkIntensity = Options.GetInt(UpdaterCheckIntensity);
	if (checkIntensity > 5){ return; }
	//0 on start, 1 on close, 2 on close without asking
	int checkOptions = Options.GetInt(UpdaterCheckOptions);
	dontAskForUpdate = checkOptions == 2;
	checkOnClose = checkOptions > 0;
	updateStable = Options.GetBool(UpdaterCheckForStable);
}

UpdateChecker::~UpdateChecker()
{
	if (checkOnClose){
		CheckAsynchronously(this, false);
	}
	else if (updateOnClose){
		Update(false);
	}
}

int UpdateChecker::CheckAsynchronously(UpdateChecker *checker, bool closeProgram /*= true*/)
{
	string output;
	if (checker->Downloader(L"github.com", L"bjakja/Kainote/blob/master/Kaiplayer/VersionKainote.h", NULL, &output)){
		//error
		SAFE_DELETE(checker->thread);
		return 1;
	}
	wxString wxOutput = output;
#ifdef _M_IX86
	wxString commandname = (checker->updateStable) ? "StableRelease: " : "VersionKainoteX86: ";
	size_t found = wxOutput.find(commandname);
#else
	wxString commandname = (checker->updateStable) ? "StableRelease: " : "VersionKainote ";
	size_t found = wxOutput.find(commandname);
#endif
	if (found == wxNOT_FOUND){
		SAFE_DELETE(checker->thread);
		return 1;
	}
	size_t foundn = wxOutput.find('\n', found);
	if (foundn == wxNOT_FOUND){
		SAFE_DELETE(checker->thread);
		return 1;
	}
	found += commandname.Len();
	wxString version = wxOutput.Mid(found, foundn - found);
	wxRegEx reg("<[^<>]*>", wxRE_ADVANCED);
	reg.ReplaceAll(&version, "");
	version.Replace("\"", "");
	wxString buildnum = version.AfterLast('.');
	int intbuild = wxAtoi(buildnum);
	int actualBuild = wxAtoi(wxString(VersionKainote).AfterLast('.'));
	if (actualBuild < intbuild){
		wxString link;
		if (checker->updateStable){
			wxString commandname = "StableReleaseLink: ";
			size_t found = wxOutput.find(commandname, foundn);
			if (found == wxNOT_FOUND){
				SAFE_DELETE(checker->thread);
				return 1;
			}
			found += commandname.Len();
			size_t foundn = wxOutput.find('\n', found);
			if (foundn == wxNOT_FOUND){
				SAFE_DELETE(checker->thread);
				return 1;
			}
			link = wxOutput.Mid(found, foundn - found);
#ifdef _M_IX86
			link.Replace("x64.zip", "x86.zip");
#endif // _M_IX86
		} else{
#ifdef _M_IX86
			link = "https://www.dropbox.com/s/zzz552tm6hq64oi/Kainote%20x86.zip?dl=1";
#else
			link = "https://www.dropbox.com/s/t8pkey94ruakyox/Kainote%20x64.zip?dl=1";
#endif // _M_IX86
		}
		link.Replace("https://", "");
		checker->server = link.BeforeFirst('/', &checker->page);
		int result = wxYES;
		if (!checker->dontAskForUpdate){
			KaiMessageDialog dlgmsg(0, wxString::Format(_("Dostępna jest nowa wersja programu %s zaktualizować?"), version), _("Aktualizacja"), wxYES_NO | wxOK | wxHELP);
			dlgmsg.SetHelpLabel(_("Wyłącz aktualizacje"));
			dlgmsg.SetOkLabel(_("Zakutalizuj po zamknięciu"));
			int result = dlgmsg.ShowModal();
		}
		if (result == wxYES){
			SAFE_DELETE(checker->thread);
			checker->checkOnClose = false;
			checker->Update(closeProgram);
			return 0;
		}
		else if (result == wxOK){
			checker->updateOnClose = true;
		}
		else if (result == wxHELP){
			Options.SetInt(UpdaterCheckIntensity, 6);
		}
	}
	SAFE_DELETE(checker->thread);
	return 0;
}

int UpdateChecker::Downloader(const wchar_t *server, const wchar_t *page, const wchar_t *filename, string *output)
{
	char szData[1024];
	bool writeToFile = filename != NULL;
	if (!output && !writeToFile)
		return 5;

	HANDLE hFile;
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

int UpdateChecker::DownloadZip()
{
	if (server.empty() || page.empty())
		return 6;

	savePath = Options.pathfull;
#ifdef _M_IX86
	savePath += "\\Kainote x86.zip";
#else
	savePath += "\\Kainote x64.zip";
#endif // _M_IX86
	return Downloader(server, page, savePath, NULL);
}

void UpdateChecker::Update(bool closeProgram /*= true*/)
{
	if (DownloadZip()){
		KaiMessageBox(_("Nie można pobrać nowej wersji Kainote"));
		return;
	}
	wxString updater = Options.pathfull + "\\Updater.exe";
	if (!wxFileExists(updater)){
		KaiMessageBox(_("Nie można znaleźć updatera Kainote"));
		return;
	}
	wxString ver = "Kainote v" + wxString(VersionKainote);
	wxWCharBuffer editorbuf = updater.c_str(), sfnamebuf = savePath.c_str(), versionK = ver.c_str();
	if (closeProgram){
		kainoteApp *Kaia = (kainoteApp *)wxTheApp;
		if (!Kaia){
			KaiMessageBox(_("Nie można zamknąć Kainote"));
			updateOnClose = true;
			return;
		}
		if (!Kaia->Frame->Close()){
			KaiMessageBox(_("Nie można zamknąć Kainote"));
			updateOnClose = true;
			return;
		}
	}
	wchar_t **cmdline = new wchar_t*[3];
	cmdline[0] = editorbuf.data();
	cmdline[1] = sfnamebuf.data();
	cmdline[2] = versionK.data();
	cmdline[3] = 0;
	long res = wxExecute(cmdline);
	delete[] cmdline;
}

bool UpdateChecker::CheckForUpdate()
{
	if (checkIntensity < 6){
		int lastCheck = Options.GetInt(UpdaterLastCheck);
		if (!lastCheck){
			SYSTEMTIME st;
			GetSystemTime(&st);
			lastCheck = st.wDay + (st.wMonth * 31) + (st.wYear * 365);
			Options.SetInt(UpdaterLastCheck, lastCheck);
			Options.SaveOptions(true, false);
			return false;
		}
		int templateCheck = (checkIntensity == 1) ? 2 : (checkIntensity == 2) ? 3 : (checkIntensity == 3) ? 7 :
			(checkIntensity == 4) ? 14 : 31;
		SYSTEMTIME st;
		GetSystemTime(&st);
		int today = st.wDay + (st.wMonth * 31) + (st.wYear * 365);
		if ((today - lastCheck) < templateCheck)
			return false;

		lastCheck = today;
		Options.SetInt(UpdaterLastCheck, lastCheck);
		Options.SaveOptions(true, false);
	}
	thread = new std::thread(UpdateChecker::CheckAsynchronously, this, true);
	return true;
}
