---------------------------------------------------------------------------
File name:      ELF_Loader_Readme.txt           Revision date:  2006.04.02
Created by:     Ronald Andersson (AKA: dlanor)  Creation Date:  2006.03.28
---------------------------------------------------------------------------
ELF_Loader is a code module intended to make it easier for PS2 programs
to implement a 'Back to browser' feature, that will work consitently with
existing file managers like LaunchELF. Some programs already have such a
feature, but very often their ELF loaders are limited in various ways so
that they frequently fail to launch the intended ELFs properly.
(eg: SMS, PS2MP3, etc) That is why this package was developed, which uses
an ELF launcher closely related to that of LaunchELF, so that it can be
relied upon to consistently launch ELFs in a proper manner.

By necessity an ELF launcher is split into two parts. One part needs to be
linked in as part of the main program that needs this feature. This is the
file Reboot_ELF.c of this package. But that file needs to contain a binary
image of the real ELF_Loader, which must be separately compiled.

Of course, that "must be" only applies if you make modifications.
The "Reboot_ELF.c" supplied here is already fully prepared for use.

If you make any modifications to ELF_Loader.c, then you need to 'Make all'
for the "ELF_Loader_Makefile". This will first compile "ELF_Loader.elf"
and then convert that to source code holding the binary image as a char
array declared in the file "ELF_Loader.h". Next you need to combine this
with the source in "Reboot_ELF_raw.c". You can do this either by merging
contents of "ELF_Loader.h" into the header section of "Reboot_ELF_raw.c",
or by simply adding a '#include' statement to the same effect, and then
saving it as "Reboot_ELF.c". I personally prefer the merging method, as
it results in a single file that can be added to projects that need this
'Back to browser' feature. So that's how I've done it in this release.

Finally you just add "Reboot_ELF.c" to the other source files in your
project, so that it will be compiled and linked in with the rest of
your modules. Any of those modules can then relaunch your 'Browser' ELF
(or others too), simply by making a simple function call like this:
  RunLoaderElf(Elf_path, Elf_part);

Both arguments to that call are strings, and for most cases "Elf_part"
should be an empty string, while "Elf_path" should be a full path string
for the ELF to be launched. The only exceptiuon to this is if you want
to launch an ELF from HDD, in which case "Elf_part" must specify the
partition, while "Elf_path" is the full path for pfs0. Note that this
release is NOT really intended to handle HDD, so you must be careful if
you insist on doing it. HDD drivers must then be resident already before
you make the call, and pfs0 must be free of previous mounts, as must the
Elf_part partition too, of course.
---------------------------------------------------------------------------
End of file:    ELF_Loader_Readme.txt
---------------------------------------------------------------------------
