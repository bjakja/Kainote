
EnvInvoke
=========

::

  virtual AVSValue Invoke(const char* name, const AVSValue args, const char** arg_names=0);


You can use this to call a script function. There are many script functions
which can be useful from other filters; for example, the Bob filter uses
SeparateFields, and several source filters use UnalignedSplice. Some
functions, like Weave, are implemented entirely in terms of other functions.

If you're calling a function taking exactly one argument, you can simply pass
it in the args parameter; Invoke will convert it into an array for you. In
order to call a function taking multiple arguments, you will need to create
the array yourself; it can be done like this:
::

  AVSValue args[5] = { clip, 0, true, 4.7, "my hovercraft is full of eels" };
    env->Invoke("Frob", AVSValue(args, 5));

In this case Frob would need to have a parameter-type string like "cibfs" or
"cfbfs" or "cf.*".

The arg_names parameter can be used to specify named arguments. Named
arguments can also be given positionally, if you prefer.

Invoke throws ``IScriptEnvironment :: NotFound`` if it can't find a matching
function prototype. You should be prepared to catch this unless you know that
the function exists and will accept the given arguments.

A few examples (grabbed from MipSmooth - it uses invoke a lot):
::

  try {
          AVSValue args[1] = { child };
          PClip reduced =
          env->Invoke("Reduceby2",AVSValue(args,1)).AsClip();
      } catch (IScriptEnvironment::NotFound) {
          env->ThrowError("MyFilterError: Whoa! Could not Invoke
          reduce!");
      }


This piece of code calls *ReduceBy2(child)*, and places the result in
*reduced*. If you need the videoinfo (for whatever reason), you can get by
requesting:

::

      VideoInfo reduced_vi = reduced->GetVideoInfo();


Another example (using a resizer)

::

  try {
          upsizerString="LanczosResize";
          AVSValue up_args[3] = { child, 384, 288 };
          PClip resized = env->Invoke(upsizerString,
          AVSValue(up_args,3)).AsClip();
      } catch (IScriptEnvironment::NotFound) {
          env->ThrowError("MyFilterError: Whoa! Could not Invoke
          the resizer!");
      }


In general, avoid invoking "Invoke" as much as possible. If it can be
avoided, try not to do it every frame, but do it in the constructor, if the
filter parameters doesn't change. Invoking on every frame usually brings down
the speed of the filter to a fraction of a static filter, at it has to be
created and destroyed every frame.


Caching frames from Invoked filters
-----------------------------------

env->Invoke does **not** automatically insert a cache after the filter you
invoke. That means that each time you request a frame (using GetFrame) from
the invoked filter the filter will need to generate the frame. This is not a
problem if your filter only requests one unique frame per frame it generates.

If your filter however needs both the previous, the current and the next
frame to generate the current frame, the invoked filter will get called three
(3) times per frame you return.

It really only needs to be invoked to create the next frame (as the current
and previous frame can be reused).

To avoid this scenario, the internal cache filter has been exposed from
AviSynth 2.5.3, and is available to plugin writers.

To insert a invoke the cache, invoke it like this:

::

  // We are assuming your invoked filter to be placed in "PClip resized" as above.
      try {
          AVSValue up_args[1] = { resized };
          resized = env->Invoke("InternalCache",
          AVSValue(up_args,1)).AsClip();
      } catch (IScriptEnvironment::NotFound) {
         // If the AviSynth version is too low, and the internal
         cache isn't exposed, we just ignore this situation.
      }


You can also set cache hints. In the scenario above, where we need one frame
prior and one frame after the current one, use:

::

        resized->SetCacheHints(CACHE_RANGE,1);


Update: since AviSynth 2.5.6 the cache range parameter is considered as a
'diameter'=1+radius*2. It equal to 3 in the scenario above (previous, current
and next frame), so use:

::

        resized->SetCacheHints(CACHE_RANGE,3);

----

Back to :doc:`FilterSDK`

$Date: 2014/10/27 22:04:54 $
