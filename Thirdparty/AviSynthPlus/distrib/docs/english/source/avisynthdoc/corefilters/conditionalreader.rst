
ConditionalReader
=================

``ConditionalReader`` (clip, string filename, string variablename, bool
"show")

``ConditionalReader`` allows you to import information from a text file, with
different values for each frame - or a range of frames.

Parameters
----------

+--------------+----------------------------------------------+--------------+
| Parameter    | Description                                  | Default      |
+==============+==============================================+==============+
| clip         | The control clip. It is not touched, unless  | Not optional |
|              | you specify show=true.                       |              |
+--------------+----------------------------------------------+--------------+
| filename     | The file with the variables you want to set. | Not optional |
+--------------+----------------------------------------------+--------------+
| variablename | The name of the variable you want the        | Not optional |
|              | information inserted into.                   |              |
+--------------+----------------------------------------------+--------------+
| show         | When set to true, the value given at this    | false        |
|              | frame will be overlayed on the image.        |              |
+--------------+----------------------------------------------+--------------+

File format
-----------

The file is plain text. All separation is done by spaces, and newline
indicates a new data set. It is not case sensitive!

**TYPE (int|float|bool|string)**

You can only have one type of data in each file. Currently it is possible to
have *float*, *int*, *bool* or (from v2.60) *string* values. You specify this
by using the **TYPE** keyword. You should always start out by specifying the
type of data, as nothing is saved until this keyword has been found. It is
not possible to change type once it has been set!

**DEFAULT <value>**

This specifies the default value of all frames. You should do this right
after specifying the type, as it overwrites all defined frames. You can omit
this setting, you must be sure to specify a setting for all frames, as it
will lead to unexpected results otherwise.

**OFFSET <value> (from v2.60)**

When specified, this will offset all framenumbers by a constant offset. This
is done for all framenumbers which are set after this keyword.

**<framenumber> <value>**

This will set the value only for frame <framenumber>.

**R <startframe> <endframe> <value>**

This will apply a value to a range of frames. You should note that both
start-frame AND end-frame are included.

**I <startframe> <endframe> <startvalue> <stopvalue>**

This will interpolate between two values over a range of frames. This only
works on int and float values. You should note that both start-frame AND end-
frame are included.

Types
-----

As mentioned, the types can be either *float*, *int*, *bool* or (from v2.60)
*string*.

*Int* numbers is a number optionally preceeded with a sign.

*Float* is a decimal number containing a decimal point, optionally preceded
by a sign and optionally followed by the e or E character and a decimal
number. Valid entries are -732.103 or 7.12e4.

*Bool* can either be *true*, *T*, *yes*, *false*, *F* or *no*.

*String* is a sequence of characters representing text.

Examples
--------


Basic usage
~~~~~~~~~~~

File.txt:
::

    Type float
    Default 3.45567

    R 45 300 76.5654
    2 -671.454
    72 -671.454

The file above will return float values. It will by default return 3.45567.
However frames 45 to 300 it will return 76.5654. And frame 2 and 72 will
return -671.454.

As you might notice - later changes overrule settings done earlier in the
file. This is illustrated by frame '72' - even though it is inside the range
of 45-300, the later value will be returned. On the other hand - if the range
was specified AFTER '72 -671.454' - it would return 76.5654.

A script to invoke this file could be:
::

    colorbars(512,512)
    trim(0,500)
    ScriptClip("subtitle(string(myvar))")
    ConditionalReader("file.txt", "myvar", false)

This will put the values into the variable called "myvar", which is used by
:doc:`Subtitle <subtitle>`, invoked by :ref:`ScriptClip` to display the conditional value.

**Note!** The ConditionalReader() line comes **after** any use of "myvar" in
your script.


Adjusting Overlay
~~~~~~~~~~~~~~~~~

**AviSynth script:**
::

    colorbars(512,256)
    a1 = trim(0,600)
    a2 = MessageClip("Text clip")
    overlay(a1,a2, y = 100, x = 110, mode="subtract", opacity=0, pc_range=true)
    ConditionalReader("opacity.txt", "ol_opacity_offset", false)
    ConditionalReader("xoffset.txt", "ol_x_offset", false)

**xoffset.txt:**
::

    Type int
    Default -50

    I 25 50 -50 100
    R 50 250 100
    I 250 275 100 250

**opacity.txt:**
::

    Type float
    Default 0.0

    I 25 50 0.0 1.0
    R 50 250 1.0
    I 250 275 1.0 0.0

Basically it defines keyframes for an x-offset and the opacity. Frame 25->50
the opacity is scaled from 0.0 to 1.0, while the text is moving from left to
right. The text is then kept steady from frame 50 to 250, whereafter it moves
further to the right, while fading out.

It is easier to watch the clip above than completely describe what it does.


.. _complicated-applyrange:

Complicated ApplyRange
~~~~~~~~~~~~~~~~~~~~~~

As you may have noticed using a large number of :doc:`ApplyRange() <animate>` calls in a
script can lead to resource issue. Using ``ConditionalReader`` together with
:doc:`ConditionalFilter <conditionalfilter>` can lead to an efficient solution:

File.txt:
::

    Type Bool
    Default False

    2 True
    R 45 60 True
    72 True
    R 200 220 True
    210 False
    315 True

The file above will return boolean values. It will by default return False.
However frames 2, 45 to 60, 72, 200 to 220 and 315 except for 210 it will
return True. As you might notice, later changes overrule settings done
earlier in the file. This is illustrated by frame '210' - even though it is
inside the range of 200-220, the later value, False, will be returned.

A script to make use of this file could be:
::

    colorbars(512,512)
    trim(0,500)
    A=Last
    FlipHorizontal() # Add a complex filter chain
    B=Last
    ConditionalFilter(A, B, "MyVar", "==", "False", false)
    ConditionalReader("File.txt", "MyVar", false)

This will put the values into the variable called "MyVar", which is used by
:doc:`ConditionalFilter <conditionalfilter>` to select between the unprocessed and flipped version of
the source.

**Note!** The ``ConditionalReader()`` line comes **after** any use of "MyVar"
in your script.


Returning Strings
-----------------

``ConditionalReader`` cannot return strings prior to v2.60, but one solution
is to create a list of variables with corresponding string assignments, and
eval the indexed solution. For example:

::

    import("strings.txt")
    ScriptClip("""subtitle(Eval("n"+string(mystringindex)))""")
    ConditionalReader("range_string.txt", "mystringindex")

range_string.txt

::

    Type int
    Default 0

    R 10 1000 1
    R 1005 3000 2
    R 3200 3800 3

strings.txt

::

    n0=""
    n1="Intro"
    n2="Main"
    n3="Credits"

Obviously strings.txt does not need to be a separate file, but this solution
is sometimes appropriate in some multilingual applications, e.g.

::

    language="spanish"
    import(language + "_strings.txt")

+------------+----------------------------------+
| Changelog: |                                  |
+============+==================================+
| v2.60      | Added OFFSET, Added Type=string. |
+------------+----------------------------------+

$Date: 2011/04/29 20:09:50 $
