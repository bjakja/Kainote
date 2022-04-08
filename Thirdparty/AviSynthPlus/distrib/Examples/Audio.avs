# Generates colorbar image at the size 320x240, Adds a 440Hz tone that pulses in the right speaker

Colorbars(320,240)

# Selects the right channel

GetRightChannel()

# Amplifies the signal down 3DB, and puts the result into the variable 'signal'

signal = AmplifyDB(-3)

# Puts the new signal as right channel, leaving the sound from GetRightChannel as new left channel.

MonoToStereo(signal)

# Left channel now contains a pulsing signal (from ColorBars).
# Right channel now contains the same signal at -3dB.