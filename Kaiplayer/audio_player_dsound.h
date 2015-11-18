// Copyright (c) 2006, Rodrigo Braz Monteiro
// All rights reserved.
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions are met:
//
//   * Redistributions of source code must retain the above copyright notice,
//     this list of conditions and the following disclaimer.
//   * Redistributions in binary form must reproduce the above copyright notice,
//     this list of conditions and the following disclaimer in the documentation
//     and/or other materials provided with the distribution.
//   * Neither the name of the Aegisub Group nor the names of its contributors
//     may be used to endorse or promote products derived from this software
//     without specific prior written permission.
//
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
// AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
// IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
// ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE
// LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
// CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
// SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
// INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
// CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
// ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
// POSSIBILITY OF SUCH DAMAGE.
//
// -----------------------------------------------------------------------------
//
// AEGISUB
//
// Website: http://aegisub.cellosoft.com
// Contact: mailto:zeratul@cellosoft.com
//


///////////
// Headers
#ifndef WITH_DIRECTSOUND
#define WITH_DIRECTSOUND

#include <wx/wxprec.h>

#include <stdint.h>
#include <mmsystem.h>
#include <dsound.h>
#include "ffmpeg2.h"

//////////////
// Prototypes
class DirectSoundPlayer;


//////////
// Thread
class DirectSoundPlayerThread : public wxThread {
private:
	DirectSoundPlayer *parent;
	HANDLE stopnotify;

public:
	void Stop(); // Notify thread to stop audio playback. Thread safe.
	DirectSoundPlayerThread(DirectSoundPlayer *parent);
	~DirectSoundPlayerThread();

	wxThread::ExitCode Entry();
};

////////////////////
// Portaudio player
class DirectSoundPlayer : public wxEvtHandler {
	friend class DirectSoundPlayerThread;

private:
	bool playing;
	float volume;
	int offset;
	DWORD bufSize;

	int64_t playPos;
	int64_t startPos;
	int64_t endPos;
	DWORD startTime;

	IDirectSound8 *directSound;
	IDirectSoundBuffer8 *buffer;

	bool FillBuffer(bool fill);

	DirectSoundPlayerThread *thread;

public:
	DirectSoundPlayer();
	virtual ~DirectSoundPlayer();

	void OpenStream(ffmpeg2 *_provider);
	void CloseStream();

	void Play(int64_t start,int64_t count);
	void Stop(bool timerToo=true);
	bool IsPlaying() { return playing; }

	int64_t GetStartPosition() { return startPos; }
	int64_t GetEndPosition() { return endPos; }
	int64_t GetCurrentPosition();
	void SetEndPosition(int64_t pos);
	void SetCurrentPosition(int64_t pos);

	void SetVolume(double vol) { volume = vol; }
	double GetVolume() { return volume; }
	void SetDisplayTimer(wxTimer *timer);

	ffmpeg2 *provider;
	wxTimer *displayTimer;

	//void OnStopAudio(wxCommandEvent &event);
	//DECLARE_EVENT_TABLE()
};


#endif
