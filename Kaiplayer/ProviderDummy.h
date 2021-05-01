//  Copyright (c) 2021, Marcin Drob

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

#pragma once

#include "Provider.h"


class ProviderDummy : public Provider 
{
public:
	virtual ~ProviderDummy();
	ProviderDummy(const wxString& filename, RendererVideo* renderer, wxWindow* progressSinkWindow, bool* success);
	void GetFrameBuffer(byte** buffer) override;
	void GetFrame(int frame, byte* buff) override;
	void GetBuffer(void* buf, int64_t start, int64_t count, double vol = 1.0) override;
	void GetChapters(std::vector<chapter>* _chapters) override;
	void DeleteOldAudioCache() override;
	void SetColorSpace(const wxString& matrix) override;
	bool HasVideo();
private:
	static unsigned int __stdcall DummyProc(void* cls);
	void Processing();
	void GenerateTimecodes();
	void GenerateFrame();
	bool ParseDummyData(const wxString& data);
	byte *m_FrameBuffer = NULL;
	wxColour m_frameColor;
	bool m_pattern = false;
};