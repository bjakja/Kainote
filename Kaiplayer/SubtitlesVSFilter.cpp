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

#include "SubtitlesProvider.h"
#ifdef subsProvider


#include "RendererVideo.h"
#include "OpennWrite.h"
#include "kainoteMain.h"
#include "DshowRenderer.h"


csri_rend *SubtitlesProvider::m_CsriRenderer = NULL;
ASS_Renderer *SubtitlesProvider::m_Libass = NULL;
ASS_Library *SubtitlesProvider::m_Library = NULL;


void SubtitlesProvider::DestroySubtitlesProvider()
{
	if (m_Libass)
		ass_renderer_done(m_Libass);
	if (m_Library)
		ass_library_done(m_Library);

	if (m_CsriRenderer)
		csri_close_renderer(m_CsriRenderer);
}


SubtitlesVSFilter::SubtitlesVSFilter()
{

}

SubtitlesVSFilter::~SubtitlesVSFilter()
{
	SAFE_DELETE(m_CsriFrame);
	SAFE_DELETE(m_CsriFormat);
	if (m_CsriInstance)
		csri_close(m_CsriInstance);
	

}
void SubtitlesVSFilter::Draw(unsigned char* buffer, int time)
{
	if (m_CsriInstance){
		//for swap -pitch and buffer set to last element - pitch
		m_CsriFrame->strides[0] = (m_IsSwapped) ?
			-(m_CsriFormat->width * m_BytesPerColor)
			: m_CsriFormat->width * m_BytesPerColor;
		m_CsriFrame->planes[0] = (m_IsSwapped) ?
			buffer + (m_CsriFormat->width * (m_CsriFormat->height - 1) * m_BytesPerColor)
			: buffer;
		csri_render(m_CsriInstance, m_CsriFrame, double(time / 1000.0));
	}
};

bool SubtitlesVSFilter::Open(TabPanel *tab, int flag, wxString *text)
{
	if (m_CsriInstance) csri_close(m_CsriInstance);
	m_CsriInstance = NULL;

	wxString *textsubs = text;
	bool fromFile = false;
	switch (flag){
	case OPEN_DUMMY:
		textsubs = tab->Grid->GetVisible();
		break;
	case OPEN_WHOLE_SUBTITLES:
		textsubs = tab->Grid->SaveText();
		fromFile = true;
		break;
	case CLOSE_SUBTITLES:
	case OPEN_HAS_OWN_TEXT:
		break;
	default:
		break;
	}

	RendererVideo* renderer = tab->Video->GetRenderer();
	if (!renderer){
		delete textsubs;
		return false;
	}

	if (!textsubs) {
		renderer->m_HasDummySubs = true;
		delete textsubs;
		return true;
	}

	if (renderer->m_HasVisualEdition && renderer->m_Visual->Visual == VECTORCLIP && renderer->m_Visual->dummytext){
		wxString toAppend = renderer->m_Visual->dummytext->Trim().AfterLast(L'\n') + L"\r\n";
		if (fromFile){
			OpenWrite ow(*textsubs, false);
			ow.PartFileWrite(toAppend);
			ow.CloseFile();
		}
		else{
			(*textsubs) << toAppend;
		}
	}

	renderer->m_HasDummySubs = !fromFile;

	wxScopedCharBuffer buffer = textsubs->mb_str(wxConvUTF8);
	int size = strlen(buffer);


	// Select renderer
	csri_rend *vobsub = GetVSFilter();
	if (!vobsub){ KaiLog(_("Nie mo¿na zinicjalizowaæ CSRI.")); delete textsubs; return false; }

	m_CsriInstance = (fromFile) ? csri_open_file(vobsub, buffer, NULL) : csri_open_mem(vobsub, buffer, size, NULL);
	if (!m_CsriInstance){ KaiLog(_("Nie mo¿na utworzyæ instancji CSRI.")); delete textsubs; return false; }


	if (!m_CsriFormat || csri_request_fmt(m_CsriInstance, m_CsriFormat)){
		KaiLog(_("CSRI nie obs³uguje tego formatu."));
		csri_close(m_CsriInstance);
		m_CsriInstance = NULL;
		delete textsubs; return false;
	}

	delete textsubs;
	return true;
};

bool SubtitlesVSFilter::OpenString(wxString *text)
{
	if (m_CsriInstance) csri_close(m_CsriInstance);
	m_CsriInstance = NULL;

	if (!m_HasParameters){
		delete text;
		return false;
	}

	wxScopedCharBuffer buffer = text->mb_str(wxConvUTF8);
	int size = strlen(buffer);


	// Select renderer
	csri_rend *vobsub = GetVSFilter();
	if (!vobsub){ 
		KaiLog(_("Nie mo¿na zinicjalizowaæ CSRI.")); 
		delete text; 
		return false; 
	}

	m_CsriInstance = csri_open_mem(vobsub, buffer, size, NULL);
	if (!m_CsriInstance){ 
		KaiLog(_("Nie mo¿na utworzyæ instancji CSRI.")); 
		delete text; 
		return false; 
	}

	if (!m_CsriFormat || csri_request_fmt(m_CsriInstance, m_CsriFormat)){
		KaiLog(_("CSRI nie obs³uguje tego formatu."));
		csri_close(m_CsriInstance);
		m_CsriInstance = NULL;
		delete text; 
		return false;
	}

	delete text;
	return true;
};

csri_rend *SubtitlesVSFilter::GetVSFilter()
{
	if (!m_CsriRenderer){
		m_CsriRenderer = csri_renderer_default();
		csri_info *info = csri_renderer_info(m_CsriRenderer);
		wxString name = GetString(VSFILTER_INSTANCE);
		if (!name.empty()){
			while (info->name != name){
				m_CsriRenderer = csri_renderer_next(m_CsriRenderer);
				if (!m_CsriRenderer)
					break;
				info = csri_renderer_info(m_CsriRenderer);
			}
			if (!m_CsriRenderer)
				m_CsriRenderer = csri_renderer_default();
		}
	}
	return m_CsriRenderer;
}

void SubtitlesVSFilter::GetProviders(wxArrayString *providerList)
{
	csri_rend *filter = csri_renderer_default();
	if (!filter)
		return;
	csri_info *info = csri_renderer_info(filter);
	providerList->Add(info->name);
	while (1){
		filter = csri_renderer_next(filter);
		if (!filter)
			break;
		info = csri_renderer_info(filter);
		providerList->Add(info->name);
	}
	csri_close_renderer(filter);
	//test if current renderer is removed
	//if yes than just set NULL to renderer
	//without destroying it it makes memory leaks
	csri_close_renderer(m_CsriRenderer);
	m_CsriRenderer = NULL;
}

void SubtitlesVSFilter::SetVideoParameters(const wxSize & size, unsigned char format, bool isSwapped)
{
	m_VideoSize = size;
	m_IsSwapped = isSwapped;
	m_Format = format;
	byte bytes = (m_Format == RGB32) ? 4 : (m_Format == YUY2) ? 2 : 1;
	m_BytesPerColor = bytes;
	m_HasParameters = true;

	if (!m_CsriFrame) {
		m_CsriFrame = new csri_frame;
		//we only uses first planes and strides rest can be reset just once
		for (int i = 1; i < 4; i++) {
			m_CsriFrame->planes[i] = NULL;
			m_CsriFrame->strides[i] = NULL;
		}
	}
	if (!m_CsriFormat) { m_CsriFormat = new csri_fmt; }
		
	m_CsriFrame->pixfmt = (m_Format == NV12) ? CSRI_F_YV12A : (m_Format == YV12) ? CSRI_F_YV12 :
		(m_Format == YUY2) ? CSRI_F_YUY2 : CSRI_F_BGR_;

	m_CsriFormat->width = size.GetWidth();
	m_CsriFormat->height = size.GetHeight();
	m_CsriFormat->pixfmt = m_CsriFrame->pixfmt;
}

#endif
