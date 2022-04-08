
Greyscale
=========

``Greyscale`` (clip [, string "matrix"])

Converts the input clip to greyscale (without changing the color format).

In YCbCr based formats, the chroma channels are set to 128. In RGB based
formats, the conversion produces the luma using the coefficients given in the
matrix parameter (rec601 by default, which reflects the behaviour in old
AviSynth versions). This option is added in *v2.56*. When setting
matrix="rec709", the clip is converted to greyscale using Rec.709
coefficients. When setting matrix="Average" the luma is calculated as
(R+G+B)/3. See :doc:`ColorConversions <convert>` for an explanation of these coefficients.

$Date: 2008/02/10 13:57:17 $
