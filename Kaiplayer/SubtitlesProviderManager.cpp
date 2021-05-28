//  Copyright (c) 2020, Marcin Drob

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

#include "SubtitlesProvider.h"
#ifdef subsProvider

#include "Config.h"
//#include "Utils.h"

std::vector< SubtitlesProviderManager*> SubtitlesProviderManager::gs_Base;

SubtitlesProviderManager::~SubtitlesProviderManager()
{
	SAFE_DELETE(SP);
}

SubtitlesProvider *SubtitlesProviderManager::GetProvider()
{
	if (!SP){
		wxString provider = Options.GetString(VSFILTER_INSTANCE);
		if (provider == L"libass")
			SP = new SubtitlesLibass();
		else
			SP = new SubtitlesVSFilter();

	}
	return SP;
}

void SubtitlesProviderManager::GetProviders(wxArrayString *providerList)
{
	SubtitlesVSFilter::GetProviders(providerList);
	providerList->Add(L"libass");
}

void SubtitlesProviderManager::DestroyProviders()
{
	for (auto *spm : gs_Base) {
		SAFE_DELETE(spm->SP)
	}
}


void SubtitlesProviderManager::DestroySubsProvider()
{
	SubtitlesProvider::DestroySubtitlesProvider();
	//if there is some providers I may destroy it here
	for (auto *spm : gs_Base) {
		SAFE_DELETE(spm)
	}
	gs_Base.clear();
}

SubtitlesProviderManager *SubtitlesProviderManager::Get()
{
	auto * spm = new SubtitlesProviderManager();
	gs_Base.push_back(spm);
	return spm;
}

void SubtitlesProviderManager::Release()
{

	for (size_t i = 0; i < gs_Base.size(); i++) {
		auto *spm = gs_Base[i];
		if (spm == this) {
			delete spm;
			gs_Base.erase(gs_Base.begin() + i);
			return;
		}
	}
	//if it goes here it means that some alocation was outside base = memory leaks
	return;
}

void SubtitlesProviderManager::Draw(unsigned char* buffer, int time)
{
	GetProvider()->Draw(buffer, time);
}

bool SubtitlesProviderManager::Open(TabPanel *tab, int flag, wxString *text)
{
	return GetProvider()->Open(tab, flag, text);
}
//for styles preview
bool SubtitlesProviderManager::OpenString(wxString *text)
{
	return GetProvider()->OpenString(text);
}

void SubtitlesProviderManager::SetVideoParameters(const wxSize& size, unsigned char format, bool isSwapped)
{
	GetProvider()->SetVideoParameters(size, format, isSwapped);
}

bool SubtitlesProviderManager::IsLibass()
{
	return GetProvider()->IsLibass();
}

bool SubtitlesProviderManager::ReloadLibraries()
{
	wxString provider = Options.GetString(VSFILTER_INSTANCE);
	if (provider == L"libass") {
		if (gs_Base.size() > 0) {
			gs_Base[0]->GetProvider()->ReloadLibraries(true);
		}
		return true;
	}
	return false;
}

#endif