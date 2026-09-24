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

#pragma once

// Every video view shares one Direct3D 9Ex device and every audio view
// another, each view drawing into its own swap chain: making a device costs
// about 100 ms, a swap chain about 1 ms. Views of one kind draw one at a time.
enum class SharedDeviceKind { Video, Audio };

#ifdef _WIN32
#include <d3d9.h>

namespace SharedD3D9Device
{
	// the device for kind, AddRef'd, and its generation; nullptr without 9Ex
	IDirect3DDevice9 *Acquire(SharedDeviceKind kind, unsigned *generation);
	// a view found it removed: the next Acquire makes a new one
	void Removed(SharedDeviceKind kind, unsigned generation);
	// changes when the device is replaced, so the other views make theirs again
	unsigned Generation(SharedDeviceKind kind);
}

// Creates a Direct3D 9Ex device where Windows has it, else a Direct3D 9 one.
// 9Ex devices are not lost when another program takes the screen and queue
// only one frame ahead. object is released and replaced.
bool CreateD3D9Device(HWND window, D3DPRESENT_PARAMETERS *params, DWORD flags,
	IDirect3D9 **object, IDirect3DDevice9 **device);

// the device is gone for good and has to be made again
inline bool IsD3D9DeviceRemoved(HRESULT hr)
{
	return hr == D3DERR_DEVICEREMOVED || hr == D3DERR_DEVICEHUNG;
}
#else
#include "d3d9.h"

namespace SharedD3D9Device
{
	inline IDirect3DDevice9 *Acquire(SharedDeviceKind, unsigned *generation) { *generation = 0; return nullptr; }
	inline void Removed(SharedDeviceKind, unsigned) {}
	inline unsigned Generation(SharedDeviceKind) { return 0; }
}

inline bool CreateD3D9Device(HWND window, D3DPRESENT_PARAMETERS *params, DWORD flags,
	IDirect3D9 **object, IDirect3DDevice9 **device)
{
	if (!*object)
		*object = Direct3DCreate9(D3D_SDK_VERSION);
	return SUCCEEDED((*object)->CreateDevice(D3DADAPTER_DEFAULT, D3DDEVTYPE_HAL, window, flags, params, device));
}

inline bool IsD3D9DeviceRemoved(HRESULT)
{
	return false;
}
#endif
