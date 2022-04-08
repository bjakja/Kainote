
EnvSaveString
=============

**env->SaveString** is given to allow users to pass strings to AVSValue, and
ensure that they are being deallocated on unload.

An Example:

::

    fnpluginnew = new char[string_len];
    strcpy(fnpluginnew, fnplugin.AsString());
    strcat(fnpluginnew, " ");
    strcat(fnpluginnew, name);

    env->SetGlobalVar("$PluginFunctions$", AVSValue(env->SaveString(fnpluginnew, string_len)));

    // Since fnpluginnew has now been saved it can safely be deleted.
    delete[] fnpluginnew;


But you must still remember that these strings are not deallocated until the
filter is unloaded.

So in general if you don't want your filter to be allocating more memory on
each load, try avoiding using non-const strings.

----

Back to :doc:`FilterSDK`

$Date: 2006/11/24 18:21:26 $
