#ifndef CONFIG_H_INCLUDED
#define CONFIG_H_INCLUDED

//#pragma once
#include <wx/textfile.h>
#include <wx/tokenzr.h>
#include <wx/colour.h>
#include <wx/image.h>
#include <map>
#include <vector>
#include <algorithm>
#include "Styles.h"
#include <wx/utils.h> 
#undef wxBITMAP_PNG



#ifndef MIN
#define MIN(a,b) ((a)<(b))?(a):(b)
#endif

#ifndef MAX
#define MAX(a,b) ((a)>(b))?(a):(b)
#endif

#ifndef MID
#define MID(a,b,c) MAX((a),MIN((b),(c)))
#endif

class config
{
    private:
    std::map<wxString,wxString> rawcfg;
   

    public:
	std::vector<Styles*> assstore;
    wxString progname;
	//aktualny katalog --- œcie¿ka do folderu programu
    wxString acdir, pathfull;
    wxArrayString dirs;
	bool AudioOpts;


    wxString GetString(wxString lopt);
    bool GetBool(wxString lopt);
    wxColour GetColour(wxString lopt);
    int GetInt(wxString lopt);
    float GetFloat(wxString lopt);

    void SetString(wxString lopt, wxString sopt);
    void SetBool(wxString lopt, bool bopt);
    void SetColour(wxString lopt, wxColour copt);
    void SetInt(wxString lopt, int iopt);
    void SetFloat(wxString lopt, float fopt);
	void SetTable(wxString lopt, wxArrayString iopt,wxString split="|");
	void SetIntTable(wxString lopt, wxArrayInt iopt,wxString split="|");
	wxArrayString GetTable(wxString lopt,wxString split="|");
	wxArrayInt GetIntTable(wxString lopt, wxString split="|");
	void SetCoords(wxString lopt, int coordx, int coordy);
	void GetCoords(wxString lopt, int* coordx, int* coordy);
    wxString GetRawOptions(bool Audio=false);
    void AddStyle(Styles *styl);
    void ChangeStyle(Styles *styl,int i);
    Styles *GetStyle(int i,wxString name=_T(""), Styles* styl=NULL);
    int FindStyle(wxString name, int *multiplication=NULL);
    void DelStyle(int i);
    int StoreSize();
    void CatchValsLabs(wxString rawoptions);
    bool SetRawOptions(wxString textconfig);
    int LoadOptions();
	bool LoadAudioOpts();
	void SaveAudioOpts();
    void SaveOptions(bool cfg=true, bool style=true);
    void LoadStyles(wxString katalog);
    void clearstyles();
	void Sortstyles();
    config();
    ~config();

};
bool sortfunc(Styles *styl1,Styles *styl2);
//formatowanie w tym przypadku wygl¹da tak, 
//liczba która mówi ile cyfr przed przecinkiem i ile po, np 5.3f;
wxString getfloat(float num, wxString format="5.3f");
wxBitmap CreateBitmapFromPngResource(const wxString& t_name);
wxImage CreateImageFromPngResource(const wxString& t_name);
#define wxBITMAP_PNG(x) CreateBitmapFromPngResource(x)


enum{
	ASS=1,
	SRT,
	TMP,
	MDVD,
	MPL2
};

extern config Options;

#endif // CONFIG_H_INCLUDED
