# Generates colorbar image at the size 512x384

Colorbars(512,384)

# Resizes the image using the lanczos3 algorithm

LanczosResize(640,480)

# Now the image is 640x480
# Now crop 16 pixels off the entire image all the way around.

Crop(16, 16, -16, -16)

# Convert to greyscale

Greyscale()

