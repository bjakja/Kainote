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

SubtitlesProvider *SubtitlesProviderManager::SP = NULL;

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

void SubtitlesProviderManager::SetProvider(const wxString &provider)
{
	Options.SetString(VSFILTER_INSTANCE, provider);
	SAFE_DELETE(SP);
}

SubtitlesProviderManager::SubtitlesProviderManager()
{

}

void SubtitlesProviderManager::DestroySubsProvider()
{
	SubtitlesProvider::DestroySubtitlesProvider();
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

void SubtitlesProviderManager::SetVideoParameters(const wxSize& size, char bytesPerColor, bool isSwapped)
{
	GetProvider()->SetVideoParameters(size, bytesPerColor, isSwapped);
}

#endif