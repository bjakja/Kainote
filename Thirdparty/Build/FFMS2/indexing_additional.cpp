//  Copyright (c) 2021 Drob Marcin
//
//  Permission is hereby granted, free of charge, to any person obtaining a copy
//  of this software and associated documentation files (the "Software"), to deal
//  in the Software without restriction, including without limitation the rights
//  to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
//  copies of the Software, and to permit persons to whom the Software is
//  furnished to do so, subject to the following conditions:
//
//  The above copyright notice and this permission notice shall be included in
//  all copies or substantial portions of the Software.
//
//  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
//  IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
//  FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
//  AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
//  LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
//  OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
//  THE SOFTWARE.

#include "../../ffms2/src/core/indexing.h"
#include "../../ffms2/include/ffms.h"


const char* FFMS_Indexer::GetTrackName(int Track)
{
	AVDictionary* d = FormatContext->streams[Track]->metadata;
	AVDictionaryEntry* t = av_dict_get(d, "title", nullptr, 0);
	if (t) {
		return t->value;
	}
	return nullptr;
}

const char* FFMS_Indexer::GetTrackLanguage(int Track)
{
	AVDictionary* d = FormatContext->streams[Track]->metadata;
	AVDictionaryEntry* t = av_dict_get(d, "language", nullptr, 0);
	if (t) {
		return t->value;
	}
	return nullptr;
}

FFMS_Chapters* FFMS_Indexer::GetChapters()
{
	if (!FormatContext->nb_chapters)
		return nullptr;

	FFMS_Chapters *Chapters = new FFMS_Chapters;
	Chapters->Chapters = new FFMS_Chapter[FormatContext->nb_chapters];
	Chapters->NumOfChapters = FormatContext->nb_chapters;
	for (int i = 0; i < FormatContext->nb_chapters; i++) {
		AVChapter* chapter = FormatContext->chapters[i];
		int den = chapter->time_base.den / 1000.f;
		Chapters->Chapters[i].Start = chapter->start / den;
		Chapters->Chapters[i].End = chapter->end / den;
		AVDictionaryEntry* e = av_dict_get(chapter->metadata, "title", nullptr, 0);
		Chapters->Chapters[i].Title = (e) ? e->value : nullptr;
	}
	return Chapters;
}

FFMS_Attachment* FFMS_Indexer::GetAttachment(int Track)
{
	if (FormatContext->streams[Track]->codecpar->codec_type == AVMEDIA_TYPE_ATTACHMENT) {
		FFMS_Attachment* Attachment = new FFMS_Attachment();
		AVStream* stream = FormatContext->streams[Track];
		AVDictionary* d = stream->metadata;
		AVDictionaryEntry* tf = av_dict_get(d, "filename", 0, 0);
		AVDictionaryEntry* tm = av_dict_get(d, "mimetype", 0, 0);
		Attachment->Filename = (tf) ? tf->value : nullptr;
		Attachment->Mimetype = (tm) ? tm->value : nullptr;
		Attachment->DataSize = stream->codecpar->extradata_size;
		Attachment->Data = stream->codecpar->extradata;
		return Attachment;
	}
	return nullptr;
}

void FFMS_Indexer::GetSubtitles(int Track, GetSubtitlesCallback IC, void* ICPrivate)
{
	if (FormatContext->streams[Track]->codecpar->codec_type == AVMEDIA_TYPE_SUBTITLE) {
		AVPacket* Packet = av_packet_alloc();
		AVStream* stream = FormatContext->streams[Track];
		if (Packet) {
			av_seek_frame(FormatContext, Track, 0, 0);
			while (av_read_frame(FormatContext, Packet) >= 0) {
				if (Packet->stream_index == Track) {
					if (Packet->size < 2) {
						av_packet_unref(Packet);
						continue;
					}
					if (IC(Packet->pts / Packet->time_base.den, Packet->duration / Packet->time_base.den,
						stream->duration / (stream->time_base.den / 1000.f),
						(char*)Packet->data, ICPrivate) == 1)
						break;
				}
				av_packet_unref(Packet);
			}
			av_packet_unref(Packet);
			av_packet_free(&Packet);
		}
	}
}

const char* FFMS_Indexer::GetSubtitleExtradata(int Track)
{
	AVStream* stream = FormatContext->streams[Track];
	if (stream->codecpar->codec_type == AVMEDIA_TYPE_SUBTITLE) {
		if (stream->codecpar->extradata_size) {
			return (char*)stream->codecpar->extradata;
		}
	}
	return nullptr;
}

const char* FFMS_Indexer::GetSubtitleFormat(int Track)
{
	AVCodecID CodecID = FormatContext->streams[Track]->codecpar->codec_id;
	if (CodecID == AV_CODEC_ID_ASS)
		return "ass";
	else if (CodecID == AV_CODEC_ID_SSA)
		return "ssa";
	else {
		const AVCodec *codec = avcodec_find_decoder(CodecID);
		if (codec)
			return codec->name;
	}
	return "";
}

FFMS_API(const char*) FFMS_GetTrackLanguage(FFMS_Indexer* Indexer, int Track) 
{
	return Indexer->GetTrackLanguage(Track);
}

FFMS_API(FFMS_Chapters*) FFMS_GetChapters(FFMS_Indexer* Indexer)
{
	return Indexer->GetChapters();
}

FFMS_API(void) FFMS_FreeChapters(FFMS_Chapters** Chapters)
{
	delete[](*Chapters)->Chapters;
	delete (*Chapters);
}

FFMS_API(FFMS_Attachment*) FFMS_GetAttachment(FFMS_Indexer* Indexer, int Track)
{
	return Indexer->GetAttachment(Track);
}

FFMS_API(void) FFMS_FreeAttachment(FFMS_Attachment** Attachment)
{
	delete (*Attachment);
}

FFMS_API(const char*) FFMS_GetSubtitleExtradata(FFMS_Indexer* Indexer, int Track)
{
	return Indexer->GetSubtitleExtradata(Track);
}

FFMS_API(const char*) FFMS_GetSubtitleFormat(FFMS_Indexer* Indexer, int Track)
{
	return Indexer->GetSubtitleFormat(Track);
}

FFMS_API(void) FFMS_GetSubtitles(FFMS_Indexer* Indexer, int Track, GetSubtitlesCallback IC, void* ICPrivate)
{
	return Indexer->GetSubtitles(Track, IC, ICPrivate);
}

FFMS_API(const char*) FFMS_GetTrackName(FFMS_Indexer* Indexer, int Track)
{
	return Indexer->GetTrackName(Track);
}