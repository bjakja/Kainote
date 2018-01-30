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
//#include <vld.h>

int check_exist_file(const char* filename)
{
	FILE* ftestexist;
	int ret = 1;
	ftestexist = fopen(filename, "rb");
	if (ftestexist == NULL)
		ret = 0;
	else
		fclose(ftestexist);
	return ret;
}

//bool convert(wchar_t *source, char **dest){
//	int sourcelen = wcslen(source);
//	int result = WideCharToMultiByte(CP_ACP, 0, source, sourcelen, 0, 0, 0, 0);
//	if (!result)
//		return false;
//	*dest = new char[result + 1];
//	int copied = WideCharToMultiByte(CP_ACP, 0, source, sourcelen, *dest, result, 0, 0);
//	if (copied != result){
//		delete[] (*dest);
//		*dest = NULL;
//		return false;
//	}
//	(*dest)[result] = 0;
//	return true;
//}

#define CHUNK 16384

int _tmain(int argc, _TCHAR* argv[])
{
	if (argc<3){ 
		std::cerr << "Too few arguments"; 
		for (int i = 0; i < argc; i++){
			std::cout << argv[i] << "\r\n";
		}
		return 1; 
	}
	char * path = argv[1];//"H:\\Kainote\\x64\\Release\\Kainote x64.zip\0";
	if (!check_exist_file(path)){ std::cerr << "Kainote path don't exist " << path; return 1; }

	int filenamessize = 140;//tyle policzy³ visual studio
	char * filenames[] = { 
		"Kainote_x64\\Automation\\automation\\Autoload\\Aegisub-Motion.moon\0",
		"Kainote_x64\\Automation\\automation\\Autoload\\cleantags-autoload.lua\0",
		"Kainote_x64\\Automation\\automation\\Autoload\\gradient-factory-1.3.lua\0",
		"Kainote_x64\\Automation\\automation\\Autoload\\kara-templater.lua\0",
		"Kainote_x64\\Automation\\automation\\Autoload\\l0.DependencyControl.Toolbox.moon\0",
		"Kainote_x64\\Automation\\automation\\Autoload\\lyger.CircleText.lua\0",
		"Kainote_x64\\Automation\\automation\\Autoload\\lyger.ClipGrad.lua\0",
		"Kainote_x64\\Automation\\automation\\Autoload\\lyger.GradientByChar.lua\0",
		"Kainote_x64\\Automation\\automation\\Autoload\\lyger.GradientEverything.moon\0",
		"Kainote_x64\\Automation\\automation\\Autoload\\lyger.ModifyMocha.lua\0",
		"Kainote_x64\\Automation\\automation\\Autoload\\qc.lua\0",
		"Kainote_x64\\Automation\\automation\\Autoload\\strip-tags.lua\0",
		"Kainote_x64\\Automation\\automation\\Autoload\\ua.Colorize.lua\0",
		"Kainote_x64\\Automation\\automation\\Autoload\\ua.HYDRA.lua\0",
		"Kainote_x64\\Automation\\automation\\Autoload\\ua.Recalculator.lua\0",
		"Kainote_x64\\Automation\\automation\\Include\\a-mo\\ConfigHandler.moon\0",
		"Kainote_x64\\Automation\\automation\\Include\\a-mo\\DataHandler.moon\0",
		"Kainote_x64\\Automation\\automation\\Include\\a-mo\\DataWrapper.moon\0",
		"Kainote_x64\\Automation\\automation\\Include\\a-mo\\Line.moon\0",
		"Kainote_x64\\Automation\\automation\\Include\\a-mo\\LineCollection.moon\0",
		"Kainote_x64\\Automation\\automation\\Include\\a-mo\\Log.moon\0",
		"Kainote_x64\\Automation\\automation\\Include\\a-mo\\Math.moon\0",
		"Kainote_x64\\Automation\\automation\\Include\\a-mo\\MotionHandler.moon\0",
		"Kainote_x64\\Automation\\automation\\Include\\a-mo\\ShakeShapeHandler.moon\0",
		"Kainote_x64\\Automation\\automation\\Include\\a-mo\\Statistics.moon\0",
		"Kainote_x64\\Automation\\automation\\Include\\a-mo\\Tags.moon\0",
		"Kainote_x64\\Automation\\automation\\Include\\a-mo\\Transform.moon\0",
		"Kainote_x64\\Automation\\automation\\Include\\a-mo\\TrimHandler.moon\0",
		"Kainote_x64\\Automation\\automation\\Include\\aegisub\\argcheck.moon\0",
		"Kainote_x64\\Automation\\automation\\Include\\aegisub\\clipboard.lua\0",
		"Kainote_x64\\Automation\\automation\\Include\\aegisub\\ffi.moon\0",
		"Kainote_x64\\Automation\\automation\\Include\\aegisub\\lfs.moon\0",
		"Kainote_x64\\Automation\\automation\\Include\\aegisub\\lfs2.moon\0",
		"Kainote_x64\\Automation\\automation\\Include\\aegisub\\newlfs.lua\0",
		"Kainote_x64\\Automation\\automation\\Include\\aegisub\\re.moon\0",
		"Kainote_x64\\Automation\\automation\\Include\\aegisub\\unicode.moon\0",
		"Kainote_x64\\Automation\\automation\\Include\\aegisub\\util.moon\0",
		"Kainote_x64\\Automation\\automation\\Include\\bakukara.lua\0",
		"Kainote_x64\\Automation\\automation\\Include\\BM\\BadMutex.lua\0",
		"Kainote_x64\\Automation\\automation\\Include\\BM\\BadMutex\\BadMutex.dll\0",
		"Kainote_x64\\Automation\\automation\\Include\\BM\\BadMutex2.lua\0",
		"Kainote_x64\\Automation\\automation\\Include\\BM\\BadMutex2\\BadMutex.dll\0",
		"Kainote_x64\\Automation\\automation\\Include\\cleantags.lua\0",
		"Kainote_x64\\Automation\\automation\\Include\\clipboard.lua\0",
		"Kainote_x64\\Automation\\automation\\Include\\DM\\DownloadManager.lua\0",
		"Kainote_x64\\Automation\\automation\\Include\\DM\\DownloadManager\\DownloadManager.dll\0",
		"Kainote_x64\\Automation\\automation\\Include\\json.lua\0",
		"Kainote_x64\\Automation\\automation\\Include\\json\\decode.lua\0",
		"Kainote_x64\\Automation\\automation\\Include\\json\\decode\\composite.lua\0",
		"Kainote_x64\\Automation\\automation\\Include\\json\\decode\\number.lua\0",
		"Kainote_x64\\Automation\\automation\\Include\\json\\decode\\others.lua\0",
		"Kainote_x64\\Automation\\automation\\Include\\json\\decode\\state.lua\0",
		"Kainote_x64\\Automation\\automation\\Include\\json\\decode\\strings.lua\0",
		"Kainote_x64\\Automation\\automation\\Include\\json\\decode\\util.lua\0",
		"Kainote_x64\\Automation\\automation\\Include\\json\\encode.lua\0",
		"Kainote_x64\\Automation\\automation\\Include\\json\\encode\\array.lua\0",
		"Kainote_x64\\Automation\\automation\\Include\\json\\encode\\calls.lua\0",
		"Kainote_x64\\Automation\\automation\\Include\\json\\encode\\number.lua\0",
		"Kainote_x64\\Automation\\automation\\Include\\json\\encode\\object.lua\0",
		"Kainote_x64\\Automation\\automation\\Include\\json\\encode\\others.lua\0",
		"Kainote_x64\\Automation\\automation\\Include\\json\\encode\\output.lua\0",
		"Kainote_x64\\Automation\\automation\\Include\\json\\encode\\output_utility.lua\0",
		"Kainote_x64\\Automation\\automation\\Include\\json\\encode\\strings.lua\0",
		"Kainote_x64\\Automation\\automation\\Include\\json\\util.lua\0",
		"Kainote_x64\\Automation\\automation\\Include\\karahelper.lua\0",
		"Kainote_x64\\Automation\\automation\\Include\\karaskel-auto4.lua\0",
		"Kainote_x64\\Automation\\automation\\Include\\karaskel.lua\0",
		"Kainote_x64\\Automation\\automation\\Include\\l0\\ASSFoundation.moon\0",
		"Kainote_x64\\Automation\\automation\\Include\\l0\\ASSFoundation\\Base.lua\0",
		"Kainote_x64\\Automation\\automation\\Include\\l0\\ASSFoundation\\ClassFactory.lua\0",
		"Kainote_x64\\Automation\\automation\\Include\\l0\\ASSFoundation\\Common.lua\0",
		"Kainote_x64\\Automation\\automation\\Include\\l0\\ASSFoundation\\Draw\\Bezier.lua\0",
		"Kainote_x64\\Automation\\automation\\Include\\l0\\ASSFoundation\\Draw\\Close.lua\0",
		"Kainote_x64\\Automation\\automation\\Include\\l0\\ASSFoundation\\Draw\\CommandBase.lua\0",
		"Kainote_x64\\Automation\\automation\\Include\\l0\\ASSFoundation\\Draw\\Contour.lua\0",
		"Kainote_x64\\Automation\\automation\\Include\\l0\\ASSFoundation\\Draw\\DrawingBase.lua\0",
		"Kainote_x64\\Automation\\automation\\Include\\l0\\ASSFoundation\\Draw\\Line.lua\0",
		"Kainote_x64\\Automation\\automation\\Include\\l0\\ASSFoundation\\FoundationMethods.lua\0",
		"Kainote_x64\\Automation\\automation\\Include\\l0\\ASSFoundation\\LineBounds.lua\0",
		"Kainote_x64\\Automation\\automation\\Include\\l0\\ASSFoundation\\LineContents.lua\0",
		"Kainote_x64\\Automation\\automation\\Include\\l0\\ASSFoundation\\Primitive\\Number.lua\0",
		"Kainote_x64\\Automation\\automation\\Include\\l0\\ASSFoundation\\Primitive\\Point.lua\0",
		"Kainote_x64\\Automation\\automation\\Include\\l0\\ASSFoundation\\Primitive\\String.lua\0",
		"Kainote_x64\\Automation\\automation\\Include\\l0\\ASSFoundation\\Primitive\\Time.lua\0",
		"Kainote_x64\\Automation\\automation\\Include\\l0\\ASSFoundation\\Section\\Comment.lua\0",
		"Kainote_x64\\Automation\\automation\\Include\\l0\\ASSFoundation\\Section\\Drawing.lua\0",
		"Kainote_x64\\Automation\\automation\\Include\\l0\\ASSFoundation\\Section\\Tag.lua\0",
		"Kainote_x64\\Automation\\automation\\Include\\l0\\ASSFoundation\\Section\\Text.lua\0",
		"Kainote_x64\\Automation\\automation\\Include\\l0\\ASSFoundation\\Tag\\Align.lua\0",
		"Kainote_x64\\Automation\\automation\\Include\\l0\\ASSFoundation\\Tag\\Base.lua\0",
		"Kainote_x64\\Automation\\automation\\Include\\l0\\ASSFoundation\\Tag\\ClipRect.lua\0",
		"Kainote_x64\\Automation\\automation\\Include\\l0\\ASSFoundation\\Tag\\ClipVect.lua\0",
		"Kainote_x64\\Automation\\automation\\Include\\l0\\ASSFoundation\\Tag\\Color.lua\0",
		"Kainote_x64\\Automation\\automation\\Include\\l0\\ASSFoundation\\Tag\\Fade.lua\0",
		"Kainote_x64\\Automation\\automation\\Include\\l0\\ASSFoundation\\Tag\\Indexed.lua\0",
		"Kainote_x64\\Automation\\automation\\Include\\l0\\ASSFoundation\\Tag\\Move.lua\0",
		"Kainote_x64\\Automation\\automation\\Include\\l0\\ASSFoundation\\Tag\\String.lua\0",
		"Kainote_x64\\Automation\\automation\\Include\\l0\\ASSFoundation\\Tag\\Toggle.lua\0",
		"Kainote_x64\\Automation\\automation\\Include\\l0\\ASSFoundation\\Tag\\Transform.lua\0",
		"Kainote_x64\\Automation\\automation\\Include\\l0\\ASSFoundation\\Tag\\Weight.lua\0",
		"Kainote_x64\\Automation\\automation\\Include\\l0\\ASSFoundation\\TagList.lua\0",
		"Kainote_x64\\Automation\\automation\\Include\\l0\\DependencyControl.moon\0",
		"Kainote_x64\\Automation\\automation\\Include\\l0\\DependencyControl\\ConfigHandler.moon\0",
		"Kainote_x64\\Automation\\automation\\Include\\l0\\DependencyControl\\FileOps.moon\0",
		"Kainote_x64\\Automation\\automation\\Include\\l0\\DependencyControl\\Logger.moon\0",
		"Kainote_x64\\Automation\\automation\\Include\\l0\\DependencyControl\\UnitTestSuite.moon\0",
		"Kainote_x64\\Automation\\automation\\Include\\l0\\DependencyControl\\UpdateFeed.moon\0",
		"Kainote_x64\\Automation\\automation\\Include\\l0\\DependencyControl\\Updater.moon\0",
		"Kainote_x64\\Automation\\automation\\Include\\l0\\Functional.moon\0",
		"Kainote_x64\\Automation\\automation\\Include\\lfs.lua\0",
		"Kainote_x64\\Automation\\automation\\Include\\lyger\\LibLyger.moon\0",
		"Kainote_x64\\Automation\\automation\\Include\\moonscript.lua\0",
		"Kainote_x64\\Automation\\automation\\Include\\PT\\PreciseTimer.lua\0",
		"Kainote_x64\\Automation\\automation\\Include\\PT\\PreciseTimer\\PreciseTimer.dll\0",
		"Kainote_x64\\Automation\\automation\\Include\\re.lua\0",
		"Kainote_x64\\Automation\\automation\\Include\\requireffi\\requireffi.lua\0",
		"Kainote_x64\\Automation\\automation\\Include\\SubInspector\\Inspector.moon\0",
		"Kainote_x64\\Automation\\automation\\Include\\SubInspector\\Inspector\\SubInspector.dll\0",
		"Kainote_x64\\Automation\\automation\\Include\\unicode.lua\0",
		"Kainote_x64\\Automation\\automation\\Include\\utils.lua\0",
		"Kainote_x64\\Automation\\automation\\Include\\Yutils.lua\0",
		"Kainote_x64\\Automation\\automation\\packages\\karahelper.dll\0",
		//"Kainote_x64\\Automation\\automation\\tests\\DepUnit\\modules\\l0\0",
		"Kainote_x64\\Automation\\autosave\\txt.txt\0",
		//"Kainote_x64\\Automation\\config\0",
		//"Kainote_x64\\Automation\\feedDump\0",
		"Kainote_x64\\Automation\\log\\txt.txt\0",
		//"Kainote_x64\\Automation\\temp\0",
		//"Kainote_x64\\Automation\\tests\0",
		"Kainote_x64\\Dictionary\\en_US.aff\0",
		"Kainote_x64\\Dictionary\\en_US.dic\0",
		"Kainote_x64\\Dictionary\\pl.aff\0",
		"Kainote_x64\\Dictionary\\pl.dic\0",
		"Kainote_x64\\D3DX9_43.dll\0",
		"Kainote_x64\\ffms2.dll\0",
		"Kainote_x64\\KaiNote.exe\0",
		"Kainote_x64\\KaiNote.pdb\0",
		"Kainote_x64\\KaiNote_AVX.exe\0",
		"Kainote_x64\\KaiNote_AVX.pdb\0",
		"Kainote_x64\\LICENSE.txt\0",
		"Kainote_x64\\Locale\\en.mo\0",
		"Kainote_x64\\msvcp120.dll\0",
		"Kainote_x64\\msvcr120.dll\0",
		"Kainote_x64\\Themes\\MyDeepDark.txt\0",
		"Kainote_x64\\VSFilter_kainote.dll\0"
	};
	
	zipFile zf = zipOpen(path, APPEND_STATUS_CREATE);
	if (zf == NULL)
		return 1;
	char * pch = strrchr(path, '\\');
	char zipdir[2000];
	size_t newstart = pch - path + 1;
	strncpy(zipdir, path, newstart);
	//strcpy(&zipdir[newstart], "\\");
	bool _return = true;
	char buffer[CHUNK];

	for (int i = 0; i < filenamessize; i++){
		char * fn = filenames[i];
		char * pch1 = strchr(fn, '\\');
		size_t newnewstart = pch1 - fn + 1;
		strcpy(&zipdir[newstart], &fn[newnewstart]);
		
		size_t size = 0;
		FILE *file = fopen(zipdir, "rb");
		if (!file){
			zip_fileinfo zfi = { 0 };
			if (S_OK == zipOpenNewFileInZip(zf, fn, &zfi, NULL, 0, NULL, 0, NULL, Z_DEFLATED, Z_DEFAULT_COMPRESSION)){
				if (zipCloseFileInZip(zf))
					_return = false;

				continue;
			}
			_return = false;
			continue;
		}
		if (file)
		{
			zip_fileinfo zfi = { 0 };
			if (S_OK == zipOpenNewFileInZip(zf, fn, &zfi, NULL, 0, NULL, 0, NULL, Z_DEFLATED, Z_DEFAULT_COMPRESSION))
			{
				while (size = fread(buffer, 1, CHUNK, file))
				{
					
					if (zipWriteInFileInZip(zf, buffer, size))
						_return = false;

				}
				if (zipCloseFileInZip(zf))
					_return = false;

				continue;
			}
			_return = false;
		}
		_return = false;
	}
	if (zipClose(zf, NULL))
		return 3;

	if (!_return)
		return 4;
	
	//char dbx[] = "C:\\Users\\Bakura\\Dropbox\\Kainote x64.zip\0";
	char * dbx = argv[2];
	//if (convert(, &dbx)){
		//if (!check_exist_file(dbx)){ std::cerr << "Dropbox path don't exist " << path; delete[] dbx; return 1; }
	std::ifstream src(path, std::ios::binary);
	std::ofstream dst(dbx, std::ios::binary);
	dst << src.rdbuf();
	
	/*}
	else{
		std::cerr << "Cannot convert Dropbox path " << dbxpath; return 1;
	}*/
	std::cout << "End packing";
	return 0;
}

