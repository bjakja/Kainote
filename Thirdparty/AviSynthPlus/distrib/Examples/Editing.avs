# Generates colorbar image at the size 320x240

Colorbars(320, 240)

# Converts the image to greyscale

Greyscale()

# Adds framenumbers to the image

Showframenumber()

# Selects the range 0 to 50000

Trim (0, 50000)

# Selects every second frame. Fps is now half of 29.97

SelectEven()

# Selects first 1000 frames inside the stream, and assigns it to variable substream

substream = Trim (0, -1000)

# Loop the substream 25 times

substream = Loop(substream, 25)

# Put the substream on bottom of the original

Stackvertical(substream)

