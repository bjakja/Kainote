//  Copyright (c) 2016, Marcin Drob

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

#ifndef DATA_H_INCLUDED
#define DATA_H_INCLUDED

#pragma once

#include "Config.h"
#include "Styles.h"
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
	friend class SubsGrid;
private:
	std::vector<File*> undo;
	int iter;
	File *subs;
public:
	SubsFile();
	~SubsFile();
	void SaveUndo();
	void Redo();
	void Undo();
	void DummyUndo();
	void DummyUndo(int newIter);
	void EndLoad();
	Dialogue *CopyDial(int i, bool push=true, bool keepstate=false);
	Styles *CopyStyle(int i, bool push=true);
	SInfo *CopySinfo(int i, bool push=true);
	void GetURStatus(bool *_undo, bool *_redo);
	bool IsNotSaved();
	int maxx();
	int Iter();
	void RemoveFirst(int num);
	File *GetSubs();
	
	bool edited;
};







#endif // DATA_H_INCLUDED
