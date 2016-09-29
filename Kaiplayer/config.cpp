#include "Config.h"
#include "OpennWrite.h"
#include <wx/stdpaths.h>
#include <wx/dir.h>
#include <wx/string.h>
#include <wx/log.h>
#include "VersionKainote.h"
#include <wx/mstream.h>
#include <wx/bitmap.h>
#include <windows.h>


config::config()
{
	progname=_T("Kainote v")+wxString(VersionKainote)+"  "+wxString(__DATE__)+"  "+wxString(__TIME__);
#if _DEBUG
	progname+= " DEBUG";
#endif
	AudioOpts=false;
}


config::~config()
{
	rawcfg.clear();
	for(std::vector<Styles*>::iterator it=assstore.begin(); it!=assstore.end(); it++)
	{
		delete (*it);
	}
	assstore.clear();
}



bool config::SetRawOptions(wxString textconfig)
{
	wxStringTokenizer cfg(textconfig,_T("\n"));
	int g=0;
	while(cfg.HasMoreTokens())
	{
		wxString token=cfg.NextToken();
		token.Trim(false);
		token.Trim(true);
		if (token.Len()>0){CatchValsLabs(token);g++;}
	}
	if (g>10){return true;};
	return false;
}

wxString config::GetString(wxString lopt)
{
	//std::map<wxString,wxString>::iterator it = rawcfg.find(lopt);
	//if(it!=rawcfg.end()){return it->second;}
	return rawcfg[lopt];
}


bool config::GetBool(wxString lopt)
{   wxString opt=rawcfg[lopt];
if(opt==_T("true")){return true;}
return false;
}

wxColour config::GetColour(wxString lopt)
{
	return wxString(rawcfg[lopt]);
}

int config::GetInt(wxString lopt)
{
	//wxString intopt=rawcfg[lopt];
	//if(intopt=="")return 0;
	return wxAtoi(rawcfg[lopt]);
}
float config::GetFloat(wxString lopt)
{   double fl;
wxString rawfloat=rawcfg[lopt];
//if(rawfloat=="")return 0.0;
if(!rawfloat.ToDouble(&fl)){return 0.0;}
return fl;
}

void config::SetString(wxString lopt, wxString sopt)
{
	rawcfg[lopt]=sopt;
}

void config::SetBool(wxString lopt, bool bopt)
{
	wxString bopt1 = (bopt)? _T("true") : _T("false");
	rawcfg[lopt]=bopt1;
}

void config::SetColour(wxString lopt, wxColour copt)
{
	wxString copt1 = copt.GetAsString(wxC2S_HTML_SYNTAX);
	rawcfg[lopt]=copt1;
}

void config::SetInt(wxString lopt, int iopt)
{
	wxString iopt1=_T("");
	rawcfg[lopt]=iopt1<<iopt;
}

void config::SetFloat(wxString lopt, float fopt)
{
	wxString fopt1=_T("");
	fopt1<<fopt;
	fopt1.Replace(",",".");
	rawcfg[lopt]=fopt1;
}

wxString config::GetRawOptions(bool Audio)
{
	wxString TextOpt=_T("["+progname+"]\r\n");
	for (std::map<wxString,wxString>::iterator cur=rawcfg.begin();cur!=rawcfg.end();cur++) {
		if((!Audio && cur->first.StartsWith("Audio")) || (Audio && !cur->first.StartsWith("Audio"))) {continue;}
		TextOpt<<cur->first << _T("=") << cur->second << _T("\r\n");
	}
	return TextOpt;
}

void config::CatchValsLabs(wxString line)
{
	wxString Values=line.AfterFirst('=');
	Values.Trim(false);
	Values.Trim(true);
	wxString Labels=line.BeforeFirst('=');
	Labels.Trim(false);
	Labels.Trim(true);
	rawcfg[Labels]=Values;
}
void config::AddStyle(Styles *styl)
{
	assstore.push_back(styl);

}

Styles *config::GetStyle(int i,wxString name, Styles* _styl)
{
	if(name!=_T("")){
		for(unsigned int j=0;j<assstore.size();j++){
			if(name==assstore[j]->Name){if(_styl){_styl=assstore[j];} return assstore[j];}
		}

	}
	return assstore[i];
}

int config::FindStyle(wxString name, int *multiplication)
{
	int isfound=-1;
	for(unsigned int j=0;j<assstore.size();j++)
	{
		if(name==assstore[j]->Name){isfound=j;
		if(multiplication){
			*multiplication++;
		}else{break;}
		}
	}
	return isfound;
}

int config::StoreSize()
{
	return assstore.size();
}

void config::ChangeStyle(Styles *styl,int i)
{
	Styles *style= assstore[i];
	delete style;
	assstore[i]=styl;
}

void config::DelStyle(int i)
{
	Styles *styl= assstore[i];
	delete styl;
	assstore.erase(assstore.begin()+i);
}

void config::SaveOptions(bool cfg, bool style)
{
	OpenWrite ow;
	if(cfg){
		wxString textfile=GetRawOptions();
		wxString path;
		path<<pathfull<<_T("\\Config.txt");
		ow.FileWrite(path, textfile);
	}

	if(style){
		wxString stylefile;
		for (int j=0;j<StoreSize();j++){
			stylefile<<GetStyle(j)->styletext();
		}
		wxString path;
		path<<pathfull<<_T("\\Catalog\\")<<acdir<<_T(".sty");
		ow.FileWrite(path, stylefile);
	}
}

int config::LoadOptions()
{
	wxStandardPathsBase &paths = wxStandardPaths::Get();
	pathfull=paths.GetExecutablePath().BeforeLast('\\');
	wxString path;
	path<<pathfull<<_T("\\Config.txt");
	OpenWrite ow;
	wxString txt=ow.FileOpen(path,false);
	bool checkVer=true;
	if(txt==_T("")){
		txt = "["+progname+"]\r\nChange Time=2000\r\n"\
			"Change mode=0\r\n"\
			"Default FPS=23.976\r\n"\
			"Default Style=Default\r\n"\
			"Editbox Spellchecker=true\r\n"\
			"Frames=false\r\n"\
			"Grid Active Line=#CA0065\r\n"\
			"Grid Background=#C0C0C0\r\n"\
			"Grid Comment=#D8DEF5\r\n"\
			"Grid Dialogue=#C0C0C0\r\n"\
			"Grid Collisions=#FF0000\r\n"\
			"Grid Font Name=Tahoma\r\n"\
			"Grid Font Size=10\r\n"\
			"Grid Label Normal=#19B3EC\r\n"\
			"Grid Label Modified=#FBF804\r\n"\
			"Grid Label Saved=#C4ECC9\r\n"\
			"Grid Lines=#808080\r\n"\
			"Grid Selected Comment=#D3EEEE\r\n"\
			"Grid Selected Dialogue=#CEFFE7\r\n"\
			"Grid Text=#000000\r\n"\
			"Move time forward=true\r\n"\
			"New end times=false\r\n"\
			"Show settings window=false\r\n"\
			"Start end times=0\r\n"\
			"Styles of time change=\r\n"\
			"Time show of letter=110\r\n"\
			"Grid Spellchecker=#FA9292\r\n"\
			"Video Prog Bar=true\r\n"\
			"Show Editor=true\r\n"\
			"Window Size=800,600\r\n"\
			"Video Window Size=500,350\r\n"\
			"Preview Text=Podgl¹d\r\n"\
			"Offset of start time=0\r\n"\
			"Offset of end time=0\r\n"\
			"Convert Resolution W=1280\r\n"\
			"Convert Resolution H=720\r\n"\
			"Move Video To Active Line=0\r\n"\
			"Play Afrer Selection=0\r\n"\
			"Grid tag changing char = *";
	}else{
		wxString ver= txt.BeforeFirst(']').Mid(1);
		if(ver!=progname){checkVer=false;}
	}
	bool isgood=SetRawOptions(txt.AfterFirst('\n'));
	acdir = _T("Default");
	path=_T("");
	path<<pathfull<<_T("\\Catalog\\");
	wxDir kat(path);
	if(!kat.IsOpened()){ow.FileWrite(path<<acdir<<_T(".sty"),_T("Style: Default,Garamond,30,&H00FFFFFF,&H000000FF,&H00FF0000,&H00000000,0,0,0,0,100,100,0,0,0,2,2,2,10,10,10,1"));
	AddStyle(new Styles());dirs.Add(acdir);
	}
	else{
		wxArrayString tmp;kat.GetAllFiles(path,&tmp,_T(""), wxDIR_FILES);
		for(size_t i=0;i<tmp.GetCount();i++){
			wxString fullpath=tmp[i].AfterLast('\\');
			if(fullpath.EndsWith(_T(".sty"))){dirs.Add(fullpath.BeforeLast('.'));}
		}
	}
	LoadStyles(acdir);
	return isgood;
}

void config::LoadStyles(wxString katalog)
{
	acdir=katalog;
	wxString path;
	path<<pathfull<<_T("\\Catalog\\")<<katalog<<_T(".sty");
	OpenWrite ow;
	for(std::vector<Styles*>::iterator it=assstore.begin(); it!=assstore.end(); it++){
		delete (*it);
	}
	assstore.clear();
	wxString stylee=ow.FileOpen(path,false);
	if(stylee!=_T("")){
		wxStringTokenizer cfg(stylee,_T("\n"));
		while(cfg.HasMoreTokens()){
			wxString token=cfg.NextToken();
			if (token.StartsWith(_T("Style: "))){
				AddStyle(new Styles(token));
			}
		}
	}
}

void config::clearstyles()
{
	for(std::vector<Styles*>::iterator it=assstore.begin(); it!=assstore.end(); it++){
		delete (*it);
	}
	assstore.clear();
}

void config::SetCoords(wxString lopt, int coordx, int coordy)
{
	wxString iopt1=_T("");
	rawcfg[lopt]=iopt1<<coordx<<","<<coordy;
}

void config::GetCoords(wxString lopt, int *coordx, int *coordy)
{
	wxString sopt=rawcfg[lopt];
	*coordx=wxAtoi(sopt.BeforeFirst(','));
	*coordy=wxAtoi(sopt.AfterFirst(','));
}

void config::SetTable(wxString lopt, wxArrayString asopt,wxString split)
{
	wxString sresult;
	wxString ES;
	for(size_t i=0;i<asopt.size();i++)
	{
		wxString endchar=(i==asopt.size()-1)? ES : split;
		sresult<<asopt[i]<<endchar;
	}
	rawcfg[lopt]=sresult;
}

void config::SetIntTable(wxString lopt, wxArrayInt asopt,wxString split)
{
	wxString sresult;
	wxString ES;
	for(size_t i=0;i<asopt.size();i++)
	{
		wxString endchar=(i==asopt.size()-1)? ES : split;
		sresult<<asopt[i]<<endchar;
	}
	rawcfg[lopt]=sresult;
}

wxArrayString config::GetTable(wxString lopt, wxString split)
{
	wxArrayString strcfg;
	wxString strtbl=rawcfg[lopt];
	if(strtbl!=""){
		wxStringTokenizer cfgtable(strtbl,split,wxTOKEN_STRTOK);
		while(cfgtable.HasMoreTokens()){
			strcfg.Add(cfgtable.NextToken());
		}  
	}
	return strcfg;
}

wxArrayInt config::GetIntTable(wxString lopt, wxString split)
{
	wxArrayInt intcfg;
	wxString strtbl=rawcfg[lopt];
	if(strtbl!=""){
		wxStringTokenizer cfgtable(strtbl,split,wxTOKEN_STRTOK);
		while(cfgtable.HasMoreTokens()){
			intcfg.Add(wxAtoi(cfgtable.NextToken()));
		}  
	}
	return intcfg;
}

bool sortfunc(Styles *styl1,Styles *styl2){
	wxString str1=styl1->Name;
	wxString str2=styl2->Name;
	return (str1.CmpNoCase(str2)<0);
}

void config::Sortstyles()
{
	std::sort(assstore.begin(),assstore.end(),sortfunc);
}

bool config::LoadAudioOpts()
{
	OpenWrite ow;
	wxString txt=ow.FileOpen(pathfull+_T("\\AudioConfig.txt"),false);

	if(txt==_T("")){
		txt="Audio Autocommit=true\r\n"\
			"Audio Autofocus=true\r\n"\
			"Audio Autoscroll=true\r\n"\
			"Audio Background=#000000\r\n"\
			"Audio Box Height=169\r\n"\
			"Audio Draw Cursor Time=true\r\n"\
			"Audio Draw Keyframes=true\r\n"\
			"Audio Draw Secondary Lines=true\r\n"\
			"Audio Draw Selection Background=true\r\n"\
			"Audio Draw video Position=true\r\n"\
			"Audio Grab Times On Select=true\r\n"\
			"Audio Inactive Lines Display Mode=1\r\n"\
			"Audio Keyframes=#C200FF\r\n"\
			"Audio Lead In=200\r\n"\
			"Audio Lead Out=300\r\n"\
			"Audio Line Boundaries Thickness=2\r\n"\
			"Audio Line Boundary End=#E67D00\r\n"\
			"Audio Line Boundary Inactive Line=#808080\r\n"\
			"Audio Line Boundary Start=#D80000\r\n"\
			"Audio Line Boundary Mark=#FF00FF\r\n"\
			"Audio Link=false\r\n"\
			"Audio Lock Scroll On Cursor=false\r\n"\
			"Audio Mark Play Time=1000\r\n"\
			"Audio Next Line On Commit=true\r\n"\
			"Audio Play Cursor=#FFFFFF\r\n"\
			"Audio RAM Cache=false\r\n"\
			"Audio Sample Rate=0\r\n"\
			"Audio Seconds Boundaries=#0064FF\r\n"\
			"Audio Selection Background=#404040\r\n"\
			"Audio Selection Background Modified=#5C0000\r\n"\
			"Audio Snap To Keyframes=false\r\n"\
			"Audio Snap To Other Lines=false\r\n"\
			"Audio Spectrum=false\r\n"\
			"Audio Start Drag Sensitivity=2\r\n"\
			"Audio Syllable Boundaries=#FFFF00\r\n"\
			"Audio Syllable Text=#FF0000\r\n"\
			"Audio Waveform=#00C800\r\n"\
			"Audio Waveform Inactive=#005000\r\n"\
			"Audio Waveform Modified=#FFE6E6\r\n"\
			"Audio Waveform Selected=#FFFFFF\r\n"\
			"Audio Wheel Default To Zoom=false";
	}
	return (AudioOpts=SetRawOptions(txt.AfterFirst('\n')));
}

void config::SaveAudioOpts()
{
	OpenWrite ow;
	ow.FileWrite(pathfull+_T("\\AudioConfig.txt"), GetRawOptions(true));
}


wxString getfloat(float num, wxString format)
{
	wxString strnum=wxString::Format(_T("%"+format),num);
	//if(strnum.find('.')!= -1){return strnum.Trim(false);}
	int rmv=0;
	bool trim=false;
	for(int i=strnum.Len()-1;i>0;i--)
	{
		if(strnum[i]=='0'){rmv++;}//&&!trim
		//else if(strnum[i]=='9'){rmv++;trim=true;}
		else if(strnum[i]=='.'){rmv++;break;}//}if(!trim){
		else{/*if(trim){int tmpc=static_cast < int >(strnum.GetChar(i));tmpc++;strnum[i]=(wxUniChar)tmpc;}*/break;}
	}
	if(rmv){strnum.RemoveLast(rmv);}
	return strnum.Trim(false);
}


bool LoadDataFromResource(char*& t_data, DWORD& t_dataSize, const wxString& t_name)
{
	bool     r_result    = false;
	HGLOBAL  a_resHandle = 0;
	HRSRC    a_resource;

	a_resource = FindResource(0, t_name.wchar_str(), RT_RCDATA);

	if(0 != a_resource)
	{
		a_resHandle = LoadResource(NULL, a_resource);
		if (0 != a_resHandle)
		{
			t_data = (char*)LockResource(a_resHandle);
			t_dataSize = SizeofResource(NULL, a_resource);
			r_result = true;
		}
	}

	return r_result;
}


wxBitmap GetBitmapFromMemory(const char* t_data, const DWORD t_size)
{
	wxMemoryInputStream a_is(t_data, t_size);
	return wxBitmap(wxImage(a_is, wxBITMAP_TYPE_PNG, -1), -1);
}

wxBitmap CreateBitmapFromPngResource(const wxString& t_name)
{
	wxBitmap   r_bitmapPtr;

	char*       a_data      = 0;
	DWORD       a_dataSize  = 0;

	if(LoadDataFromResource(a_data, a_dataSize, t_name))
	{
		r_bitmapPtr = GetBitmapFromMemory(a_data, a_dataSize);
	}

	return r_bitmapPtr;
}

wxImage CreateImageFromPngResource(const wxString& t_name)
{
	wxImage   image;

	char*       a_data      = 0;
	DWORD       a_dataSize  = 0;

	if(LoadDataFromResource(a_data, a_dataSize, t_name))
	{
		wxMemoryInputStream a_is(a_data, a_dataSize);
		image=wxImage(a_is, wxBITMAP_TYPE_PNG, -1);
	}

	return image;
}

config Options;




