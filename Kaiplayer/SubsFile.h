#ifndef DATA_H_INCLUDED
#define DATA_H_INCLUDED

#pragma once

#include "config.h"
#include "styles.h"
#include "SubsDialogue.h"
#include <vector>

class File
{
public:
	std::vector<Dialogue*> dials;
	std::vector<Styles*> styles;
	std::vector<SInfo*> sinfo;
	std::vector<Dialogue*> ddials;
	std::vector<Styles*> dstyles;
	std::vector<SInfo*> dsinfo;
	File();
	~File();
	void Clear();
	File *Copy();
};

class SubsFile
{
private:
	std::vector<File*> undo;
	int iter;

public:
	SubsFile();
	~SubsFile();
	void SaveUndo();
	void Redo();
	void Undo();
	void EndLoad();
	Dialogue *CopyDial(int i, bool push=true, bool keepstate=false);
	Styles *CopyStyle(int i, bool push=true);
	SInfo *CopySinfo(int i, bool push=true);
	bool IsNotSaved();
	int maxx();
	int Iter();
	void RemoveFirst(int num);
	File *subs;
	bool edited;
};







#endif // DATA_H_INCLUDED
