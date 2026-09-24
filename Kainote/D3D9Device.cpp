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
#include <atomic>
#include <mutex>

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

namespace
{
	struct SharedSlot
	{
		std::mutex mutex;
		IDirect3D9Ex *object = nullptr;
		IDirect3DDevice9Ex *device = nullptr;
		std::atomic<unsigned> generation{ 1 };
	};
	SharedSlot gs_Slots[2];
}

IDirect3DDevice9 *SharedD3D9Device::Acquire(SharedDeviceKind kind, unsigned *generation)
{
	SharedSlot &slot = gs_Slots[(int)kind];
	std::lock_guard<std::mutex> lock(slot.mutex);
	if (!slot.device) {
		// the implicit swap chain is never shown, as each view presents its own;
		// the desktop window outlives every thread that may make the device
		HWND window = GetDesktopWindow();
		if (FAILED(Direct3DCreate9Ex(D3D_SDK_VERSION, &slot.object)))
			return nullptr;
		D3DPRESENT_PARAMETERS params = {};
		params.Windowed = TRUE;
		params.hDeviceWindow = window;
		params.BackBufferWidth = params.BackBufferHeight = 1;
		params.BackBufferCount = 1;
		params.SwapEffect = D3DSWAPEFFECT_COPY;
		params.BackBufferFormat = D3DFMT_X8R8G8B8;
		params.PresentationInterval = D3DPRESENT_INTERVAL_ONE;
		const DWORD vertexProcessing[] = { D3DCREATE_HARDWARE_VERTEXPROCESSING, D3DCREATE_SOFTWARE_VERTEXPROCESSING };
		for (DWORD processing : vertexProcessing) {
			if (SUCCEEDED(slot.object->CreateDeviceEx(D3DADAPTER_DEFAULT, D3DDEVTYPE_HAL, window,
				processing | D3DCREATE_MULTITHREADED | D3DCREATE_FPU_PRESERVE, &params, nullptr, &slot.device)))
				break;
		}
		if (!slot.device) {
			slot.object->Release();
			slot.object = nullptr;
			return nullptr;
		}
		slot.device->SetMaximumFrameLatency(1);
	}
	slot.device->AddRef();
	*generation = slot.generation;
	return slot.device;
}

void SharedD3D9Device::Removed(SharedDeviceKind kind, unsigned generation)
{
	SharedSlot &slot = gs_Slots[(int)kind];
	std::lock_guard<std::mutex> lock(slot.mutex);
	if (generation != slot.generation || !slot.device)
		return;
	slot.device->Release();
	slot.device = nullptr;
	slot.object->Release();
	slot.object = nullptr;
	++slot.generation;
}

unsigned SharedD3D9Device::Generation(SharedDeviceKind kind)
{
	return gs_Slots[(int)kind].generation;
}

#endif
