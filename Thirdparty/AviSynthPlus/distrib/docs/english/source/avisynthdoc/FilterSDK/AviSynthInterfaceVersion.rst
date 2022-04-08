
AviSynth Interface Version
==========================

The AVISYNTH_INTERFACE_VERSION describes the level of features
available, both in the core avisynth.dll and the third party plugin.

For a plugin author it describes what the core IScriptEnvironment
vtable contains and the what behaviour the core expects of their
plugin.

- Version 1 is Avisynth 2.0
- Version 2 is Avisynth 2.5, with the vtable having members up to
  IScriptEnvironment::SetWorkingDir(const char * newdir)
- Version 3 is Avisynth 2.5.6, with the IScriptEnvironment vtable
  adding 3 new members ManageCache, PlanarChromaAlignment and
  SubframePlanar.
- Version 4 is reserved and does not apply to any released Avisynth
  version. It's only significance is it greater then 3 and less then
  6. No public core provides this version, plugins that report this
  version are treated as if they reported version 3.
- Version 5 is the alpha development releases of Avisynth 2.6.0a1-a5,
  with the IScriptEnvironment vtable adding 3 more new members
  DeleteScriptEnvironment, ApplyMessage and GetAVSLinkage. Also with
  version 5 some core versions provides AVS_Linkage support for baked
  code replacement. The IClip::SetCacheHints member was changed to
  return an int. Some core versions demand this behaviour, but early
  adopter plugins may not conform. This has caused some confusion.
- Version 6 is the production releases of Avisynth 2.6.0, with the
  IScriptEnvironment vtable adding one more new member GetVarDef. It
  also formally uses type size_t in place of type int for things that
  are memory sizes, ready for a 64 bit port. Version 6 relaxes the
  requirement for IClip::SetCacheHints of version 5 plugins to return
  an int. All version 6 and later plugins are expected to honour this
  requirement.

When using the raw IClip interface it is the authors responsibility to
declare the level of support the plugin provides. The
GenericVideoFilter class provides this:
::

    int __stdcall UserClass::GetVersion() { return AVISYNTH_INTERFACE_VERSION; }


- Version 1 is Avisynth 2.0
- Version 2 and 3 are Avisynth 2.5, supporting YV12, YUY2, RGB32 and
  RGB24 colour spaces.
- Version 4 is reserved and does not apply to any Avisynth version.
  It's significance is it greater then 3 and less then 6.
- Version 5 is for early adopter alpha release of Avisynth 2.6.
- Version 6 is Avisynth 2.6, and the IClip interface must support
  this update:

::

    /* Plugins that do not implement the interface must always return zero. */
    virtual int __stdcall IClip::SetCacheHints(int cachehints,int frame_range);

____

Back to :doc:`FilterSDK`

$Date: 2015/01/13 00:24:50 $
