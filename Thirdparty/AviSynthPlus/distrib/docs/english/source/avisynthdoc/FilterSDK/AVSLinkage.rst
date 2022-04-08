
AVS Linkage
===========

Removal of backed code
----------------------

The plugin api v3 contains baked code (see avisynth.h from v2.58 and
older versions) which is code that is included inline. The result is
all that code is "baked" into all and every plugin instead being called
from avisynth.dll. The problem with baked code is that it cannot be
changed without recompiling the plugin. (If it were just a declaration
only then the active code would be whatever is in the current
avisynth.dll.) This is a problem if the plugin api will be changed
again in the future (and it probably will one day) and this issue is
solved in 2.6.

Starting from 2.6 the version 2.5 plugins are supported directly (with
current baked code; meaning that plugins compiled for 2.5 can be loaded
when using 2.6) and all the baked code from 2.6+ plugins is removed and
the plugins are still source compatible. Note that the baked code is
moved to interface.cpp, where also the structure AVS_Linkage is
defined.

AVS_Linkage is a mechanism to initialize the function table with the
entry points for an alternate server. Server here means something that
loads and executes an avisynth.h api plugin. This is normally
avisynth.dll but Avery Lee has expressed a desire to be able to load
and execute avisynth plugins directly in VirtualDub. Much the same way
Avisynth can currently load VirtualDub plugins today.

The implementation solution
---------------------------

On the user plugin side the entry-point is now called
AvisynthPluginInit3 and it take 2 arguments, an IScriptEnvironment* and
an AVS_Linkage*. The user needs to declare static storage for
AVS_Linkage* AVS_linkage; and in their AvisynthPluginInit3 code save
the supplied 2nd value to the AVS_linkage variable. The rest of the
code should be the same as for 2.5 plugins, i.e. env->AddFunction, etc.

In avisynth.h the type AVS_Linkage structure is declared and macros
emit stub baked code that calls the appropriate code in the server via
the AVS_Linkage function pointers. The stub code checks to make sure
AVS_linkage is not Null and the offset of the function pointer is
within the current size of the structure. If okay the function is
called otherwise it evaluates as 0. Within the Avisynth.dll project
these macros resolve to a simple ";" so no stub code is emitted just
declarations occur, this is triggered by the definition of the reserved
symbol AVISYNTH_CORE.

In interface.cpp the real code is defined and the instance of the
AVS_Linkage structure is declared and initialised. In plugins.cpp the
address of this is passed to the users AvisynthPluginInit3 code along
with the ScriptEnvironment address. A minor hurdle was that you may not
take the address of constructor or destructor routines in C++ so I to
move forward I just nested the appropriate code for these down one
method and used the address of the nested method. I unimaginatively
called these CONSTRUCTORn() and DESTRUCTOR() [edit: shouldn't this be
DESTRUCTORn()???].

In your plugin source file the changes are trivial. Declare the storage
for AVS_linkage, rename the .dll entrypoint from AvisynthPluginInit2 to
AvisynthPluginInit3 and add the extra AVS_Linkage* argument.

`discussion on Doom9 <http://forum.doom9.org/showthread.php?t=101730>`_ (in
particular
`this post <http://forum.doom9.org/showthread.php?p=1567792#post1567792>`__ and
`this post <http://forum.doom9.org/showthread.php?p=1631250#post1631250>`__).

____

Back to :doc:`FilterSDK`

$Date: 2014/10/27 22:04:54 $
