//  Copyright (c) 2026, Marcin Drob

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

#include "D3D9Device.h"

#ifdef _WIN32

bool CreateD3D9Device(HWND window, D3DPRESENT_PARAMETERS *params, DWORD flags,
	IDirect3D9 **object, IDirect3DDevice9 **device)
{
	if (*object) {
		(*object)->Release();
		*object = nullptr;
	}
	const DWORD vertexProcessing[] = { D3DCREATE_HARDWARE_VERTEXPROCESSING, D3DCREATE_SOFTWARE_VERTEXPROCESSING };

	IDirect3D9Ex *objectEx = nullptr;
	if (SUCCEEDED(Direct3DCreate9Ex(D3D_SDK_VERSION, &objectEx))) {
		for (DWORD processing : vertexProcessing) {
			IDirect3DDevice9Ex *deviceEx = nullptr;
			if (SUCCEEDED(objectEx->CreateDeviceEx(D3DADAPTER_DEFAULT, D3DDEVTYPE_HAL, window,
				flags | processing, params, nullptr, &deviceEx))) {
				deviceEx->SetMaximumFrameLatency(1);
				*object = objectEx;
				*device = deviceEx;
				return true;
			}
		}
		objectEx->Release();
	}

	*object = Direct3DCreate9(D3D_SDK_VERSION);
	if (!*object)
		return false;
	for (DWORD processing : vertexProcessing) {
		if (SUCCEEDED((*object)->CreateDevice(D3DADAPTER_DEFAULT, D3DDEVTYPE_HAL, window,
			flags | processing, params, device)))
			return true;
	}
	return false;
}

#endif
