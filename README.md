Fceumm-PS2
==========

[![Build Status](https://travis-ci.org/AKuHAK/Fceumm-PS2.svg?branch=master)](https://travis-ci.org/AKuHAK/Fceumm-PS2)


*(ragnarok2040)
	I ported FCEUltra to PS2 to play a couple NES games on a TV in a different room for the holiday season. I have an autistic brother who loves Megaman, heh.
	The built-in game genie rom support works if you put the game genie rom as gg.rom. I haven't used any codes, so I'm not sure if they actually work on the PS2.

	Some notes about the source code:
	The original project source code that I ported is at sourceforge.net/projects/fceumm. It's a mapper modded version, which supports the most mappers of any FCEUltra version, I think. I tried to make as little change as possible, so using a diff utility won't be too much trouble to see the changes I made. From what I can remember:
 I had to remove support from gzip compressed nes roms because of a reference to dup() which wasn't supported and it was over my head on how to implement a similar function.

Thanks:
 -CaH4e3 for making a version of FCEUltra with extra mapper support.
 -DCGrendel for providing a space to host my homebrew and helping me with various logic errors I had.
 -The entire ps2dev/ps2-scene community for PS2SDK, gsKit, and various other projects I utilized when porting FCEUltra, especially dlanor for his help and providing uLaunchelf's code as an example from which to work.

FCEUmm-PS2
