// Avisynth v2.5.
// http://www.avisynth.org

// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation; either version 2 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA, or visit
// http://www.gnu.org/copyleft/gpl.html .
//
// Linking Avisynth statically or dynamically with other modules is making a
// combined work based on Avisynth.  Thus, the terms and conditions of the GNU
// General Public License cover the whole combination.
//
// As a special exception, the copyright holders of Avisynth give you
// permission to link Avisynth with independent modules that communicate with
// Avisynth solely through the interfaces defined in avisynth.h, regardless of the license
// terms of these independent modules, and to copy and distribute the
// resulting combined work under terms of your choice, provided that
// every copy of the combined work is accompanied by a complete copy of
// the source code of Avisynth (the version of Avisynth used to produce the
// combined work), being distributed under the terms of the GNU General
// Public License plus this exception.  An independent module is a module
// which is not derived from or based on Avisynth, such as 3rd-party filters,
// import and export plugins, or graphical user interfaces.

#include <avisynth.h>
#include <avs/win.h>
#include <avs/minmax.h>

/********************************************************************
* VFAPI plugin support
********************************************************************/

#define VF_STREAM_VIDEO   0x00000001
#define VF_STREAM_AUDIO   0x00000002
#define VF_OK       0x00000000
#define VF_ERROR      0x80004005

struct VF_PluginInfo {
  DWORD dwSize;
  DWORD dwAPIVersion;
  DWORD dwVersion;
  DWORD dwSupportStreamType;
  char  cPluginInfo[256];
  char  cFileType[256];
};

typedef DWORD VF_FileHandle;

struct VF_FileInfo {
  DWORD dwSize;
  DWORD dwHasStreams;
};

struct VF_StreamInfo_Video {
  DWORD dwSize;
  DWORD dwLengthL;
  DWORD dwLengthH;
  DWORD dwRate;
  DWORD dwScale;
  DWORD dwWidth;
  DWORD dwHeight;
  DWORD dwBitCount;
};

struct VF_StreamInfo_Audio {
  DWORD dwSize;
  DWORD dwLengthL;
  DWORD dwLengthH;
  DWORD dwRate;
  DWORD dwScale;
  DWORD dwChannels;
  DWORD dwBitsPerSample;
  DWORD dwBlockAlign;
};

struct VF_ReadData_Video {
  DWORD dwSize;
  DWORD dwFrameNumberL;
  DWORD dwFrameNumberH;
  void  *lpData;
  long  lPitch;
};

struct VF_ReadData_Audio {
  DWORD dwSize;
  DWORD dwSamplePosL;
  DWORD dwSamplePosH;
  DWORD dwSampleCount;
  DWORD dwReadedSampleCount;
  DWORD dwBufSize;
  void  *lpBuf;
};

struct VF_PluginFunc {
  DWORD dwSize;
  HRESULT (_stdcall *OpenFile)( const char *lpFileName, VF_FileHandle* lpFileHandle );
  HRESULT (_stdcall *CloseFile)( VF_FileHandle hFileHandle );
  HRESULT (_stdcall *GetFileInfo)( VF_FileHandle hFileHandle, VF_FileInfo* lpFileInfo );
  HRESULT (_stdcall *GetStreamInfo)( VF_FileHandle hFileHandle,DWORD dwStream,void *lpStreamInfo );
  HRESULT (_stdcall *ReadData)( VF_FileHandle hFileHandle,DWORD dwStream,void *lpData );
};

struct VFAPI_PluginRefs
{
  HMODULE Library;
  VF_PluginFunc PluginFunc;
};

typedef HRESULT (__stdcall *VF_GetPluginInfo)(VF_PluginInfo* lpPluginInfo);
typedef HRESULT (__stdcall *VF_GetPluginFunc)(VF_PluginFunc* lpPluginFunc);

void CheckHresult(IScriptEnvironment* env, HRESULT hr) {
  if (FAILED(hr)) {
    env->ThrowError("VFAPI plugin returned an error (0x%X)", hr);
  }
}

void DeleteVFPluginFunc(void* vfpf, IScriptEnvironment*) {

  VFAPI_PluginRefs *ref = (VFAPI_PluginRefs*)vfpf;
  FreeLibrary(ref->Library);
  delete ref;
}

class VFAPIPluginProxy : public IClip {
  VideoInfo vi;
  const VF_PluginFunc* const plugin_func;
  VF_FileHandle h;
public:
  VFAPIPluginProxy(const char* filename, const VF_PluginFunc* _plugin_func, IScriptEnvironment* env)
    : plugin_func(_plugin_func)
  {
    CheckHresult(env, plugin_func->OpenFile(filename, &h));
    VF_FileInfo file_info = { sizeof(VF_FileInfo) };
    CheckHresult(env, plugin_func->GetFileInfo(h, &file_info));

    memset(&vi, 0, sizeof(VideoInfo));
    if (file_info.dwHasStreams & VF_STREAM_VIDEO) {
      VF_StreamInfo_Video stream_info = { sizeof(VF_StreamInfo_Video) };
      CheckHresult(env, plugin_func->GetStreamInfo(h, VF_STREAM_VIDEO, &stream_info));
      if (stream_info.dwBitCount == 24) {
        vi.pixel_type = VideoInfo::CS_BGR24;
      } else if (stream_info.dwBitCount == 32) {
        vi.pixel_type = VideoInfo::CS_BGR32;
      } else {
        env->ThrowError("VFAPIPluginProxy: plugin returned invalid bit depth (%d)", stream_info.dwBitCount);
      }
      vi.width = stream_info.dwWidth;
      vi.height = stream_info.dwHeight;
      vi.num_frames = stream_info.dwLengthL;
      vi.SetFPS(stream_info.dwRate, stream_info.dwScale);
    }
    if (file_info.dwHasStreams & VF_STREAM_AUDIO) {
      VF_StreamInfo_Audio stream_info = { sizeof(VF_StreamInfo_Audio) };
      CheckHresult(env, plugin_func->GetStreamInfo(h, VF_STREAM_AUDIO, &stream_info));
      vi.audio_samples_per_second = stream_info.dwRate / stream_info.dwScale;
      vi.num_audio_samples = stream_info.dwLengthL;
      vi.nchannels = stream_info.dwChannels;

      if (stream_info.dwBitsPerSample == 8)
        vi.sample_type = SAMPLE_INT8;
      else if (stream_info.dwBitsPerSample == 16)
        vi.sample_type = SAMPLE_INT16;
      else if (stream_info.dwBitsPerSample == 24)
        vi.sample_type = SAMPLE_INT24;
      else if (stream_info.dwBitsPerSample == 32)
        vi.sample_type = SAMPLE_INT32;
      else
        env->ThrowError("VFAPIPluginProxy: plugin returned invalid audio sample depth (%d)", stream_info.dwBitsPerSample);
    }

    if (!vi.HasVideo() && !vi.HasAudio())
      env->ThrowError("VFAPIPluginProxy: no video or audio stream");
  }

  const VideoInfo& __stdcall GetVideoInfo() { return vi; }

  PVideoFrame __stdcall GetFrame(int n, IScriptEnvironment* env) {
    n = max(min(n, vi.num_frames-1), 0);
    PVideoFrame result = env->NewVideoFrame(vi);
    VF_ReadData_Video vfrdv = { sizeof(VF_ReadData_Video), n, 0, result->GetWritePtr(), result->GetPitch() };
    CheckHresult(env, plugin_func->ReadData(h, VF_STREAM_VIDEO, &vfrdv));
    return result;
  }

  void __stdcall GetAudio(void* buf, __int64 start, __int64 count, IScriptEnvironment* env) {
    if (start < 0) {
      int bytes = (int)vi.BytesFromAudioSamples(-start);
      memset(buf, 0, bytes);
      buf = (char*)buf + bytes;
      count += start;
      start = 0;
    }
    VF_ReadData_Audio vfrda = { sizeof(VF_ReadData_Audio), (int)start, 0, (int)count, 0, (int)vi.BytesFromAudioSamples(count), buf };
    CheckHresult(env, plugin_func->ReadData(h, VF_STREAM_AUDIO, &vfrda));
    if (int(vfrda.dwReadedSampleCount) < count) {
      memset((char*)buf + vi.BytesFromAudioSamples(vfrda.dwReadedSampleCount),
        0, (size_t)vi.BytesFromAudioSamples(count - vfrda.dwReadedSampleCount));
    }
  }

  bool __stdcall GetParity(int n) { return false; }
  int __stdcall SetCacheHints(int cachehints,int frame_range) { return 0; };

  static AVSValue __cdecl Create(AVSValue args, void* user_data, IScriptEnvironment* env) {
    args = args[0];
    PClip result = new VFAPIPluginProxy(args[0].AsString(), (VF_PluginFunc*)user_data, env);
    for (int i=1; i<args.ArraySize(); ++i)
      result = new_Splice(result, new VFAPIPluginProxy(args[i].AsString(), (VF_PluginFunc*)user_data, env), false, env);
    return result;
  }
};


AVSValue LoadVFAPIPlugin(AVSValue args, void*, IScriptEnvironment* env) {
  const char* plugin_name = args[0].AsString();

  HMODULE hmodule = LoadLibrary(plugin_name);
  if (!hmodule)
    env->ThrowError("LoadVirtualdubPlugin: Error opening \"%s\", error=0x%x", plugin_name, GetLastError());

  VF_GetPluginInfo vfGetPluginInfo = (VF_GetPluginInfo)GetProcAddress(hmodule, "vfGetPluginInfo");
  VF_GetPluginFunc vfGetPluginFunc = (VF_GetPluginFunc)GetProcAddress(hmodule, "vfGetPluginFunc");
  if (!vfGetPluginInfo || !vfGetPluginFunc)
  {
    FreeLibrary(hmodule);
    env->ThrowError("LoadPlugin: \"%s\" is not a VFAPI plugin", plugin_name);
  }

  VF_PluginInfo plugin_info = { sizeof(VF_PluginInfo) };
  CheckHresult(env, vfGetPluginInfo(&plugin_info));

  VFAPI_PluginRefs *refs = new VFAPI_PluginRefs();
  refs->Library = hmodule;
  VF_PluginFunc* plugin_func = &(refs->PluginFunc);
  env->AtExit(DeleteVFPluginFunc, plugin_func);

  plugin_func->dwSize = sizeof(VF_PluginFunc);
  CheckHresult(env, vfGetPluginFunc(plugin_func));

  env->AddFunction(args[1].AsString(), "s+", VFAPIPluginProxy::Create, plugin_func);

  return plugin_info.cPluginInfo ? AVSValue(plugin_info.cPluginInfo) : AVSValue();
}

const AVS_Linkage * AVS_linkage = 0;
extern "C" __declspec(dllexport) const char* __stdcall AvisynthPluginInit3(IScriptEnvironment* env, const AVS_Linkage* const vectors) {
	AVS_linkage = vectors;

  // clip, base filename, start, end, image format/extension, info
  env->AddFunction("LoadVFAPIPlugin", "ss", LoadVFAPIPlugin, 0);

  return "`LoadVFAPIPlugin' Allows to load and use filters written for VFAPI.";
}
