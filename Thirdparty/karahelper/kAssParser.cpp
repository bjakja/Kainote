// Advanced subtitle format parser, some parts

#include "stdafx.h"

nModTypeT getTagType(std::wstring tag)
{
    if(tag == L"") return nModTypeT(TAG_NONE, 0);

    int ch = (int)tag[0];
    size_t len = tag.length();
    bool isn = (len > 1) ? _isnum(tag[1]) : false;

    // some switches
    if(len == 1)
    {
        return nModTypeT(TAG_NONE, 0);
    }
    else if((ch >= 'a') && (ch <= 'e'))
    {
        // a..e
        // \be
        if(tag.substr(0, 2) == L"be") return nModTypeT(TAG_BE, 2);
        // \bord
        if(tag.substr(0, 4) == L"blur") return nModTypeT(TAG_BLUR, 4);
    }
    else if(ch == 'f')
    {
        // f
        // \fscx
        if(tag.substr(0, 4) == L"fscx") return nModTypeT(TAG_FSCX, 4);
        // \fscy
        if(tag.substr(0, 4) == L"fscy") return nModTypeT(TAG_FSCY, 4);
        // \fsp
        if(tag.substr(0, 3) == L"fsp") return nModTypeT(TAG_FSP, 3);
        // \fs
        if(tag[1] == 's') return nModTypeT(TAG_FS, 2);
    }
    return nModTypeT(TAG_NONE, 0);
}

bool parseASStags(kStyle& style, std::wstring str)
{
    std::wstring tag;// tag info

    // cycle for all tags
    ptrdiff_t ntag = str.find('\\');
    while(ntag != -1)
    {
        ptrdiff_t nexttag = str.find('\\', 1); // find next tag
        ptrdiff_t nextbrack = str.find('(');  // find bracket

        if((nextbrack < nexttag) && (nextbrack != -1)) // brackets \t(\nya(123,123))
        {
            size_t br = 1, j = nextbrack;
            for(wchar_t c = str[++j]; c; c = str[++j])
            {
                if(c == '(') br++;
                if(c == ')') br--;
                if(br == 0) break;
            }
            nexttag = str.find('\\', j);
        }
        if(nexttag == -1)
        {
            tag = str.substr(1);
            str = L"";
        }
        else
        {
            tag = str.substr(1, nexttag - 1);
            str.erase(0, nexttag);
        }
        // now we have tag
        nModTypeT tp = getTagType(tag);
        // valid tag
        if(tp.type > 0) useASSTag(style, tag, tp);

        // next tag
        ntag = str.find('\\');
    }
    return true;
}

void getTagParams(std::wstring tag, int tagsize, nStrArray& params)
{
    params.push_back(tag.substr(0, tagsize)); // tagname
    std::wstring temp = tag.substr(tagsize);
    // if in brackets:
    ptrdiff_t b1 = temp.find_first_of(L"(");
    ptrdiff_t b2 = temp.find_last_of(L")");
    if(b1 > 0 && b2 > b1) temp = temp.substr(b1 + 1, b2 - b1 - 1);

    while(!temp.empty())
    {
        std::wstring param;
        ptrdiff_t nextp = temp.find(','); // find next tag
        ptrdiff_t nextbrack = temp.find('(');  // find bracket

        if((nextbrack < nextp) && (nextbrack != -1)) // brackets \t(\nya(123,123))
        {
            size_t br = 1, j = nextbrack;
            for(wchar_t c = temp[++j]; c; c = temp[++j])
            {
                if(c == '(') br++;
                if(c == ')') br--;
                if(br == 0) break;
            }
            nextp = temp.find(',', j);
        }
        if(nextp == -1)
        {
            param = temp;
            temp = L"";
        }
        else
        {
            param = temp.substr(0, nextp);
            temp.erase(0, nextp + 1);
        }
        params.push_back(param);
    }
}

// no animation, but it's not difficult to add it (mod array)
void useASSTag(kStyle& style, std::wstring str, nModTypeT tp)
{
    nStrArray params; // tags \b1 and \b(1) are valid
    getTagParams(str, tp.tagsize, params);

    // now we have type and params for it
    switch(tp.type)
    {
    case TAG_BE:	// \be
        style.mod_be(params);
        break;
    case TAG_BLUR:	// \an
        style.mod_blur(params);
        break;
    case TAG_FS:	// \fs
        style.mod_fs(params);
        break;
    case TAG_FSCX:	// \fscx
        style.mod_fscx(params);
        break;
    case TAG_FSCY:	// \fscy
        style.mod_fscy(params);
        break;
    case TAG_FSP:	// \fsp
        style.mod_fsp(params);
        break;
    }
}

std::wstring parseLine(std::wstring str, kStyle& style)
{
    ptrdiff_t i = str.find(L'}');
    std::wstring temp;
    while(!str.empty())
    {
        if(str[0] == '{' && i > 0) // override tag
        {
            if(parseASStags(style, str.substr(1, i - 1)))
                str = str.substr(i + 1);
            i = str.find(L'{'); // next tag
            if(i > 0)
                return str.substr(0, i);
            else
                return str;
            // only first part of string {\nya}Text|{\nya}Text{\nya}123
        }
        else // no tags
        {
            return str;
        }
    }
    return L"";
}