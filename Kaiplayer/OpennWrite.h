#ifndef OPENNWRITE_H_INCLUDED
#define OPENNWRITE_H_INCLUDED

#include <wx/file.h>
#include <wx/thread.h>

class OpenWrite
{
    public:
    OpenWrite();
    OpenWrite(wxString filename, bool clear=true);
    ~OpenWrite();
    void CloseFile();
    wxString FileOpen(wxString filename,bool test=true);
    void FileWrite(wxString filename, wxString alltext, bool utf=true);
    void PartFileWrite(wxString parttext);
	bool IsUTF8withoutBOM(const char* buf, size_t size);
	wxFile file;
    private:

    bool isfirst;
};

#endif // OPENNWRITE_H_INCLUDED
