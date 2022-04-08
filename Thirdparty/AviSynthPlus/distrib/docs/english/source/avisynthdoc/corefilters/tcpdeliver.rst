
TCPServer / TCPSource
=====================

| ``TCPServer`` (clip, int "port")
| ``TCPSource`` (string hostname, int "port", string "compression")

This filter will enable you to send clips over your network. You can connect
several clients to the same machine.

Syntax
------


Server
~~~~~~

``TCPServer`` (clip, int "port")

This will spawn a server thread on the current machine running on the
specified port. Port default is 22050.
You will get output in the application you open your script in, but the
server will only be running as long as the application (vdub for instance) is
open.

Example:

::

    Colorbars(512, 256)
    TCPServer()

will start a server.

Client
~~~~~~

``TCPSource`` (string hostname, int "port", string "compression")

This will connect to the machine with the given address (IP-number for
instance) to a server running on the given port. Port default is also 22050
here.

Compression enable you to choose the compression used for the video:

+------------------+----------------------------------------------------------------------------+
| Compression Type | Description                                                                |
+==================+============================================================================+
| None             | Use no compression. Fastest option - video will not be compressed before   |
|                  | being sent over the net.                                                   |
+------------------+----------------------------------------------------------------------------+
| LZO              | Use `LZO`_ dictionary compression. Fairly fast, but only compresses well   |
|                  | on artificial sources, like cartoons and anime with very uniform surfaces. |
+------------------+----------------------------------------------------------------------------+
| Huffman          | Uses a fairly slow Huffman routine by `Marcus Geelnard`_. Compresses       |
|                  | natural video better than LZO.                                             |
+------------------+----------------------------------------------------------------------------+
| GZip             | Uses a `Gzip`_ Huffman only compression. Works much like Huffman           |
|                  | setting, but seems faster.                                                 |
+------------------+----------------------------------------------------------------------------+

If no compression is given, GZip is currently used by default. Interlaced
material compresses worse than non-interlaced due to downwards deltaencoding.
If network speed is a problem you might want to use :doc:`SeparateFields <separatefields>`.

Example:

::

    TCPSource("127.0.0.1")
    Info()

This will connect to the local machine, if a server is running.

Examples
--------

You can use this to run each/some filters on different PC's. For example:
::

    #Clustermember 1:
    AVISource
    Deinterlacer
    TCPServer

    # Clustermember 2:
    TCPSource
    Sharpener
    TCPServer

    # Clustermember 3:
    TCPSource
    # client app -> video codec -> final file

Usability Notes
---------------

Once you have added a TCPServer, you cannot add more filters to the chain, or
use the output from the filter. The server runs in a separate thread, but
since AviSynth isn't completely thread-safe you cannot reliably run multiple
servers. This should **not** be used:

::

    AviSource("avi.avi")
    TCPServer(1001)
    TCPServer(1002) # This is NOT a good idea

So the basic rule is **never more than one TCPServer per script**.

Using commands after TCPServer is also a bad idea:

::

    AviSource("avi.avi")
    TCPServer(1001)
    AviSource("avi2.avi") # Do not do this, this will disable the server.

AviSynth detects that the output of TCPServer isn't used, so it kills the
Server filter. **TCPServer should always be the last filter.**


+-----------+-----------------+
| Changelog |                 |
+===========+=================+
| v2.55     | Initial Release |
+-----------+-----------------+

$Date: 2006/01/02 14:51:17 $

.. _LZO: http://www.oberhumer.com/opensource/lzo/
.. _Marcus Geelnard: http://bcl.sourceforge.net/
.. _Gzip: http://www.gzip.org/
