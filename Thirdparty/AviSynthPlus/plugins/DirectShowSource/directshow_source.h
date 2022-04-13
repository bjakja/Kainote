// Avisynth v2.5.  Copyright 2002 Ben Rudiak-Gould et al.
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

#undef UNICODE
#include <avisynth.h>
#include <avs/win.h>

#include <streams.h>
#include <stdio.h>


/********************************************************************
# Build Hints for DirectShow SDK

# Hints for Avisynth+, Visual Studio 2019

- download and install 7.1 SDK
  Microsoft Windows SDK for Windows 7 and .NET Framework 4
    https://www.microsoft.com/en-us/download/details.aspx?id=8279

  In case of problems:
  https://social.msdn.microsoft.com/Forums/vstudio/en-US/1de7c9b4-1feb-4c98-b426-f7f02cbafa99/windows-sdk-71-on-windows-10
  Uninstall VC2010 redist, download offline ISO image installer, install

- [Building strmbase.lib]
  - Open solution in
      c:\Program Files\Microsoft SDKs\Windows\v7.1A\Samples\multimedia\directshow\baseclasses\
    (or replace v7.1A with appropriate v7 SDK folder)
    Project is of old format, will be converted.
  - Compile for targets Release_MBCS x86 and x64.
    Find compiled library strmbase.lib in
      c:\Program Files\Microsoft SDKs\Windows\v7.1A\Samples\multimedia\directshow\baseclasses\Release_MBCS\;
    and
      c:\Program Files\Microsoft SDKs\Windows\v7.1A\Samples\multimedia\directshow\baseclasses\x64\Release_MBCS\;
- [Building DirectShowSource.dll]
  - Open PluginDirectShowSource project in the AviSynth plus solution
  - Note: Usually Avisynth+ CMake is configuring the settings below properly.
  - Edit Project Properties|VC++ Directories|Include Paths
    Add to the beginning
    c:\Program Files\Microsoft SDKs\Windows\v7.1A\Samples\multimedia\directshow\baseclasses\;
    c:\Program Files\Microsoft SDKs\Windows\v7.1A\Include\; or put behind $(VC_IncludePath) if windows.h not found
  - Edit Project Properties|VC++ Directories|Library Directories
    For x86 target add
      c:\Program Files\Microsoft SDKs\Windows\v7.1A\Samples\multimedia\directshow\baseclasses\Release_MBCS\;
    For x64 target add
      c:\Program Files\Microsoft SDKs\Windows\v7.1A\Samples\multimedia\directshow\baseclasses\x64\Release_MBCS\;
    For XP target add (to find winmm.lib)
      c:\Program Files (x86)\Microsoft SDKs\Windows\v7.1A\Lib\; (for 32 bit build)
      c:\Program Files (x86)\Microsoft SDKs\Windows\v7.1A\Lib\x64\; (for 64 bit build)
  - Edit Project Properties|Linker|Input|Additional Dependencies
    Add strmbase.lib to the list
  - For XP compatibility
    - choose Platform Toolset v141_xp
    - Edit Project Properties|C/C++|Command Line
      Add
      /Zc:threadSafeInit-
  - Build

# Hints from 2.6.1alpha1:

# Patch ($Platform SDK)\Samples\Multimedia\DirectShow\BaseClasses\streams.h
# remove ATL reference

#define NO_SHLWAPI_STRFCNS
// #include <atlbase.h>    <<---- Line 179
#include <strsafe.h>

# Start a Visual Studio Command Prompt
Tools>Visual Studio 200x Command Prompt

# Add DirectX SDK to front of INCLUDE path
Set INCLUDE=C:\Program Files\Microsoft DirectX SDK (August 2009)\Include;%INCLUDE%

Set INCLUDE=C:\Program Files (x86)\Microsoft DirectX SDK (August 2009)\Include;%INCLUDE%

# Cd to the Platform SDK directory.
cd "C:\Program Files\Microsoft Platform SDK for Windows Server 2003 R2"

# Define SDK environment
SetEnv /XP32 /RETAIL

# Cd to BaseClasses directory
cd Samples\Multimedia\DirectShow\BaseClasses

# Make strmbase.lib (Probably won't have write access, so take a copy)
nmake

# Add DirectShow and DirectX to C++ command line INCLUDE path
/I "C:\Program Files\Microsoft Platform SDK for Windows Server 2003 R2\Samples\Multimedia\DirectShow\BaseClasses"
/I "C:\Program Files\Microsoft DirectX SDK (August 2009)\Include"

/I "C:\Program Files (x86)\Microsoft DirectX SDK (August 2009)\Include"

# Add DirectShow and DirectX to link command line LIB path
/LIBPATH:"C:\Program Files\Microsoft Platform SDK for Windows Server 2003 R2\Samples\Multimedia\DirectShow\BaseClasses\XP32_RETAIL"
/LIBPATH:"C:\Program Files\Microsoft DirectX SDK (August 2009)\Lib\x86"

/LIBPATH:"C:\Program Files (x86)\Microsoft DirectX SDK (August 2009)\Lib\x86"
********************************************************************/

#define SAFE_RELEASE(x) { if (x) x->Release(); x = NULL; }



#include <evcode.h>
#include <control.h>
#include <strmif.h>
#include <amvideo.h>
#include <dvdmedia.h>
#include <vfwmsgs.h>
#include <initguid.h>
#include <uuids.h>
#include <errors.h>


// Ahhgg we don't want all of vfw.h just for this
#define WAVE_FORMAT_IEEE_FLOAT 0x0003


class GetSample;


// Log utility class
class LOG {
  int count;

public:
  FILE* file;
  const int mask;

  LOG(const char* fn, int _mask, IScriptEnvironment* env);
  ~LOG();
  void AddRef() { count += 1; };
  void DelRef(const char* s);
};


class GetSampleEnumMediaTypes : public IEnumMediaTypes {
  long refcnt;
  GetSample* const parent;
  unsigned pos, count;

  LOG* log;

public:
  GetSampleEnumMediaTypes(GetSample* _parent, unsigned _count, unsigned _pos=0);
  ~GetSampleEnumMediaTypes();

// IUnknown::

  ULONG __stdcall AddRef() { InterlockedIncrement(&refcnt); return refcnt; }
  ULONG __stdcall Release() {
    if (!InterlockedDecrement(&refcnt)) {
      delete this;
      return 0;
    } else {
      return refcnt;
    }
  }
  HRESULT __stdcall QueryInterface(REFIID iid, void** ppv) {
    if      (iid == IID_IUnknown)        *ppv = static_cast<IUnknown*>(this);
    else if (iid == IID_IEnumMediaTypes) *ppv = static_cast<IEnumMediaTypes*>(this);
    else {
      *ppv = 0;
      return E_NOINTERFACE;
    }
    AddRef();
    return S_OK;
  }

// IEnumMediaTypes::

  HRESULT __stdcall Next(ULONG cMediaTypes, AM_MEDIA_TYPE** ppMediaTypes, ULONG* pcFetched);
  HRESULT __stdcall Skip(ULONG cMediaTypes);
  HRESULT __stdcall Reset();
  HRESULT __stdcall Clone(IEnumMediaTypes** ppEnum);
};


class GetSampleEnumPins : public IEnumPins {
  long refcnt;
  GetSample* const parent;
  int pos;

  LOG* log;

public:
  GetSampleEnumPins(GetSample* _parent, int _pos=0);
  ~GetSampleEnumPins();

// IUnknown::

  ULONG __stdcall AddRef() { InterlockedIncrement(&refcnt); return refcnt; }
  ULONG __stdcall Release() {
    if (!InterlockedDecrement(&refcnt)) {
      delete this;
      return 0;
    } else {
      return refcnt;
    }
  }
  HRESULT __stdcall QueryInterface(REFIID iid, void** ppv) {
    if      (iid == IID_IUnknown)  *ppv = static_cast<IUnknown*>(this);
    else if (iid == IID_IEnumPins) *ppv = static_cast<IEnumPins*>(this);
    else {
      *ppv = 0;
      return E_NOINTERFACE;
    }
    AddRef();
    return S_OK;
  }

// IEnumPins::

  HRESULT __stdcall Next(ULONG cPins, IPin** ppPins, ULONG* pcFetched);
  HRESULT __stdcall Skip(ULONG cPins) { return E_NOTIMPL; }
  HRESULT __stdcall Reset() { pos=0; return S_OK; }
  HRESULT __stdcall Clone(IEnumPins** ppEnum) { return E_NOTIMPL; }
};


class GetSample : public IBaseFilter, public IPin, public IMemInputPin {

  long refcnt;
  IMemAllocator* Allocator;  // refcounted
  IPin* source_pin;  // not refcounted
  IFilterGraph* filter_graph;  // not refcounted
  IReferenceClock* pclock;  // not refcounted
  FILTER_STATE state;
  bool end_of_stream, flushing, seeking;
  IUnknown *m_pPos;  // Pointer to the CPosPassThru object.
  VideoInfo vi;
  bool m_bInvertFrames; // Data in av_buffer is flipped vertically compared to the expected orientation of vi.pixel_type
  bool lockvi; // Format negotiation is allowed until DSS is fully created

  HANDLE evtDoneWithSample, evtNewSampleReady;

  const bool load_audio;
  const bool load_video;

  bool graphTimeout;

  const char * const streamName;

  AM_MEDIA_TYPE *am_media_type;

  unsigned media, no_my_media_types;
  AM_MEDIA_TYPE *my_media_types[18]; // 2.6

  PVideoFrame pvf;

public:
  enum {
    mediaNONE   = 0,
    mediaYUV9   = 1<<0, // not implemented
    mediaYV12   = 1<<1,
    mediaYUY2   = 1<<2,
    mediaARGB   = 1<<3,
    mediaRGB32  = 1<<4,
    mediaRGB24  = 1<<5,
    mediaAYUV   = 1<<6, // 2.6
    mediaY411   = 1<<7, // 2.6
    mediaY41P   = 1<<8, // 2.6
    mediaYV16   = 1<<9, // 2.6
    mediaYV24   = 1<<10,// 2.6
    mediaI420   = 1<<11,// 2.6
    mediaNV12   = 1<<12,// 2.6
    mediaRGB64  = 1<<13,
    mediaRGB48  = 1<<14,
    mediaRGB    = mediaARGB | mediaRGB32 | mediaRGB24 | mediaRGB64 | mediaRGB48,
    mediaYUV    = mediaYUV9 | mediaYV12 | mediaYUY2 | mediaAYUV | mediaY411 | mediaY41P,
    mediaYUVex  = mediaYUV  | mediaYV16 | mediaYV24 | mediaI420 | mediaNV12,
    mediaAUTO   = mediaRGB | mediaYUV,
    mediaFULL   = mediaRGB | mediaYUVex,
    mediaPAD    = 1<<31,
  };

  __int64 segment_start_time, segment_stop_time, sample_start_time, sample_end_time;

  int avg_time_per_frame;
  int time_of_last_frame;

  int av_sample_bytes;
  BYTE* av_buffer;         // Killed on StopGraph

  LOG* log;

  GetSample(bool _load_audio, bool _load_video, unsigned _media, LOG* _log);
  ~GetSample();

  // These are utility functions for use by DirectShowSource.  Note that
  // the other thread (the one used by the DirectShow filter graph side of
  // things) is always blocked when any of these functions is called, and
  // is always blocked when they return

  bool IsConnected() { return !!source_pin; }
  bool IsEndOfStream() { return end_of_stream; }
  const VideoInfo& GetVideoInfo() { lockvi = true; return vi; }
  PVideoFrame GetCurrentFrame(IScriptEnvironment* env, int n, bool _TrapTimeouts, DWORD &timeout);
  __int64 GetSampleStartTime() { return segment_start_time + sample_start_time; }
  __int64 GetSampleEndTime() { return segment_start_time + sample_end_time; }
  bool WaitForStart(DWORD &timeout);
  const AM_MEDIA_TYPE *GetMediaType(unsigned pos);

  // These all cause the other thread (the one used by the DirectShow filter
  // graph side of things) to do it's thing and cycle. Hopefully it is blocked
  // again as they exit, a timeouts from StartGraph violates this.

  HRESULT StartGraph(IGraphBuilder* gb);
  void StopGraph(IGraphBuilder* gb);
  void PauseGraph(IGraphBuilder* gb);
  HRESULT SeekTo(__int64 pos, IGraphBuilder* gb);
  bool NextSample(DWORD &timeout);

// IUnknown::

  ULONG __stdcall AddRef();
  ULONG __stdcall Release();
  HRESULT __stdcall QueryInterface(REFIID iid, void** ppv);

// IPersist::

  HRESULT __stdcall GetClassID(CLSID* pClassID);

// IMediaFilter::

  HRESULT __stdcall Stop();
  HRESULT __stdcall Pause();
  HRESULT __stdcall Run(REFERENCE_TIME tStart);
  HRESULT __stdcall GetState(DWORD dwMilliSecsTimeout, FILTER_STATE* State);
  HRESULT __stdcall SetSyncSource(IReferenceClock* pClock);
  HRESULT __stdcall GetSyncSource(IReferenceClock** ppClock);

// IBaseFilter::

  HRESULT __stdcall EnumPins(IEnumPins** ppEnum);
  HRESULT __stdcall FindPin(LPCWSTR Id, IPin** ppPin);
  HRESULT __stdcall QueryFilterInfo(FILTER_INFO* pInfo);
  HRESULT __stdcall JoinFilterGraph(IFilterGraph* pGraph, LPCWSTR pName);
  HRESULT __stdcall QueryVendorInfo(LPWSTR* pVendorInfo);

// IPin::

  HRESULT __stdcall Connect(IPin* pReceivePin, const AM_MEDIA_TYPE* pmt);
  HRESULT __stdcall ReceiveConnection(IPin* pConnector, const AM_MEDIA_TYPE* pmt);
  HRESULT __stdcall Disconnect();
  HRESULT __stdcall ConnectedTo(IPin** ppPin);
  HRESULT __stdcall ConnectionMediaType(AM_MEDIA_TYPE* pmt);
  HRESULT __stdcall QueryPinInfo(PIN_INFO* pInfo);
  HRESULT __stdcall QueryDirection(PIN_DIRECTION* pPinDir);
  HRESULT __stdcall QueryId(LPWSTR* Id);
  HRESULT __stdcall QueryAccept(const AM_MEDIA_TYPE* pmt);
  HRESULT __stdcall EnumMediaTypes(IEnumMediaTypes** ppEnum);
  HRESULT __stdcall QueryInternalConnections(IPin** apPin, ULONG* nPin);
  HRESULT __stdcall EndOfStream();
  HRESULT __stdcall BeginFlush();
  HRESULT __stdcall EndFlush();
  HRESULT __stdcall NewSegment(REFERENCE_TIME tStart, REFERENCE_TIME tStop, double dRate);

// IMemInputPin::

  HRESULT __stdcall GetAllocator(IMemAllocator** ppAllocator);
  HRESULT __stdcall NotifyAllocator(IMemAllocator* pAllocator, BOOL bReadOnly);
  HRESULT __stdcall GetAllocatorRequirements(ALLOCATOR_PROPERTIES* pProps);
  HRESULT __stdcall Receive(IMediaSample* pSamples);
  HRESULT __stdcall ReceiveMultiple(IMediaSample** ppSamples, long nSamples, long* nSamplesProcessed);
  HRESULT __stdcall ReceiveCanBlock();

private:
  /// \param bInvertFrames[out] If true, the image is flipped vertically compared to the expected orientation of vi.pixel_type
  HRESULT InternalQueryAccept(const AM_MEDIA_TYPE* pmt, VideoInfo &vi, bool &bInvertFrames);

};


static bool HasNoConnectedOutputPins(IBaseFilter* bf);
static void DisconnectAllPinsAndRemoveFilter(IGraphBuilder* gb, IBaseFilter* bf);
static void RemoveUselessFilters(IGraphBuilder* gb, IBaseFilter* not_this_one, IBaseFilter* nor_this_one);
static HRESULT AttemptConnectFilters(IGraphBuilder* gb, IBaseFilter* connect_filter);
static void SetMicrosoftDVtoFullResolution(IGraphBuilder* gb);


class DirectShowSource : public IClip {

  __int64 sampleStartTime;
  __int64 next_sample;

  GetSample get_sample;
  IGraphBuilder* gb;

  PVideoFrame currentFrame;

  VideoInfo vi;
  bool frame_units;
  bool convert_fps;
  int cur_frame;
  int seekmode;
  int audio_bytes_read;

  const bool TrapTimeouts;
  const DWORD WaitTimeout;

  LOG* log;

  void CheckHresult(IScriptEnvironment* env, HRESULT hr, const char* msg, const char* msg2 = "");
  HRESULT LoadGraphFile(IGraphBuilder *pGraph, const WCHAR* wszName);
  void cleanUp();
  void SetMicrosoftDVtoFullResolution(IGraphBuilder* gb);
  void DisableDeinterlacing(IFilterGraph *pGraph);
  void SetWMAudioDecoderDMOtoHiResOutput(IFilterGraph *pGraph);

public:

  DirectShowSource(const char* filename, int _avg_time_per_frame, int _seekmode, bool _enable_audio, bool _enable_video,
                   bool _convert_fps, unsigned _media, int _timeout, int _frames, LOG* _log, IScriptEnvironment* env);
  ~DirectShowSource();
  const VideoInfo& __stdcall GetVideoInfo() { return vi; }
  PVideoFrame __stdcall GetFrame(int n, IScriptEnvironment* env);


  bool __stdcall GetParity(int n) { return false; }
  int __stdcall SetCacheHints(int cachehints,int frame_range);

  void __stdcall GetAudio(void* buf, __int64 start, __int64 count, IScriptEnvironment* env);

};


AVSValue __cdecl Create_DirectShowSource(AVSValue args, void*, IScriptEnvironment* env);
