#include "Locale.h"
#include "config.h"

Locale::Locale(int language)
{
	locale=new wxLocale;
	if(!locale->Init(language)){
		wxMessageBox("wxLocale cannot initialize, language changing failed");
	}
    locale->AddCatalogLookupPathPrefix(Options.pathfull+"\\Locale");
    locale->AddCatalog("Locale");
}
	
Locale::~Locale()
{
	wxDELETE(locale);
}