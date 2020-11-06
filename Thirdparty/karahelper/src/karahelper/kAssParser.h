// tag parser, some parts of it
#pragma once
#include "stdafx.h"
#include "kUtils.h"
#include "kStyle.h"

#define _isnum(v) (v>='0')&&(v<='9')

enum nModType
{
    TAG_NONE = 0,
    TAG_BLUR,	//x \be
    TAG_BE,		//x \blur
    TAG_FS,		//x \fs
    TAG_FSCX,	//x \fscx
    TAG_FSCY,	//x \fscy
    TAG_FSP		//x \fsp
};

struct nModTypeT
{
    nModType type;
    int tagsize;

    nModTypeT(nModType tp, int ts)
    {
        type = tp;
        tagsize = ts;
    };
};

// get type of tag
nModTypeT getTagType(std::wstring tag);

// tagsize: length of tag name: for \bord<xxx> is 4, for \b<xxx> is 1
void getTagParams(std::wstring tag, int tagsize, nStrArray& params);
bool parseASStags(kStyle& style, std::wstring str);
void useASSTag(kStyle& style, std::wstring str, nModTypeT tp);
std::wstring parseLine(std::wstring str, kStyle& style); // !!!