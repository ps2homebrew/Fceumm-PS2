FCEUmm-PS2
===========

[Download](https://github.com/infval/Fceumm-PS2/releases) binary release (ELF file).

AKuHAK's [release notes](http://psx-scene.com/forums/f176/fceu-mm-ps2-new-version-118166/).
## Extra features
* Multitap support up to 4 players
* Separate turbo buttons
## Controls
### In-game

Action | Button | Action | Button | Action | Button
------ | :----: | ------ | :----: | ------ | :----:
B | ![Square](https://user-images.githubusercontent.com/38145742/38648065-120deeb8-3df8-11e8-984b-cccab8dd4622.png) | Select | ![Select](https://user-images.githubusercontent.com/38145742/38648134-65d593ca-3df8-11e8-9926-44357e5c44cd.png) | Start | ![Start](https://user-images.githubusercontent.com/38145742/38648145-717997d0-3df8-11e8-99b0-f98a75fc682b.png)
A | ![Cross](https://user-images.githubusercontent.com/38145742/38648019-e30fd27a-3df7-11e8-8e6b-660cdf65b9f6.png) | Menu | ![L1](https://user-images.githubusercontent.com/38145742/38646430-9b548400-3df0-11e8-9158-e6d77afd2115.png) | `---` | ![R1](https://user-images.githubusercontent.com/38145742/38648091-2a2bbe76-3df8-11e8-9894-c53092ee8557.png)
Turbo B | ![Triangle](https://user-images.githubusercontent.com/38145742/38646514-f0d0f2ec-3df0-11e8-9ff6-968a9f43ba9b.png) | Load State | ![L2](https://user-images.githubusercontent.com/38145742/38648101-3684328e-3df8-11e8-83b6-a17a6bb076a9.png) | Save State | ![R2](https://user-images.githubusercontent.com/38145742/38648109-4039008e-3df8-11e8-9171-1b6bacb38091.png)
Turbo A | ![Circle](https://user-images.githubusercontent.com/38145742/38646507-eef3b536-3df0-11e8-8057-c4f8dd361eba.png) | FDS Disk Swap<br/>VS. Insert Coin | ![L3](https://user-images.githubusercontent.com/38145742/38648117-4e97d3d0-3df8-11e8-9278-bc95530fad35.png) | FDS Side Swap | ![R3](https://user-images.githubusercontent.com/38145742/38648122-5b92778e-3df8-11e8-82ea-2eadfcd8764a.png)

D-pad: D-pad or Analog stick
### Browser

Action | Button | Action | Button | Action | Button
------ | :----: | ------ | :----: | ------ | :----:
`---` | ![Square](https://user-images.githubusercontent.com/38145742/38648065-120deeb8-3df8-11e8-984b-cccab8dd4622.png) | Options | ![Select](https://user-images.githubusercontent.com/38145742/38648134-65d593ca-3df8-11e8-9926-44357e5c44cd.png) | Confirm save path,<br/>center screen | ![Start](https://user-images.githubusercontent.com/38145742/38648145-717997d0-3df8-11e8-99b0-f98a75fc682b.png)
`---` | ![Cross](https://user-images.githubusercontent.com/38145742/38648019-e30fd27a-3df7-11e8-8e6b-660cdf65b9f6.png) | Menu | ![L1](https://user-images.githubusercontent.com/38145742/38646430-9b548400-3df0-11e8-9158-e6d77afd2115.png) | `---` | ![R1](https://user-images.githubusercontent.com/38145742/38648091-2a2bbe76-3df8-11e8-9894-c53092ee8557.png)
Back | ![Triangle](https://user-images.githubusercontent.com/38145742/38646514-f0d0f2ec-3df0-11e8-9ff6-968a9f43ba9b.png) | `---` | ![L2](https://user-images.githubusercontent.com/38145742/38648101-3684328e-3df8-11e8-83b6-a17a6bb076a9.png) | `---` | ![R2](https://user-images.githubusercontent.com/38145742/38648109-4039008e-3df8-11e8-9171-1b6bacb38091.png)
OK | ![Circle](https://user-images.githubusercontent.com/38145742/38646507-eef3b536-3df0-11e8-8057-c4f8dd361eba.png) | `---` | ![L3](https://user-images.githubusercontent.com/38145742/38648117-4e97d3d0-3df8-11e8-9278-bc95530fad35.png) | `---` | ![R3](https://user-images.githubusercontent.com/38145742/38648122-5b92778e-3df8-11e8-82ea-2eadfcd8764a.png)

D-pad: D-pad
## Dependencies
* https://github.com/ps2dev/ps2sdk (use [ps2toolchain](https://github.com/ps2dev/ps2toolchain))
* https://github.com/ps2dev/gsKit
* https://github.com/ps2dev/ps2sdk-ports (libjpeg)
* https://github.com/ps2dev/ps2-packer (optinal)

## Historical note

*(ragnarok2040)

I ported FCEUltra to PS2 to play a couple NES games on a TV in a different room for the holiday season. I have an autistic brother who loves Megaman, heh.

The built-in game genie rom support works if you put the game genie rom as gg.rom. I haven't used any codes, so I'm not sure if they actually work on the PS2.

Some notes about the source code:

The original project source code that I ported is at sourceforge.net/projects/fceumm. It's a mapper modded version, which supports the most mappers of any FCEUltra version, I think. I tried to make as little change as possible, so using a diff utility won't be too much trouble to see the changes I made. From what I can remember:

I had to remove support from gzip compressed nes roms because of a reference to dup() which wasn't supported and it was over my head on how to implement a similar function.

Thanks:
- CaH4e3 for making a version of FCEUltra with extra mapper support.
- DCGrendel for providing a space to host my homebrew and helping me with various logic errors I had.
- The entire ps2dev/ps2-scene community for PS2SDK, gsKit, and various other projects I utilized when porting FCEUltra, especially dlanor for his help and providing uLaunchelf's code as an example from which to work.

FCEUmm-PS2
