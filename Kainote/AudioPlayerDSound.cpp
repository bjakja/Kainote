//  Copyright (c) 2016-2026, Marcin Drob

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

//this code piervously was taken from Aegisub 2 it's rewritten by me almost all.

///////////
// Headers

#include "AudioPlayerDSound.h"
#include <process.h>
#include "LogHandler.h"
#include "kainoteApp.h"
#include "UtilsWindows.h"

#ifndef _WIN32
#include <SDL.h>
#include <algorithm>
#include <chrono>
#include <condition_variable>
#include <cstring>
#include <mutex>
#include <thread>
#include <vector>

struct DirectSoundPlayer2Thread::LinuxAudioState {
	std::thread thread;
	std::mutex mutex;
	std::condition_variable cv;
	bool running = true;
	bool initialized = false;
	bool failed = false;
	bool playing = false;
	bool restart = false;
	long long currentFrame = 0;
	std::chrono::steady_clock::time_point playbackStart;
	SDL_AudioDeviceID device = 0;
	SDL_AudioSpec obtained{};
};

void DirectSoundPlayer2Thread::Run()
{
	if (SDL_InitSubSystem(SDL_INIT_AUDIO) != 0) {
		KaiLog(wxString::Format(L"Cannot initialize SDL audio: %s", wxString::FromUTF8(SDL_GetError())));
		std::lock_guard<std::mutex> lock(linuxState->mutex);
		linuxState->failed = true;
		linuxState->initialized = true;
		linuxState->cv.notify_all();
		SDL_QuitSubSystem(SDL_INIT_AUDIO);
		return;
	}

	SDL_AudioSpec want{};
	want.freq = provider->GetSampleRate();
	want.format = AUDIO_S16SYS;
	want.channels = static_cast<Uint8>(std::max(1, provider->GetChannels()));
	want.samples = static_cast<Uint16>(std::clamp((want.freq * wanted_latency) / 1000, 256, 4096));
	want.callback = nullptr;

	linuxState->device = SDL_OpenAudioDevice(nullptr, 0, &want, &linuxState->obtained, 0);
	if (!linuxState->device) {
		KaiLog(wxString::Format(L"Cannot open SDL audio device: %s", wxString::FromUTF8(SDL_GetError())));
		std::lock_guard<std::mutex> lock(linuxState->mutex);
		linuxState->failed = true;
		linuxState->initialized = true;
		linuxState->cv.notify_all();
		SDL_QuitSubSystem(SDL_INIT_AUDIO);
		return;
	}

	if (linuxState->obtained.freq != want.freq || linuxState->obtained.format != want.format || linuxState->obtained.channels != want.channels) {
		KaiLog(L"SDL audio device does not support Kainote PCM format");
		SDL_CloseAudioDevice(linuxState->device);
		linuxState->device = 0;
		std::lock_guard<std::mutex> lock(linuxState->mutex);
		linuxState->failed = true;
		linuxState->initialized = true;
		linuxState->cv.notify_all();
		SDL_QuitSubSystem(SDL_INIT_AUDIO);
		return;
	}

	SDL_PauseAudioDevice(linuxState->device, 0);
	{
		std::lock_guard<std::mutex> lock(linuxState->mutex);
		linuxState->initialized = true;
		linuxState->cv.notify_all();
	}

	const int bytesPerFrame = provider->GetChannels() * provider->GetBytesPerSample();
	const int maxQueuedFrames = std::max(1024, provider->GetSampleRate() * wanted_latency * buffer_length / 1000);
	const int chunkFrames = std::max(256, std::min(4096, provider->GetSampleRate() * wanted_latency / 1000));
	std::vector<unsigned char> chunk(static_cast<size_t>(chunkFrames) * bytesPerFrame);

	while (true) {
		long long requestStart = 0;
		long long requestEnd = 0;
		bool restartPlayback = false;
		{
			std::unique_lock<std::mutex> lock(linuxState->mutex);
			linuxState->cv.wait_for(lock, std::chrono::milliseconds(10), [&] {
				return !linuxState->running || linuxState->playing || linuxState->restart;
			});
			if (!linuxState->running)
				break;
			if (!linuxState->playing) {
				SDL_ClearQueuedAudio(linuxState->device);
				continue;
			}
			restartPlayback = linuxState->restart;
			linuxState->restart = false;
			requestStart = start_frame;
			requestEnd = end_frame;
			if (restartPlayback) {
				linuxState->currentFrame = requestStart;
				linuxState->playbackStart = std::chrono::steady_clock::now();
			}
		}

		if (restartPlayback)
			SDL_ClearQueuedAudio(linuxState->device);

		const Uint32 queuedBytes = SDL_GetQueuedAudioSize(linuxState->device);
		const int queuedFrames = bytesPerFrame ? static_cast<int>(queuedBytes / bytesPerFrame) : 0;
		if (queuedFrames < maxQueuedFrames) {
			long long frame = 0;
			long long framesToWrite = 0;
			{
				std::lock_guard<std::mutex> lock(linuxState->mutex);
				frame = linuxState->currentFrame;
				framesToWrite = std::min<long long>(chunkFrames, std::max<long long>(0, requestEnd - frame));
			}

			if (framesToWrite > 0) {
				provider->GetBuffer(chunk.data(), frame, framesToWrite, volume);
				if (SDL_QueueAudio(linuxState->device, chunk.data(), static_cast<Uint32>(framesToWrite * bytesPerFrame)) != 0) {
					KaiLogSilent(wxString::Format(L"SDL audio queue failed: %s", wxString::FromUTF8(SDL_GetError())));
				}
				std::lock_guard<std::mutex> lock(linuxState->mutex);
				linuxState->currentFrame += framesToWrite;
			}
			else if (queuedBytes == 0) {
				std::lock_guard<std::mutex> lock(linuxState->mutex);
				linuxState->playing = false;
			}
		}
	}

	if (linuxState->device) {
		SDL_ClearQueuedAudio(linuxState->device);
		SDL_CloseAudioDevice(linuxState->device);
		linuxState->device = 0;
	}
	SDL_QuitSubSystem(SDL_INIT_AUDIO);
}

unsigned int DirectSoundPlayer2Thread::FillAndUnlockBuffers(unsigned char*, unsigned int, unsigned char*, unsigned int,
	long long&, IDirectSoundBuffer8*)
{
	return 0;
}

void DirectSoundPlayer2Thread::CheckError()
{
	if (!linuxState)
		return;

	std::lock_guard<std::mutex> lock(linuxState->mutex);
	const bool failed = linuxState->failed;
	if (!failed)
		return;

	throw _T("SDL audio player failed.");
}

DirectSoundPlayer2Thread::DirectSoundPlayer2Thread(Provider *provider, int _WantedLatency, int _BufferLength)
{
	wanted_latency = _WantedLatency;
	buffer_length = _BufferLength;
	last_playback_restart = 0;
	volume = 1.0;
	start_frame = 0;
	end_frame = 0;
	this->provider = provider;
	linuxState = new LinuxAudioState();
	linuxState->thread = std::thread([this] { Run(); });

	std::unique_lock<std::mutex> lock(linuxState->mutex);
	linuxState->cv.wait(lock, [this] { return linuxState->initialized; });
	const bool failed = linuxState->failed;
	lock.unlock();
	if (failed) {
		if (linuxState->thread.joinable())
			linuxState->thread.join();
		delete linuxState;
		linuxState = nullptr;
		throw _T("Failed creating SDL audio playback device.");
	}
}

DirectSoundPlayer2Thread::~DirectSoundPlayer2Thread()
{
	if (linuxState) {
		{
			std::lock_guard<std::mutex> lock(linuxState->mutex);
			linuxState->running = false;
			linuxState->playing = false;
			linuxState->cv.notify_all();
		}
		if (linuxState->thread.joinable())
			linuxState->thread.join();
		delete linuxState;
		linuxState = nullptr;
	}
}

void DirectSoundPlayer2Thread::Play(long long start, long long count)
{
	CheckError();
	std::lock_guard<std::mutex> lock(linuxState->mutex);
	start_frame = start;
	end_frame = start + count;
	linuxState->currentFrame = start;
	linuxState->playbackStart = std::chrono::steady_clock::now();
	linuxState->playing = true;
	linuxState->restart = true;
	last_playback_restart = static_cast<int>(timeGetTime());
	linuxState->cv.notify_all();
}

void DirectSoundPlayer2Thread::Stop()
{
	CheckError();
	std::lock_guard<std::mutex> lock(linuxState->mutex);
	linuxState->playing = false;
	linuxState->restart = false;
	linuxState->cv.notify_all();
}

void DirectSoundPlayer2Thread::SetEndFrame(long long new_end_frame)
{
	CheckError();
	std::lock_guard<std::mutex> lock(linuxState->mutex);
	end_frame = new_end_frame;
	if (linuxState->currentFrame >= end_frame)
		linuxState->playing = false;
	linuxState->cv.notify_all();
}

void DirectSoundPlayer2Thread::SetVolume(double new_volume)
{
	CheckError();
	std::lock_guard<std::mutex> lock(linuxState->mutex);
	volume = new_volume;
}

bool DirectSoundPlayer2Thread::IsPlaying()
{
	CheckError();
	std::lock_guard<std::mutex> lock(linuxState->mutex);
	return linuxState->playing;
}

long long DirectSoundPlayer2Thread::GetStartFrame()
{
	CheckError();
	return start_frame;
}

int DirectSoundPlayer2Thread::GetCurrentMS()
{
	CheckError();
	if (!IsPlaying()) return 0;
	return static_cast<int>(timeGetTime() - last_playback_restart);
}

long long DirectSoundPlayer2Thread::GetCurrentFrame()
{
	CheckError();
	std::lock_guard<std::mutex> lock(linuxState->mutex);
	if (!linuxState->playing) return 0;
	const auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(
		std::chrono::steady_clock::now() - linuxState->playbackStart).count();
	long long audibleFrame = start_frame + elapsed * provider->GetSampleRate() / 1000;
	return std::min(audibleFrame, end_frame);
}

long long DirectSoundPlayer2Thread::GetEndFrame()
{
	CheckError();
	return end_frame;
}

double DirectSoundPlayer2Thread::GetVolume()
{
	CheckError();
	std::lock_guard<std::mutex> lock(linuxState->mutex);
	return volume;
}

bool DirectSoundPlayer2Thread::IsDead()
{
	if (!linuxState) return true;
	std::lock_guard<std::mutex> lock(linuxState->mutex);
	return linuxState->failed;
}

#else

unsigned int __stdcall DirectSoundPlayer2Thread::ThreadProc(void *parameter)
{
	static_cast<DirectSoundPlayer2Thread*>(parameter)->Run();
	return 0;
}


void DirectSoundPlayer2Thread::Run()
{
	// Create DirectSound object
	IDirectSound8 * defaultPlayback = nullptr;
	HRESULT hr = DS_OK;
	
	hr = DirectSoundCreate8(&DSDEVID_DefaultPlayback, &defaultPlayback, 0);
	if (!defaultPlayback) {
		KaiLog("Cannot create DirectSound object"); 
		SetEvent(error_happened);
		return;
	}
	
	// Ensure we can get interesting wave formats (unless we have PRIORITY we can only use a standard 8 bit format)
	hr = defaultPlayback->SetCooperativeLevel(KainoteFrame::Get()->GetHWND(), DSSCL_PRIORITY);
	if (hr != DS_OK) {
		KaiLog("Cannot create set cooperativeLevel"); 
		defaultPlayback->Release();
		SetEvent(error_happened);
		return;
	}

	// Describe the wave format
	WAVEFORMATEX waveFormat;
	waveFormat.wFormatTag = WAVE_FORMAT_PCM;
	waveFormat.nSamplesPerSec = provider->GetSampleRate();
	waveFormat.nChannels = provider->GetChannels();
	waveFormat.wBitsPerSample = provider->GetBytesPerSample() * 8;
	waveFormat.nBlockAlign = waveFormat.nChannels * waveFormat.wBitsPerSample / 8;
	waveFormat.nAvgBytesPerSec = waveFormat.nSamplesPerSec * waveFormat.nBlockAlign;
	waveFormat.cbSize = sizeof(waveFormat);

	// And the buffer itself
	int aim = waveFormat.nAvgBytesPerSec * (wanted_latency * buffer_length) / 1000;
	int min = DSBSIZE_MIN;
	int max = DSBSIZE_MAX;
	DWORD bufSize = MIN(MAX(min, aim), max); // size of entier playback buffer
	DSBUFFERDESC desc;
	desc.dwSize = sizeof(DSBUFFERDESC);
	desc.dwFlags = DSBCAPS_GETCURRENTPOSITION2 | DSBCAPS_GLOBALFOCUS;
	desc.dwBufferBytes = bufSize;
	desc.dwReserved = 0;
	desc.lpwfxFormat = &waveFormat;
	desc.guid3DAlgorithm = GUID_NULL;

	// And then create the buffer
	IDirectSoundBuffer *audioBuffer7 = 0;
	if FAILED(defaultPlayback->CreateSoundBuffer(&desc, &audioBuffer7, 0)) {
		KaiLog("Could not create buffer"); 
		defaultPlayback->Release();
		SetEvent(error_happened);
		return;
	}
		// But it's an old version interface we get, query it for the DSound8 interface
	IDirectSoundBuffer8 * audioBuffer = nullptr;
	if (FAILED(audioBuffer7->QueryInterface(IID_IDirectSoundBuffer8, (LPVOID*)&audioBuffer)))
		KaiLog("Buffer doesn't support version 8 interface");

	audioBuffer7->Release();
	audioBuffer7 = nullptr;

	if (!audioBuffer) {
		SetEvent(error_happened);
		return;
	}

	// Now we're ready to roll!
	SetEvent(thread_running);
	bool running = true;

	HANDLE events_to_wait[] = {
		event_start_playback,
		event_stop_playback,
		event_update_end_time,
		event_set_volume,
		event_kill_self
	};

	long long next_input_frame = 0;
	unsigned long buffer_offset = 0;
	bool playback_should_be_running = false;
	int current_latency = wanted_latency;
	const DWORD wanted_latency_bytes = wanted_latency * waveFormat.nSamplesPerSec * provider->GetBytesPerSample() / 1000;

	while (running)
	{
		DWORD wait_result = WaitForMultipleObjects(sizeof(events_to_wait)/sizeof(HANDLE), 
			events_to_wait, FALSE, current_latency);

		switch (wait_result)
		{
		case WAIT_OBJECT_0 + 0:
			{
				// Start or restart playback
				audioBuffer->Stop();
				ResetEvent(is_playing);

				next_input_frame = start_frame;

				unsigned long buf_size; // size of buffer locked for filling
				unsigned char *buf = nullptr;
				buffer_offset = 0;

				if (FAILED(audioBuffer->SetCurrentPosition(0)))
					KaiLogSilent("Could not reset playback buffer cursor before filling first buffer.");

				HRESULT res = audioBuffer->Lock(buffer_offset, 0, (void **) &buf, &buf_size, 0, 0, DSBLOCK_ENTIREBUFFER);
				while (FAILED(res)) // yes, while, so I can break out of it without a goto!
				{
					if (res == DSERR_BUFFERLOST)
					{
						// Try to regain the buffer
						if (SUCCEEDED(audioBuffer->Restore()) &&
							SUCCEEDED(audioBuffer->Lock(buffer_offset, 0, ((void**) &buf), &buf_size, 0, 0, DSBLOCK_ENTIREBUFFER)))
						{
							break;
						}

						KaiLogSilent("Lost buffer and could not restore it.");
					}

					KaiLogSilent("Could not lock buffer for playback.");
				}

				// Clear the buffer in case we can't fill it completely
				memset(buf, 0, buf_size);

				DWORD bytes_filled = FillAndUnlockBuffers(buf, buf_size, 0, 0, next_input_frame, audioBuffer);
				buffer_offset += bytes_filled;
				if (buffer_offset >= bufSize) buffer_offset -= bufSize;

				if (FAILED(audioBuffer->SetCurrentPosition(0)))
					KaiLogSilent("Could not reset playback buffer cursor before playback.");

					if (bytes_filled < wanted_latency_bytes)
					{
						// Very short playback length, do without streaming playback
						current_latency = (bytes_filled * 1000) / (waveFormat.nSamplesPerSec*provider->GetBytesPerSample());
						current_latency = (bytes_filled * 1000) / (waveFormat.nSamplesPerSec*provider->GetBytesPerSample());
						if (FAILED(audioBuffer->Play(0, 0, 0)))
							KaiLogSilent("Could not start single-buffer playback.");
					}
					else
					{
						// We filled the entire buffer so there's reason to do streaming playback
						current_latency = wanted_latency;
						if (FAILED(audioBuffer->Play(0, 0, DSBPLAY_LOOPING)))
							KaiLogSilent("Could not start looping playback.");
					}

					SetEvent(is_playing);
					playback_should_be_running = true;

					break;
			}

		case WAIT_OBJECT_0 + 1:
			{
				// Stop playing
				audioBuffer->Stop();
				ResetEvent(is_playing);
				playback_should_be_running = false;
				break;
			}

		case WAIT_OBJECT_0 + 2:
			{
				// Set end frame
				if (end_frame <= next_input_frame)
				{
					audioBuffer->Stop();
					ResetEvent(is_playing);
					playback_should_be_running = false;
				}
				else
				{
					// If the user is dragging the start or end point in the audio display
					// the set end frame events might come in faster than the timeouts happen
					// and then new data never get filled into the buffer. See bug #915.
					goto do_fill_buffer;
				}
				break;
			}

		case WAIT_OBJECT_0 + 3:
			{
				// Change volume
				// We aren't thread safe right now, filling the buffers grabs volume directly
				// from the field set by the controlling thread, but it shouldn't be a major
				// problem if race conditions do occur, just some momentary distortion.
				goto do_fill_buffer;
			}

		case WAIT_OBJECT_0 + 4:
			{
				// Perform suicide
				audioBuffer->Stop();
				ResetEvent(is_playing);
				playback_should_be_running = false;
				running = false;
				break;
			}

		case WAIT_TIMEOUT:
do_fill_buffer:
			{
				// Time to fill more into buffer

				if (!playback_should_be_running)
					break;

				DWORD status;
				if (FAILED(audioBuffer->GetStatus(&status)))
					KaiLogDebug("Could not get playback buffer status");

					if (!(status & DSBSTATUS_LOOPING))
					{
						// Not looping playback...
						// hopefully we only triggered timeout after being done with the buffer
						audioBuffer->Stop();
						ResetEvent(is_playing);
						playback_should_be_running = false;
						break;
					}

					DWORD play_cursor;
					if (FAILED(audioBuffer->GetCurrentPosition(&play_cursor, 0)))
						KaiLogDebug("Could not get play cursor position for filling buffer.");

						int bytes_needed = (int)play_cursor - (int)buffer_offset;
					if (bytes_needed < 0) bytes_needed += (int)bufSize;

					// Requesting zero buffer makes Windows cry, and zero buffer seemed to be
					// a common request on Windows 7. (Maybe related to the new timer coalescing?)
					// We'll probably get non-zero bytes requested on the next iteration.
					if (bytes_needed == 0)
						break;

					unsigned long buf1sz, buf2sz;
					unsigned char *buf1 = nullptr, *buf2 = nullptr;

					//assert(bytes_needed > 0);
					//assert(buffer_offset < bufSize);
					//assert((unsigned long)bytes_needed <= bufSize);

					HRESULT res = audioBuffer->Lock(buffer_offset, bytes_needed, (void** )&buf1, &buf1sz, (void** )&buf2, &buf2sz, 0);
					switch (res)
					{
					case DSERR_BUFFERLOST:
						// Try to regain the buffer
						// When the buffer was lost the entire contents was lost too, so we have to start over
						if (SUCCEEDED(audioBuffer->Restore()) &&
							SUCCEEDED(audioBuffer->Lock(0, bufSize, (void**)&buf1, &buf1sz, (void**)&buf2, &buf2sz, 0)) &&
							SUCCEEDED(audioBuffer->Play(0, 0, DSBPLAY_LOOPING)))
						{
							KaiLogDebug("DirectSoundPlayer2: Lost and restored buffer");
							break;
						}
						KaiLogDebug("Lost buffer and could not restore it.");

					case DSERR_INVALIDPARAM:
						KaiLogDebug("Invalid parameters to IDirectSoundBuffer8::Lock().");

					case DSERR_INVALIDCALL:
						KaiLogDebug("Invalid call to IDirectSoundBuffer8::Lock().");

					case DSERR_PRIOLEVELNEEDED:
						KaiLogDebug("Incorrect priority level set on DirectSoundBuffer8 object.");

					default:
						if (FAILED(res))
							KaiLogDebug("Could not lock , unknown error.");
							break;
					}

					DWORD bytes_filled = FillAndUnlockBuffers(buf1, buf1sz, buf2, buf2sz, next_input_frame, audioBuffer);
					buffer_offset += bytes_filled;
					if (buffer_offset >= bufSize) buffer_offset -= bufSize;

					if (bytes_filled < 1024)
					{
						// Arbitrary low number, we filled in very little so better get back to filling in the rest with silence
						// really fast... set latency to zero in this case.
						current_latency = 0;
					}
					else if (bytes_filled < wanted_latency_bytes)
					{
						// Didn't fill as much as we wanted to, let's get back to filling sooner than normal
						current_latency = (bytes_filled * 1000) / (waveFormat.nSamplesPerSec * provider->GetBytesPerSample());
					}
					else
					{
						// Plenty filled in, do regular latency
						current_latency = wanted_latency;
					}

					break;
			}

		default:
			KaiLogDebug("Something bad happened while waiting on events in playback loop,"\
				"either the wait failed or an event object was abandoned.");
				break;
		}
	}


}


unsigned int DirectSoundPlayer2Thread::FillAndUnlockBuffers(unsigned char* buf1, unsigned int buf1sz, unsigned char* buf2,
	unsigned int buf2sz, long long& input_frame, IDirectSoundBuffer8* audioBuffer)
{
	// Assume buffers have been locked and are ready to be filled

	DWORD bytes_per_frame = provider->GetChannels() * provider->GetBytesPerSample();
	DWORD buf1szf = buf1sz / bytes_per_frame;
	DWORD buf2szf = buf2sz / bytes_per_frame;

	if (input_frame >= end_frame)
	{
		// Silence

		if (buf1)
			memset(buf1, 0, buf1sz);

		if (buf2)
			memset(buf2, 0, buf2sz);

		input_frame += buf1szf + buf2szf;

		audioBuffer->Unlock(buf1, buf1sz, buf2, buf2sz); // should be checking for success

		return buf1sz + buf2sz;
	}

	if (buf1 && buf1sz)
	{
		if (buf1szf + input_frame > end_frame)
		{
			buf1szf = end_frame - input_frame;
			buf1sz = buf1szf * bytes_per_frame;
			buf2szf = 0;
			buf2sz = 0;
		}

		provider->GetBuffer(buf1, input_frame, buf1szf, volume);

		input_frame += buf1szf;
	}

	if (buf2 && buf2sz)
	{
		if (buf2szf + input_frame > end_frame)
		{
			buf2szf = end_frame - input_frame;
			buf2sz = buf2szf * bytes_per_frame;
		}

		provider->GetBuffer(buf2, input_frame, buf2szf, volume);

		input_frame += buf2szf;
	}

	audioBuffer->Unlock(buf1, buf1sz, buf2, buf2sz); // bad? should check for success

	return buf1sz + buf2sz;
}


void DirectSoundPlayer2Thread::CheckError()
{
	try
	{
		switch (WaitForSingleObject(error_happened, 0))
		{
		case WAIT_OBJECT_0:
			//throw error_message;

		case WAIT_ABANDONED:
			throw _T("The DirectShowPlayer2Thread error signal event was abandoned, somehow. This should not happen.");

		case WAIT_FAILED:
			throw _T("Failed checking state of DirectShowPlayer2Thread error signal event.");

		case WAIT_TIMEOUT:
		default:
			return;
		}
	}
	catch (...)
	{
		ResetEvent(is_playing);
		ResetEvent(thread_running);
		throw;
	}
}


DirectSoundPlayer2Thread::DirectSoundPlayer2Thread(Provider *provider, int _WantedLatency, int _BufferLength)
	: event_start_playback  (CreateEvent(0, FALSE, FALSE, 0))
	, event_stop_playback   (CreateEvent(0, FALSE, FALSE, 0))
	, event_update_end_time (CreateEvent(0, FALSE, FALSE, 0))
	, event_set_volume      (CreateEvent(0, FALSE, FALSE, 0))
	, event_kill_self       (CreateEvent(0, FALSE, FALSE, 0))
	, thread_running        (CreateEvent(0,  TRUE, FALSE, 0))
	, is_playing            (CreateEvent(0,  TRUE, FALSE, 0))
	, error_happened        (CreateEvent(0, FALSE, FALSE, 0))
{
	

	wanted_latency	= _WantedLatency;
	buffer_length	= _BufferLength;
	last_playback_restart=0;

	this->provider = provider;
	unsigned int threadid = 0;
	thread_handle = (HANDLE)_beginthreadex(0, 0, ThreadProc, this, 0, &threadid);
	SetThreadName(threadid, "AudioThread");

	if (!thread_handle)
		throw _T("Failed creating playback thread in DirectSoundPlayer2. This is bad.");

	HANDLE running_or_error[] = { thread_running, error_happened };
	switch (WaitForMultipleObjects(2, running_or_error, FALSE, INFINITE))
	{
	case WAIT_OBJECT_0:
		// running, all good
		return;


	default:
		throw _T("Failed wait for thread start or thread error in DirectSoundPlayer2. This is bad.");
	}
}


DirectSoundPlayer2Thread::~DirectSoundPlayer2Thread()
{
	SetEvent(event_kill_self);
	WaitForSingleObject(thread_handle, 2000);
}


void DirectSoundPlayer2Thread::Play(long long start, long long count)
{
	CheckError();

	start_frame = start;
	end_frame = start+count;
	SetEvent(event_start_playback);
	//SYSTEMTIME ftime;
	//GetSystemTime (&ftime);
	//std::chrono::steady_clock::now();//
	last_playback_restart = (int)timeGetTime();
	//sw.Start (0);
}


void DirectSoundPlayer2Thread::Stop()
{
	CheckError();
	//sw.Pause ();
	SetEvent(event_stop_playback);
}


void DirectSoundPlayer2Thread::SetEndFrame(long long new_end_frame)
{
	CheckError();

	end_frame = new_end_frame;
	//if(end_frame<GetCurrentFrame()){Stop();}
	//else{
	SetEvent(event_update_end_time);//}
}


void DirectSoundPlayer2Thread::SetVolume(double new_volume)
{
	CheckError();

	volume = new_volume;
	//SetEvent(event_set_volume);
}


bool DirectSoundPlayer2Thread::IsPlaying()
{
	CheckError();

	switch (WaitForSingleObject(is_playing, 0))
	{
	case WAIT_ABANDONED:
		throw _T("The DirectShowPlayer2Thread playback state event was abandoned, somehow. This should not happen.");

	case WAIT_FAILED:
		throw _T("Failed checking state of DirectShowPlayer2Thread playback state event.");

	case WAIT_OBJECT_0:
		return true;

	case WAIT_TIMEOUT:
	default:
		return false;
	}
}


long long DirectSoundPlayer2Thread::GetStartFrame()
{
	CheckError();

	return start_frame;
}

int DirectSoundPlayer2Thread::GetCurrentMS()
{
	CheckError();

	if (!IsPlaying()) return 0;

	int milliseconds_elapsed = (int)(timeGetTime() - last_playback_restart);
	//int milliseconds_elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::steady_clock::now() - last_playback_restart).count();

	return start_frame + milliseconds_elapsed;
}

long long DirectSoundPlayer2Thread::GetCurrentFrame()
{
	CheckError();

	if (!IsPlaying()) return 0;


	int milliseconds_elapsed = (int)(timeGetTime() - last_playback_restart);
	//int milliseconds_elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::steady_clock::now() - last_playback_restart).count();

	return start_frame + (long long)milliseconds_elapsed * provider->GetSampleRate() / 1000;
}


long long DirectSoundPlayer2Thread::GetEndFrame()
{
	CheckError();

	return end_frame;
}


double DirectSoundPlayer2Thread::GetVolume()
{
	CheckError();

	return volume;
}


bool DirectSoundPlayer2Thread::IsDead()
{
	switch (WaitForSingleObject(thread_running, 0))
	{
	case WAIT_OBJECT_0:
		return false;

	default:
		return true;
	}
}




#endif

DirectSoundPlayer2::DirectSoundPlayer2()
{
	thread = 0;
	provider = 0;

	// The buffer will hold BufferLength times WantedLatency milliseconds of audio
	WantedLatency = 100;
	BufferLength = 5;

	// sanity checking
	if (WantedLatency <= 0)
		WantedLatency = 100;
	if (BufferLength <= 0)
		BufferLength = 5;
}


DirectSoundPlayer2::~DirectSoundPlayer2()
{
	CloseStream();
}


bool DirectSoundPlayer2::IsThreadAlive()
{
	if (!thread) return false;

	if (thread->IsDead())
	{
		delete thread;
		thread = 0;
		return false;
	}

	return true;
}


void DirectSoundPlayer2::OpenStream()
{
	if (IsThreadAlive()) return;

	try
	{
		thread = new DirectSoundPlayer2Thread(provider, WantedLatency, BufferLength);
	}
	catch (const wxChar *msg)
	{
		KaiLog(msg);
		thread = 0;
	}
}


void DirectSoundPlayer2::CloseStream()
{
	if (!IsThreadAlive()) return;

	try
	{
		delete thread;
	}
	catch (const wxChar *msg)
	{
		KaiLog(msg);
	}
	thread = 0;
}


void DirectSoundPlayer2::SetProvider(Provider *_provider)
{
	try
	{
		provider = _provider;
		if (IsThreadAlive())
		{
			delete thread;
			thread = new DirectSoundPlayer2Thread(provider, WantedLatency, BufferLength);
		}
	}
	catch (const wxChar *msg)
	{
		KaiLog(msg);
	}
}


void DirectSoundPlayer2::Play(long long start, long long count)
{
	try
	{
		OpenStream();
		thread->Play(start, count);

		//if (displayTimer && !displayTimer->IsRunning()) displayTimer->Start(30);
	}
	catch (const wxChar *msg)
	{
		KaiLog(msg);
	}
}


void DirectSoundPlayer2::Stop(bool timerToo)
{
	try
	{
		if (IsThreadAlive()) thread->Stop();

		/*if (displayTimer) {
			displayTimer->Stop();
		}*/
	}
	catch (const wxChar *msg)
	{
		KaiLog(msg);
	}
}


bool DirectSoundPlayer2::IsPlaying()
{
	try
	{
		if (!IsThreadAlive()) return false;
		return thread->IsPlaying();
	}
	catch (const wxChar *msg)
	{
		KaiLog(msg);
		return false;
	}
}


long long DirectSoundPlayer2::GetStartPosition()
{
	try
	{
		if (!IsThreadAlive()) return 0;
		return thread->GetStartFrame();
	}
	catch (const wxChar *msg)
	{
		KaiLog(msg);
		return 0;
	}
}


long long DirectSoundPlayer2::GetEndPosition()
{
	try
	{
		if (!IsThreadAlive()) return 0;
		return thread->GetEndFrame();
	}
	catch (const wxChar *msg)
	{
		KaiLog(msg);
		return 0;
	}
}


long long DirectSoundPlayer2::GetCurrentPosition()
{
	try
	{
		if (!IsThreadAlive()) return 0;
		return thread->GetCurrentFrame();
	}
	catch (const wxChar *msg)
	{
		KaiLog(msg);
		return 0;
	}
}

int DirectSoundPlayer2::GetCurPositionMS()
{
	try
	{
		if (!IsThreadAlive()) return 0;
		return thread->GetCurrentMS();
	}
	catch (const wxChar *msg)
	{
		KaiLog(msg);
		return 0;
	}
}


void DirectSoundPlayer2::SetEndPosition(long long pos)
{
	try
	{
		if (IsThreadAlive()) thread->SetEndFrame(pos);
	}
	catch (const wxChar *msg)
	{
		KaiLog(msg);
	}
}


void DirectSoundPlayer2::SetCurrentPosition(long long pos)
{
	try
	{
		if (IsThreadAlive()) thread->Play(pos, thread->GetEndFrame()-pos);
	}
	catch (const wxChar *msg)
	{
		KaiLog(msg);
	}
}


void DirectSoundPlayer2::SetVolume(double vol)
{
	try
	{
		if (IsThreadAlive()) thread->SetVolume(vol);
	}
	catch (const wxChar *msg)
	{
		KaiLog(msg);
	}
}


double DirectSoundPlayer2::GetVolume()
{
	try
	{
		if (!IsThreadAlive()) return 0;
		return thread->GetVolume();
	}
	catch (const wxChar *msg)
	{
		KaiLog(msg);
		return 0;
	}
}




