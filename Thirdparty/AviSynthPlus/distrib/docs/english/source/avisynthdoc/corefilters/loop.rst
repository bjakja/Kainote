
Loop
====

``Loop`` (clip, int "times", int "start", int "end")

Loops the segment from frame start to frame end a given number of times.
times (default -1) is the number of times the loop is applied.
start (default 0) the frame of the clip where the loop starts.
end (default framecount(clip)) the frame of the clip where the loop ends.

::

    # Loops frame 100 to 110 of the current clip 10 times
    Loop(10,100,110)


    Loop() # make the clip loop (almost) endlessly


    Loop(10) # make the clip loop ten times


    Loop(10,20,29) # repeat frames 20 to 29 ten times before going on


    # actual clip duration increased by 90 frames
    Loop(0,20,29) # delete frames 20 to 29


    # actual clip duration decreased by 10 frames
    Loop(-1,20,29) # frame 20 to 29 is repeated (almost) infinite times


$Date: 2004/03/07 22:44:06 $
