
Import
======

``Import`` (string filename [, ...])

``Import`` evaluates the contents of other AviSynth scripts.

Functions, variables and loaded plugins declared inside the imported script
are made available to the importing script. In addition, the return value of
the function is the last value of the last filename script (a clip or
whatever the filename script chooses to end with). The later can be assigned
to a variable of the importing script and manipulated (this is most useful
when the imported script ends with a clip).

The current working directory (CWD) is saved and set to the directory
containing the script file before compiling the script. The CWD is then
restored to the saved directory.

Some indicative uses of ``Import`` include:

* Storing multiple script-functions, variables and global variables for reuse
  by scripts (creation of script libraries).
* Retrieving pre-built streams.
* Retrieving dynamically configured pre-built streams (the core idea is: the
  importing script declares some global variables which the imported script
  uses to configure the stream that will return).

$Date: 2009/09/12 15:10:22 $
