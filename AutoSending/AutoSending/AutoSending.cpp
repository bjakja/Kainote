// AutoSending.cpp : Defines the entry point for the console application.
//

//#include "stdafx.h"
#include <stdio.h>
#include <tchar.h>
#include <windows.h>
#include <fstream>
#include <iostream>
#include <vector>

#include <contrib/minizip/zip.h>
#if _DEBUG
	#include <vld.h>
#endif

int check_exist_file(const wchar_t* filename)
{
	FILE* ftestexist;
	int ret = 1;
	ftestexist = _wfopen(filename, L"rb");
	if (ftestexist == NULL)
		ret = 0;
	else
		fclose(ftestexist);
	return ret;
}

bool ConvertAndPrepend(wchar_t *source, char *prepend, char **dest){
	size_t sourcelen = wcslen(source);
	int result = WideCharToMultiByte(CP_UTF8, 0, source, sourcelen, 0, 0, 0, 0);
	if (!result)
		return false;
	size_t prependSize = strlen(prepend);
	*dest = new char[result + prependSize + 1];
	if (*dest == NULL){ return false; }
	strcpy(*dest, prepend);
	int copied = WideCharToMultiByte(CP_UTF8, 0, source, sourcelen, &(*dest)[prependSize], result, 0, 0);
	if (copied != result){
		delete[] (*dest);
		*dest = NULL;
		return false;
	}
	(*dest)[result + prependSize] = 0;
	return true;
}

#define CHUNK 16384

int _tmain(int argc, TCHAR* argv[])
{
	if (argc<4){ 
		std::cerr << "Too few arguments"; 
		for (int i = 0; i < argc; i++){
			std::wcout << argv[i] << "\r\n";
		}
		return 1; 
	}
	bool isX86 = wcscmp(argv[3], L"x86") == 0;
	char * prependFolder = (isX86) ? "Kainote" : "Kainote_x64";
	wchar_t * path = argv[1];
	if (!check_exist_file(path)){ std::wcerr << L"Kainote zip path don't exist " << path; return 1; }

	/*bool isX86 = false;
	char * prependFolder = (isX86) ? "Kainote" : "Kainote_x64";
	wchar_t * path = (isX86) ? L"H:\\Kainote\\Win32\\Release\\Kainote x86.zip" : L"H:\\Kainote\\x64\\Release\\Kainote x64.zip";*/

	int filenamessize = (isX86) ? 138 : 140;//tyle policzy³ visual studio
	wchar_t * filenames[] = { 
		L"\\Automation\\automation\\Autoload\\Aegisub-Motion.moon\0",
		L"\\Automation\\automation\\Autoload\\cleantags-autoload.lua\0",
		L"\\Automation\\automation\\Autoload\\gradient-factory-1.3.lua\0",
		L"\\Automation\\automation\\Autoload\\kara-templater.lua\0",
		L"\\Automation\\automation\\Autoload\\l0.DependencyControl.Toolbox.moon\0",
		L"\\Automation\\automation\\Autoload\\lyger.CircleText.lua\0",
		L"\\Automation\\automation\\Autoload\\lyger.ClipGrad.lua\0",
		L"\\Automation\\automation\\Autoload\\lyger.GradientByChar.lua\0",
		L"\\Automation\\automation\\Autoload\\lyger.GradientEverything.moon\0",
		L"\\Automation\\automation\\Autoload\\lyger.ModifyMocha.lua\0",
		L"\\Automation\\automation\\Autoload\\qc.lua\0",
		L"\\Automation\\automation\\Autoload\\strip-tags.lua\0",
		L"\\Automation\\automation\\Autoload\\ua.Colorize.lua\0",
		L"\\Automation\\automation\\Autoload\\ua.HYDRA.lua\0",
		L"\\Automation\\automation\\Autoload\\ua.Recalculator.lua\0",
		L"\\Automation\\automation\\Include\\a-mo\\ConfigHandler.moon\0",
		L"\\Automation\\automation\\Include\\a-mo\\DataHandler.moon\0",
		L"\\Automation\\automation\\Include\\a-mo\\DataWrapper.moon\0",
		L"\\Automation\\automation\\Include\\a-mo\\Line.moon\0",
		L"\\Automation\\automation\\Include\\a-mo\\LineCollection.moon\0",
		L"\\Automation\\automation\\Include\\a-mo\\Log.moon\0",
		L"\\Automation\\automation\\Include\\a-mo\\Math.moon\0",
		L"\\Automation\\automation\\Include\\a-mo\\MotionHandler.moon\0",
		L"\\Automation\\automation\\Include\\a-mo\\ShakeShapeHandler.moon\0",
		L"\\Automation\\automation\\Include\\a-mo\\Statistics.moon\0",
		L"\\Automation\\automation\\Include\\a-mo\\Tags.moon\0",
		L"\\Automation\\automation\\Include\\a-mo\\Transform.moon\0",
		L"\\Automation\\automation\\Include\\a-mo\\TrimHandler.moon\0",
		L"\\Automation\\automation\\Include\\aegisub\\argcheck.moon\0",
		L"\\Automation\\automation\\Include\\aegisub\\clipboard.lua\0",
		L"\\Automation\\automation\\Include\\aegisub\\ffi.moon\0",
		L"\\Automation\\automation\\Include\\aegisub\\lfs.moon\0",
		L"\\Automation\\automation\\Include\\aegisub\\re.moon\0",
		L"\\Automation\\automation\\Include\\aegisub\\unicode.moon\0",
		L"\\Automation\\automation\\Include\\aegisub\\util.moon\0",
		L"\\Automation\\automation\\Include\\bakukara.lua\0",
		L"\\Automation\\automation\\Include\\BM\\BadMutex.lua\0",
		L"\\Automation\\automation\\Include\\BM\\BadMutex\\BadMutex.dll\0",
		L"\\Automation\\automation\\Include\\cleantags.lua\0",
		L"\\Automation\\automation\\Include\\clipboard.lua\0",
		L"\\Automation\\automation\\Include\\DM\\DownloadManager.lua\0",
		L"\\Automation\\automation\\Include\\DM\\DownloadManager\\DownloadManager.dll\0",
		L"\\Automation\\automation\\Include\\json.lua\0",
		L"\\Automation\\automation\\Include\\json\\decode.lua\0",
		L"\\Automation\\automation\\Include\\json\\decode\\composite.lua\0",
		L"\\Automation\\automation\\Include\\json\\decode\\number.lua\0",
		L"\\Automation\\automation\\Include\\json\\decode\\others.lua\0",
		L"\\Automation\\automation\\Include\\json\\decode\\state.lua\0",
		L"\\Automation\\automation\\Include\\json\\decode\\strings.lua\0",
		L"\\Automation\\automation\\Include\\json\\decode\\util.lua\0",
		L"\\Automation\\automation\\Include\\json\\encode.lua\0",
		L"\\Automation\\automation\\Include\\json\\encode\\array.lua\0",
		L"\\Automation\\automation\\Include\\json\\encode\\calls.lua\0",
		L"\\Automation\\automation\\Include\\json\\encode\\number.lua\0",
		L"\\Automation\\automation\\Include\\json\\encode\\object.lua\0",
		L"\\Automation\\automation\\Include\\json\\encode\\others.lua\0",
		L"\\Automation\\automation\\Include\\json\\encode\\output.lua\0",
		L"\\Automation\\automation\\Include\\json\\encode\\output_utility.lua\0",
		L"\\Automation\\automation\\Include\\json\\encode\\strings.lua\0",
		L"\\Automation\\automation\\Include\\json\\util.lua\0",
		L"\\Automation\\automation\\Include\\karahelper.lua\0",
		L"\\Automation\\automation\\Include\\karaskel-auto4.lua\0",
		L"\\Automation\\automation\\Include\\karaskel.lua\0",
		L"\\Automation\\automation\\Include\\l0\\ASSFoundation.moon\0",
		L"\\Automation\\automation\\Include\\l0\\ASSFoundation\\Base.lua\0",
		L"\\Automation\\automation\\Include\\l0\\ASSFoundation\\ClassFactory.lua\0",
		L"\\Automation\\automation\\Include\\l0\\ASSFoundation\\Common.lua\0",
		L"\\Automation\\automation\\Include\\l0\\ASSFoundation\\Draw\\Bezier.lua\0",
		L"\\Automation\\automation\\Include\\l0\\ASSFoundation\\Draw\\Close.lua\0",
		L"\\Automation\\automation\\Include\\l0\\ASSFoundation\\Draw\\CommandBase.lua\0",
		L"\\Automation\\automation\\Include\\l0\\ASSFoundation\\Draw\\Contour.lua\0",
		L"\\Automation\\automation\\Include\\l0\\ASSFoundation\\Draw\\DrawingBase.lua\0",
		L"\\Automation\\automation\\Include\\l0\\ASSFoundation\\Draw\\Line.lua\0",
		L"\\Automation\\automation\\Include\\l0\\ASSFoundation\\FoundationMethods.lua\0",
		L"\\Automation\\automation\\Include\\l0\\ASSFoundation\\LineBounds.lua\0",
		L"\\Automation\\automation\\Include\\l0\\ASSFoundation\\LineContents.lua\0",
		L"\\Automation\\automation\\Include\\l0\\ASSFoundation\\Primitive\\Number.lua\0",
		L"\\Automation\\automation\\Include\\l0\\ASSFoundation\\Primitive\\Point.lua\0",
		L"\\Automation\\automation\\Include\\l0\\ASSFoundation\\Primitive\\String.lua\0",
		L"\\Automation\\automation\\Include\\l0\\ASSFoundation\\Primitive\\Time.lua\0",
		L"\\Automation\\automation\\Include\\l0\\ASSFoundation\\Section\\Comment.lua\0",
		L"\\Automation\\automation\\Include\\l0\\ASSFoundation\\Section\\Drawing.lua\0",
		L"\\Automation\\automation\\Include\\l0\\ASSFoundation\\Section\\Tag.lua\0",
		L"\\Automation\\automation\\Include\\l0\\ASSFoundation\\Section\\Text.lua\0",
		L"\\Automation\\automation\\Include\\l0\\ASSFoundation\\Tag\\Align.lua\0",
		L"\\Automation\\automation\\Include\\l0\\ASSFoundation\\Tag\\Base.lua\0",
		L"\\Automation\\automation\\Include\\l0\\ASSFoundation\\Tag\\ClipRect.lua\0",
		L"\\Automation\\automation\\Include\\l0\\ASSFoundation\\Tag\\ClipVect.lua\0",
		L"\\Automation\\automation\\Include\\l0\\ASSFoundation\\Tag\\Color.lua\0",
		L"\\Automation\\automation\\Include\\l0\\ASSFoundation\\Tag\\Fade.lua\0",
		L"\\Automation\\automation\\Include\\l0\\ASSFoundation\\Tag\\Indexed.lua\0",
		L"\\Automation\\automation\\Include\\l0\\ASSFoundation\\Tag\\Move.lua\0",
		L"\\Automation\\automation\\Include\\l0\\ASSFoundation\\Tag\\String.lua\0",
		L"\\Automation\\automation\\Include\\l0\\ASSFoundation\\Tag\\Toggle.lua\0",
		L"\\Automation\\automation\\Include\\l0\\ASSFoundation\\Tag\\Transform.lua\0",
		L"\\Automation\\automation\\Include\\l0\\ASSFoundation\\Tag\\Weight.lua\0",
		L"\\Automation\\automation\\Include\\l0\\ASSFoundation\\TagList.lua\0",
		L"\\Automation\\automation\\Include\\l0\\DependencyControl.moon\0",
		L"\\Automation\\automation\\Include\\l0\\DependencyControl\\ConfigHandler.moon\0",
		L"\\Automation\\automation\\Include\\l0\\DependencyControl\\FileOps.moon\0",
		L"\\Automation\\automation\\Include\\l0\\DependencyControl\\Logger.moon\0",
		L"\\Automation\\automation\\Include\\l0\\DependencyControl\\UnitTestSuite.moon\0",
		L"\\Automation\\automation\\Include\\l0\\DependencyControl\\UpdateFeed.moon\0",
		L"\\Automation\\automation\\Include\\l0\\DependencyControl\\Updater.moon\0",
		L"\\Automation\\automation\\Include\\l0\\Functional.moon\0",
		L"\\Automation\\automation\\Include\\lfs.lua\0",
		L"\\Automation\\automation\\Include\\lyger\\LibLyger.moon\0",
		L"\\Automation\\automation\\Include\\moonscript.lua\0",
		L"\\Automation\\automation\\Include\\PT\\PreciseTimer.lua\0",
		L"\\Automation\\automation\\Include\\PT\\PreciseTimer\\PreciseTimer.dll\0",
		L"\\Automation\\automation\\Include\\re.lua\0",
		L"\\Automation\\automation\\Include\\requireffi\\requireffi.lua\0",
		L"\\Automation\\automation\\Include\\SubInspector\\Inspector.moon\0",
		L"\\Automation\\automation\\Include\\SubInspector\\Inspector\\SubInspector.dll\0",
		L"\\Automation\\automation\\Include\\unicode.lua\0",
		L"\\Automation\\automation\\Include\\utils.lua\0",
		L"\\Automation\\automation\\Include\\Yutils.lua\0",
		L"\\Automation\\automation\\packages\\karahelper.dll\0",
		//L"\\Automation\\automation\\tests\\DepUnit\\modules\\l0\0",
		L"\\Automation\\autosave\\txt.txt\0",
		//L"\\Automation\\config\0",
		//L"\\Automation\\feedDump\0",
		L"\\Automation\\log\\txt.txt\0",
		//L"\\Automation\\temp\0",
		//L"\\Automation\\tests\0",
		L"\\Dictionary\\en_US.aff\0",
		L"\\Dictionary\\en_US.dic\0",
		L"\\Dictionary\\pl.aff\0",
		L"\\Dictionary\\pl.dic\0",
		L"\\D3DX9_43.dll\0",
		L"\\ffms2.dll\0",
		L"\\Icons.dll\0",
		L"\\KaiNote.exe\0",
		L"\\KaiNote.pdb\0",
		L"\\LICENSE.txt\0",
		L"\\Locale\\en.mo\0",
		L"\\msvcp120.dll\0",
		L"\\msvcr120.dll\0",
		L"\\Themes\\DeepDark.txt\0",
		L"\\Themes\\DeepLight.txt\0",
		L"\\Themes\\MyDeepDark.txt\0",
		L"\\Csri\\VSFiltermod.dll\0",
		L"\\Csri\\xy-VSFilter_kainote.dll\0",
		L"\\KaiNote_AVX.exe\0",
		L"\\KaiNote_AVX.pdb\0"
	};
	
	zipFile zf = zipOpen64(path, APPEND_STATUS_CREATE);
	if (zf == NULL)
		return 1;
	wchar_t * pch = wcsrchr(path, '\\');
	wchar_t zipdir[4000];
	size_t newstart = pch - path;
	wcsncpy(zipdir, path, newstart);
	//strcpy(&zipdir[newstart], "\\");
	bool _return = true;
	char buffer[CHUNK];

	for (int i = 0; i < filenamessize; i++){
		wchar_t * fn = filenames[i];
		//wchar_t * pch1 = wcschr(fn, '\\');
		//size_t newnewstart = pch1 - fn + 1;
		wcscpy(&zipdir[newstart], fn);
		char * charfn = NULL;
		if (!ConvertAndPrepend(fn, prependFolder, &charfn)){
			_return = false;
			continue;
		}
		
		size_t size = 0;
		FILE *file = _wfopen(zipdir, L"rb");
		zip_fileinfo zfi = { 0 };
		HANDLE ffile = CreateFile(zipdir, GENERIC_READ, FILE_SHARE_READ, 0, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, 0);
		if (ffile!=INVALID_HANDLE_VALUE){
			FILETIME writeTime;
			GetFileTime(ffile, 0, 0, &writeTime);
			CloseHandle(ffile);
			FILETIME ftLocal;
			uLong *dt = &zfi.dosDate;
			FileTimeToLocalFileTime(&writeTime, &ftLocal);
			FileTimeToDosDateTime(&ftLocal, ((LPWORD)dt) + 1, ((LPWORD)dt) + 0);
		}
		if (!file){
			if (S_OK == zipOpenNewFileInZip(zf, charfn, &zfi, NULL, 0, NULL, 0, NULL, Z_DEFLATED, Z_DEFAULT_COMPRESSION)){
				if (zipCloseFileInZip(zf))
					_return = false;
				delete[] charfn;
				continue;
			}
		}
		else
		{
			if (S_OK == zipOpenNewFileInZip(zf, charfn, &zfi, NULL, 0, NULL, 0, NULL, Z_DEFLATED, Z_DEFAULT_COMPRESSION))
			{
				while (size = fread(buffer, 1, CHUNK, file))
				{
					
					if (zipWriteInFileInZip(zf, buffer, size))
						_return = false;

				}
				if (zipCloseFileInZip(zf))
					_return = false;
				delete[] charfn;
				fclose(file);
				continue;
			}
			_return = false;
			fclose(file);
		}
		delete[] charfn;
		_return = false;
	}
	if (zipClose(zf, NULL))
		return 3;

	if (!_return)
		return 4;
	
	//wchar_t *dbx = (isX86) ? L"C:\\Users\\Bakura\\Dropbox\\Kainote x86.zip\0" : L"C:\\Users\\Bakura\\Dropbox\\Kainote x64.zip\0";

	wchar_t * dbx = argv[2];

	std::ifstream src(path, std::ios::binary);
	std::ofstream dst(dbx, std::ios::binary);
	dst << src.rdbuf();
	
	std::cout << "End packing";
	return 0;
}

