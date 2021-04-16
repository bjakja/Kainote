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

#include "KaiDialog.h"


class AutoSavesRemoving : public KaiDialog 
{
public:
	AutoSavesRemoving(wxWindow *parent);
	~AutoSavesRemoving() {};


private:

	void ClearSelected(int id);
	void ClearAll(int id);
	void ClearByDate(int id);

	enum {
		ID_REMOVE_SELECTED_AUTO_SAVES = 12341,
		ID_REMOVE_SELECTED_INDICES,
		ID_REMOVE_SELECTED_AUDIO_CACHES,
		ID_REMOVE_ALL_AUTO_SAVES,
		ID_REMOVE_ALL_INDICES,
		ID_REMOVE_ALL_AUDIO_CACHES,
		ID_REMOVE_AUTO_SAVES_BY_DATE,
		ID_REMOVE_INDICES_BY_DATE,
		ID_REMOVE_AUDIO_CACHE_BY_DATE,
	};
};

