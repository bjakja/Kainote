// CSRI Renderer
#include <string.h>

typedef void csri_inst;
typedef void csri_rend;

struct videoframe
{
    int width;
    int height;
    BYTE* data;
};

class kCSRI
{
private:
    csri_inst*	kInstance;
	csri_rend *vobsub;
public:
    kCSRI();
	~kCSRI();
    int Open(std::wstring wstr); // text == ass file
    int Draw(videoframe& dst);
    void Close();
};