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

#include "GDIPlusInitializer.h"

bool GDIPlusInitializer::Check()
{
	if (m_loaded == -1){
		LoadGDIPlus();
	}
	return m_loaded == 1;
}

void GDIPlusInitializer::LoadGDIPlus()
{
	GdiplusStartupInput input;
	GdiplusStartupOutput output;
	if (GdiplusStartup(&m_gditoken, &input, &output) == Gdiplus::Ok)
	{
		//m_fontCollection = new PrivateFontCollection();
		//wxLogTrace("gdiplus", "successfully initialized GDI+");
		m_loaded = 1;
	}
	else
	{
		//wxLogTrace("gdiplus", "failed to initialize GDI+, missing gdiplus.dll?");
		m_loaded = 0;
	}
}
void GDIPlusInitializer::UnloadGDIPlus()
{
	if (m_drawTextStringFormat)
		delete m_drawTextStringFormat;
	
	for (auto it = fontsData.begin(); it != fontsData.end(); it++)
		delete it->second;

	if (m_gditoken)
	{
		GdiplusShutdown(m_gditoken);
		m_gditoken = 0;
	}
	m_loaded = -1;
}


// Get the string format used for the text drawing and measuring functions:
// notice that it must be the same one for all of them, otherwise the drawn
// text might be of different size than what measuring it returned.
StringFormat* GDIPlusInitializer::GetDrawTextStringFormat()
{
	if (!m_drawTextStringFormat)
	{
		m_drawTextStringFormat = new StringFormat(StringFormat::GenericTypographic());

		// This doesn't make any difference for DrawText() actually but we want
		// this behaviour when measuring text.
		m_drawTextStringFormat->SetFormatFlags
			(
			m_drawTextStringFormat->GetFormatFlags()
			| StringFormatFlagsMeasureTrailingSpaces
			);
	}

	return m_drawTextStringFormat;
}


GDIPlusInitializer Initializer;