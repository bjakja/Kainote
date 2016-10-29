/*****************************************************************************
 * csri: common subtitle renderer interface
 *****************************************************************************
 * Copyright (C) 2007  David Lamparter
 * All rights reserved.
 * 	
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 
 *  - Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 *  - Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *  - The name of the author may not be used to endorse or promote products
 *    derived from this software without specific prior written permission.
 * 
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
 * OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
 * IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT
 * NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
 * THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE
 ****************************************************************************/

#include <wx/wx.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <mutex>
#include "Vsfilterapi.h"
//enumerate
static void csrilib_enum_dir(const wchar_t *dir);

std::mutex mtx;

static const char *get_errstr()
{
	static char msg[2048];
	DWORD err = GetLastError();
	
	if (!FormatMessageA(
		FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
		NULL, err, 0, msg, sizeof(msg), NULL))
		strcpy(msg, "Unknown Error");
	else {
		size_t msglen = strlen(msg) - 1;
		if (msg[msglen] == '\n')
			msg[msglen] = '\0';
	}
	return msg;
}

static void csrilib_add(csri_rend *rend,
	const struct csri_wrap_rend *tmp, struct csri_info *info)
{
	struct csri_wrap_rend *wrend = (struct csri_wrap_rend *)
		malloc(sizeof(struct csri_wrap_rend));
	if (!wrend)
		return;
	memcpy(wrend, tmp, sizeof(struct csri_wrap_rend));
	wrend->rend = rend;
	wrend->info = info;
	csrilib_rend_initadd(wrend);
}

static void csrilib_do_load(const wchar_t *filename)
{
	HMODULE dlhandle = LoadLibraryExW(filename, NULL,
		LOAD_WITH_ALTERED_SEARCH_PATH);
	struct csri_wrap_rend tmp;
	csri_rend *rend;
	struct csri_info *(*renderer_info)(csri_rend *rend);
	csri_rend *(*renderer_default)();
	csri_rend *(*renderer_next)(csri_rend *prev);
	const char *sym;

	if (!dlhandle) {
		//subhelp_log(CSRI_LOG_ERROR, "LoadLibraryEx(\"%ls\") failed: "
			//"%s", filename, get_errstr());
		wxLogStatus(_("LoadLibraryEx(\"%s, %s\") failed: "),filename, get_errstr());
		return;
	}
	if (GetProcAddress(dlhandle, "csri_library")) {
		//subhelp_log(CSRI_LOG_ERROR, "ignoring library %ls",
			//filename);
		wxLogStatus(_("ignoring library(\"%s, %s\")"),filename, get_errstr());
		goto out_freelib;
	}
	//subhelp_log(CSRI_LOG_INFO, "loading %ls", filename);

	tmp.os.dlhandle = dlhandle;

/* okay, this is uber-ugly. either I end up casting from void *
 * to a fptr (which yields a cast warning), or I do a *(void **)&tmp.x
 * (which yields a strict-aliasing warning).
 * casting via char* works because char* can alias anything.
 */
#define _dl_map_function(x, dst) do { \
	char *t1 = (char *)&dst; \
	union x { FARPROC ptr; } *ptr = (union x *)t1; \
	sym = "csri_" # x; \
	ptr->ptr = GetProcAddress(dlhandle, sym);\
	if (!ptr->ptr) goto out_dlfail; } while (0)
#define dl_map_function(x) _dl_map_function(x, tmp.x)
	dl_map_function(query_ext);
	//subhelp_logging_pass((struct csri_logging_ext *)
		//tmp.query_ext(NULL, CSRI_EXT_LOGGING));
	dl_map_function(open_file);
	dl_map_function(open_mem);
	dl_map_function(close);
	dl_map_function(request_fmt);
	dl_map_function(render);
#define dl_map_local(x) _dl_map_function(x, x)
	dl_map_local(renderer_info);
	dl_map_local(renderer_default);
	dl_map_local(renderer_next);

	rend = renderer_default();
	csrilib_add(rend, &tmp, renderer_info(rend));
	//while (rend) {
		//csrilib_add(rend, &tmp, renderer_info(rend));
		//rend = renderer_next(rend);
	//}
	return;

out_dlfail:
	wxLogStatus(_("%s symbol %s not found %s"),filename, sym, get_errstr());
out_freelib:
	FreeLibrary(dlhandle);
}

static void csrilib_load(const wchar_t *filename)
{
	DWORD attr = GetFileAttributesW(filename);
	if (attr == INVALID_FILE_ATTRIBUTES)
		return;

	if (attr & FILE_ATTRIBUTE_DIRECTORY) {
		csrilib_enum_dir(filename);
		return;
	}
	csrilib_do_load(filename);
}

static void csrilib_enum_dir(const wchar_t *dir)
{
	WIN32_FIND_DATAW data;
	HANDLE res;
	
	wchar_t buf[MAX_PATH];

	_snwprintf(buf, sizeof(buf) / sizeof(buf[0]), L"%ls\\*", dir);
	res = FindFirstFileW(buf, &data);
	if (res == INVALID_HANDLE_VALUE) {
		wxLogStatus(_("ignoring directory %s : %s"),dir, get_errstr());
		return;
	}

	//subhelp_log(CSRI_LOG_ERROR, "scanning directory \"%ls\"", dir);
	do {
		if (data.cFileName[0] == '.')
			continue;
		_snwprintf(buf, sizeof(buf) / sizeof(buf[0]),
			L"%ls\\%ls", dir, data.cFileName);
		
		csrilib_load(buf);
		//if(wraprends){break;}
	} while (FindNextFileW(res, &data));
	FindClose(res);
}

void csrilib_os_init()
{
	/*wchar_t filename[MAX_PATH], *slash;
	DWORD rv = GetModuleFileNameW(NULL, filename, MAX_PATH);
	if (!rv)
		*filename = L'\0';
	slash = wcsrchr(filename, L'\\');
	slash = slash ? slash + 1 : filename;
	*slash = L'\0';
	wcsncpy(slash, L"csri", filename + MAX_PATH - slash);
	csrilib_enum_dir(filename);*/
	csrilib_do_load(L"vsfilter_kainote.dll");
}

struct csri_wrap_rend *wraprends = NULL;

struct csri_wrap_rend *csrilib_rend_lookup(csri_rend *rend)
{
	struct csri_wrap_rend *wrap = wraprends;
	for (; wrap; wrap = wrap->next)
		if (wrap->rend == rend)
			return wrap;
	return NULL;
}

//list
#define INSTHASHSZ 256
#define HASH(x) (((intptr_t)(x) & 0xff00) >> 8)
static struct csri_wrap_inst *wrapinsts[INSTHASHSZ];

struct csri_wrap_inst *csrilib_inst_lookup(csri_inst *inst)
{
	struct csri_wrap_inst *ent = wrapinsts[HASH(inst)];
	while (ent && ent->inst != inst)
		ent = ent->next;
	return ent;
}

csri_inst *csrilib_inst_initadd(struct csri_wrap_rend *wrend,
	csri_inst *inst)
{
	struct csri_wrap_inst *winst = (struct csri_wrap_inst *)
		malloc(sizeof(struct csri_wrap_inst)),
		**pnext;
	
	if (!winst) {
		wrend->close(inst);
		return NULL;
	}
	winst->wrend = wrend;
	winst->inst = inst;
	winst->close = wrend->close;
	winst->request_fmt = wrend->request_fmt;
	winst->render = wrend->render;
	winst->next = NULL;
	pnext = &wrapinsts[HASH(inst)];
	while (*pnext)
		pnext = &(*pnext)->next;
	*pnext = winst;
	return inst;
}

void csrilib_inst_remove(struct csri_wrap_inst *winst)
{
	struct csri_wrap_inst **pnext = &wrapinsts[HASH(winst->inst)];
	while (*pnext && *pnext != winst)
		pnext = &(*pnext)->next;
	if (!*pnext)
		return;
	*pnext = (*pnext)->next;
	free(winst);
}

void csrilib_rend_initadd(struct csri_wrap_rend *wrend)
{
	wrend->next = wraprends;
	wraprends = wrend;
}

//static int initialized = 0;

csri_rend *csri_renderer_default()
{
	if (!wraprends) {
		csrilib_os_init();
		//initialized = 1;
	}
	if (!wraprends)
		return NULL;
	return wraprends->rend;
}

csri_rend *csri_renderer_next(csri_rend *prev)
{
	struct csri_wrap_rend *wrend = csrilib_rend_lookup(prev);
	if (!wrend || !wrend->next)
		return NULL;
	return wrend->next->rend;
}

csri_rend *csri_renderer_byname(const char *name, const char *specific)
{
	struct csri_wrap_rend *wrend;
	if (!wraprends) {
		csrilib_os_init();
		//initialized = 1;
	}
	if (!name)
		return NULL;
	for (wrend = wraprends; wrend; wrend = wrend->next) {
		if (strcmp(wrend->info->name, name))
			continue;
		if (specific && strcmp(wrend->info->specific, specific))
			continue;
		return wrend->rend;
	}
	return NULL;
}

csri_rend *csri_renderer_byext(unsigned n_ext, csri_ext_id *ext)
{
	struct csri_wrap_rend *wrend;
	unsigned i;
	if (!wraprends) {
		csrilib_os_init();
		//initialized = 1;
	}
	for (wrend = wraprends; wrend; wrend = wrend->next) {
		for (i = 0; i < n_ext; i++) {
			if (!wrend->query_ext(wrend->rend, ext[i]))
				break;
		}
		if (i == n_ext)
			return wrend->rend;
	}
	return NULL;
}

int *csri_close_renderer(csri_rend *renderer)
{
	std::unique_lock<std::mutex> lck (mtx);
	if(wraprends){free(wraprends);wraprends=NULL;}
	return 0;
}

csri_inst *csri_open_file(csri_rend *rend,
	const char *filename, struct csri_openflag *flags)
{
	std::unique_lock<std::mutex> lck (mtx);
	struct csri_wrap_rend *wrend = csrilib_rend_lookup(rend);
	if (!wrend)
		return NULL;
	return csrilib_inst_initadd(wrend,
		wrend->open_file(rend, filename, flags));
}

#define instance_wrapper(wrapname, funcname) \
csri_inst *wrapname(csri_rend *rend, \
	const void *data, size_t length, struct csri_openflag *flags) \
{ \
	std::unique_lock<std::mutex> lck (mtx);\
	struct csri_wrap_rend *wrend = csrilib_rend_lookup(rend); \
	if (!wrend) \
		return NULL; \
	return csrilib_inst_initadd(wrend, \
		wrend->funcname(rend, data, length, flags)); \
}

instance_wrapper(csri_open_mem, open_mem)
static instance_wrapper(wrap_init_stream_ass, init_stream_ass)
static instance_wrapper(wrap_init_stream_text, init_stream_text)

void *csri_query_ext(csri_rend *rend, csri_ext_id extname)
{
	struct csri_wrap_rend *wrend;
	void *rv;

	if (!rend)
		return NULL;

	wrend = csrilib_rend_lookup(rend);
	if (!wrend)
		return NULL;
	rv = wrend->query_ext(rend, extname);
	if (rv && !strcmp(extname, CSRI_EXT_STREAM_ASS)) {
		struct csri_stream_ext *e = (struct csri_stream_ext *)rv;
		memcpy(&wrend->stream_ass, e, sizeof(*e));
		wrend->init_stream_ass = e->init_stream;
		wrend->stream_ass.init_stream = wrap_init_stream_ass;
		return &wrend->stream_ass;
	}
	if (rv && !strcmp(extname, CSRI_EXT_STREAM_TEXT)) {
		struct csri_stream_ext *e = (struct csri_stream_ext *)rv;
		memcpy(&wrend->stream_text, e, sizeof(*e));
		wrend->init_stream_text = e->init_stream;
		wrend->stream_text.init_stream = wrap_init_stream_text;
		return &wrend->stream_text;
	}
	return rv;
}

struct csri_info *csri_renderer_info(csri_rend *rend)
{
	struct csri_wrap_rend *wrend = csrilib_rend_lookup(rend);
	if (!wrend)
		return NULL;
	return wrend->info;
}

void csri_close(csri_inst *inst)
{
	std::unique_lock<std::mutex> lck (mtx);
	struct csri_wrap_inst *winst = csrilib_inst_lookup(inst);
	if (!winst)
		return;
	winst->close(inst);
	csrilib_inst_remove(winst);
}

int csri_request_fmt(csri_inst *inst, const struct csri_fmt *fmt)
{
	std::unique_lock<std::mutex> lck (mtx);
	struct csri_wrap_inst *winst = csrilib_inst_lookup(inst);
	if (!winst || !inst)
		return 0;
	return winst->request_fmt(inst, fmt);
}

void csri_render(csri_inst *inst, struct csri_frame *frame,
	double time)
{
	std::unique_lock<std::mutex> lck (mtx);
	struct csri_wrap_inst *winst = csrilib_inst_lookup(inst);
	if (!winst || !inst)
		return;
	winst->render(inst, frame, time);
}

const char *csri_library()
{
	return "DEV";
}

