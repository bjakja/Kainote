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

#include "indexing.h"
#include "../../include/ffms.h"

extern "C" {
#include <libavutil/avutil.h>
#include <libavutil/sha.h>
}

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

FFMS_API(const char*) FFMS_GetTrackLanguage(FFMS_Indexer* Indexer, int Track) 
{
	return Indexer->GetTrackLanguage(Track);
}
FFMS_API(const char*) FFMS_GetTrackName(FFMS_Indexer* Indexer, int Track)
{
	return Indexer->GetTrackName(Track);
}