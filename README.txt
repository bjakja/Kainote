To build You need install:

DirectX SDK
https://www.microsoft.com/en-us/download/details.aspx?id=6812

Windows SDK for windows7
https://www.microsoft.com/en-us/download/details.aspx?id=3138

Put into folder Thirdparty:

wxWidgets 
http://wxwidgets.org/

Boost
http://www.boost.org/

Icu
http://site.icu-project.org/

Check in project properties if paths of installed windows SDK's are correct
or change it to yours paths.

Before open program solution, build wxWidgets, Hunspell, BaseClasses, Freetype, xy-Vsfilter

Number of build increases automatically after release build.
It will be greater of one than in exe.
Version is stored in versionKainote.h.