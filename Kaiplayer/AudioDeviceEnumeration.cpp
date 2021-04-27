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

#include "AudioDeviceEnumeration.h"
#include "utils.h"
#include <Functiondiscoverykeys_devpkey.h>

const CLSID CLSID_MMDeviceEnumerator = __uuidof(MMDeviceEnumerator);
const IID IID_IMMDeviceEnumerator = __uuidof(IMMDeviceEnumerator);


bool EnumerateAudioDevices(wxArrayString* devices)
{
	HRESULT hr = S_OK;
	IMMDeviceEnumerator* pEnumerator = NULL;
	IMMDeviceCollection* pCollection = NULL;
	IMMDevice* pEndpoint = NULL;
	IPropertyStore* pProps = NULL;
	//LPWSTR pwszID = NULL;

	hr = CoCreateInstance(CLSID_MMDeviceEnumerator, NULL,
		CLSCTX_ALL, IID_IMMDeviceEnumerator, (void**)&pEnumerator);
	if (FAILED(hr)) { goto Exit; };

	hr = pEnumerator->EnumAudioEndpoints(eRender, DEVICE_STATE_ACTIVE, &pCollection);
	if (FAILED(hr)) { goto Exit; };

	UINT  count;
	hr = pCollection->GetCount(&count);
	if (FAILED(hr)) { goto Exit; };

	if (count == 0)
	{
		goto Exit;
	}

	// Each loop prints the name of an endpoint device.
	for (ULONG i = 0; i < count; i++)
	{
		// Get pointer to endpoint number i.
		hr = pCollection->Item(i, &pEndpoint);
		if (FAILED(hr)) { goto Exit; };

		// Get the endpoint ID string.
		//hr = pEndpoint->GetId(&pwszID);
		//if (FAILED(hr)) { goto Exit; };

		hr = pEndpoint->OpenPropertyStore(STGM_READ, &pProps);
		if (FAILED(hr)) { goto Exit; };

		PROPVARIANT varName;
		// Initialize container for property value.
		PropVariantInit(&varName);

		// Get the endpoint's friendly-name property.
		hr = pProps->GetValue(PKEY_Device_FriendlyName, &varName);
		if (FAILED(hr)) { goto Exit; };

		devices->Add(varName.pwszVal);

		//CoTaskMemFree(pwszID);
		//pwszID = NULL;
		PropVariantClear(&varName);
		SAFE_RELEASE(pProps);
		SAFE_RELEASE(pEndpoint);
	}
	SAFE_RELEASE(pEnumerator);
	SAFE_RELEASE(pCollection);
	return true;

Exit:
	//CoTaskMemFree(pwszID);
	SAFE_RELEASE(pEnumerator);
	SAFE_RELEASE(pCollection);
	SAFE_RELEASE(pEndpoint);
	SAFE_RELEASE(pProps);
	return false;
}

bool GetGuid(const wxString& name, REFIID iid, DWORD dwClsCtx, void** ppInterface)
{
	HRESULT hr = S_OK;
	IMMDeviceEnumerator* pEnumerator = NULL;
	IMMDeviceCollection* pCollection = NULL;
	IMMDevice* pEndpoint = NULL;
	IPropertyStore* pProps = NULL;
	//LPWSTR pwszID = NULL;

	hr = CoCreateInstance(CLSID_MMDeviceEnumerator, NULL,
		CLSCTX_ALL, IID_IMMDeviceEnumerator, (void**)&pEnumerator);
	if (FAILED(hr)) { goto Exit; };

	hr = pEnumerator->EnumAudioEndpoints(eRender, DEVICE_STATE_ACTIVE, &pCollection);
	if (FAILED(hr)) { goto Exit; };

	UINT  count;
	hr = pCollection->GetCount(&count);
	if (FAILED(hr)) { goto Exit; };

	if (count == 0)
	{
		goto Exit;
	}

	// Each loop prints the name of an endpoint device.
	for (ULONG i = 0; i < count; i++)
	{
		// Get pointer to endpoint number i.
		hr = pCollection->Item(i, &pEndpoint);
		if (FAILED(hr)) { goto Exit; };

		// Get the endpoint ID string.
		//hr = pEndpoint->GetId(&pwszID);
		//if (FAILED(hr)) { goto Exit; };

		hr = pEndpoint->OpenPropertyStore(STGM_READ, &pProps);
		if (FAILED(hr)) { goto Exit; };

		PROPVARIANT varName;
		// Initialize container for property value.
		PropVariantInit(&varName);

		// Get the endpoint's friendly-name property.
		hr = pProps->GetValue(PKEY_Device_FriendlyName, &varName);
		if (FAILED(hr)) { goto Exit; };

		if (wxString(varName.pwszVal) == name) {
			PropVariantClear(&varName);
			hr = pEndpoint->Activate(iid, dwClsCtx, NULL, ppInterface);
			if (FAILED(hr)) { goto Exit; };
			SAFE_RELEASE(pProps);
			SAFE_RELEASE(pEndpoint);
			SAFE_RELEASE(pEnumerator);
			SAFE_RELEASE(pCollection);
			return true;
		}

		//CoTaskMemFree(pwszID);
		//pwszID = NULL;
		PropVariantClear(&varName);
		SAFE_RELEASE(pProps);
		SAFE_RELEASE(pEndpoint);
	}
	SAFE_RELEASE(pEnumerator);
	SAFE_RELEASE(pCollection);
	return false;

Exit:
	//CoTaskMemFree(pwszID);
	SAFE_RELEASE(pEnumerator);
	SAFE_RELEASE(pCollection);
	SAFE_RELEASE(pEndpoint);
	SAFE_RELEASE(pProps);
	return false;
}
