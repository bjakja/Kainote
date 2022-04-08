// Packaged with Avisynth v1.0 beta.
// http://www.math.berkeley.edu/~benrg/avisynth.html

//	VirtualDub - Video processing and capture application
//	Copyright (C) 1998-2000 Avery Lee
//
//	This program is free software; you can redistribute it and/or modify
//	it under the terms of the GNU General Public License as published by
//	the Free Software Foundation; either version 2 of the License, or
//	(at your option) any later version.
//
//	This program is distributed in the hope that it will be useful,
//	but WITHOUT ANY WARRANTY; without even the implied warranty of
//	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//	GNU General Public License for more details.
//
//	You should have received a copy of the GNU General Public License
//	along with this program; if not, write to the Free Software
//	Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.

#ifndef f_FILE64_H
#define f_FILE64_H

#include <avs/win.h>
#include <mmsystem.h>


class File64 {
public:
	HANDLE hFile, hFileUnbuffered;
	__int64 i64FilePosition;

	File64();
	File64(HANDLE _hFile, HANDLE _hFileUnbuffered);
	long _readFile(void *data, long len);
	void _readFile2(void *data, long len);
	bool _readChunkHeader(FOURCC& pfcc, DWORD& pdwLen);
	void _seekFile(__int64 i64NewPos);
	bool _seekFile2(__int64 i64NewPos);
	void _skipFile(__int64 bytes);
	bool _skipFile2(__int64 bytes);
	long _readFileUnbuffered(void *data, long len);
	void _seekFileUnbuffered(__int64 i64NewPos);
	__int64 _posFile();
	__int64 _sizeFile();
};

#endif
