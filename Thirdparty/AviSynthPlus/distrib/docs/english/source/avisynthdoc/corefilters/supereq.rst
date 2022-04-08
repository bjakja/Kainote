
SuperEQ
=======

``SuperEQ`` (clip, string filename)
``SuperEQ`` (clip, float band1 [, float band2, ..., float band18])

Shibatch Super Equalizer is a graphic and parametric equalizer plugin for
winamp. This plugin uses 16383th order FIR filter with FFT algorithm. Its
equalization is very precise. Equalization setting can be done for each
channel separately. SuperEQ is originally written by `Naoki Shibata`_.

SuperEQ requires a `foobar2k`_ equalizer setting file. The equalizer can be
found in foobar's DSPManager, and settings are adjused and saved from there
as well.

Some preset settings can be downloaded from `here`_.

In *v2.60*, a custom band setting (band1, band2, ...) is added to allow all
18 bands to be set within your script (instead of within your preset file).
The values should be specified in decibels (float).

This plugin is optimized for processors which have cache equal to or greater
than 128k bytes (16383*2*sizeof(float) = 128k). This plugin won't work
efficiently with K6 series processors (buy Athlon!!!).

+--------------+-----------------------------------------------------+
| Parameter    |                                                     |
+==============+=====================================================+
| filename     | The foobar2k equalizer preset file to apply.        |
+--------------+-----------------------------------------------------+
| band1-band18 | Allows to set all bands within your script (in dB). |
+--------------+-----------------------------------------------------+

**Example:**

Apply "Loudness" filter from the Equalizer Presets above:

::

    SuperEq("C:\Equalizer Presets\Loudness.feq")

+-----------+------------------------------------------------------------------------+
| Changelog |                                                                        |
+===========+========================================================================+
| v2.60     | Added custom band setting to allow all 16 bands to be set from script. |
+-----------+------------------------------------------------------------------------+
| v2.54     | Initial Release                                                        |
+-----------+------------------------------------------------------------------------+

Some parts of SuperEQ are:
Copyright © Naoki Shibata

Other parts are:
Copyright © 2001-2003, Peter Pawlowski
All rights reserved.

$Date: 2009/09/12 15:10:22 $

.. _ : #Tone
.. _Naoki Shibata: http://shibatch.sourceforge.net/
.. _foobar2k: http://www.foobar2000.org
.. _here: http://www.beingalink.de/files/Equalizer%20Presets.rar
