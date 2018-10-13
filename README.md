# zx81putil

zx81putil is a commandline utility aided to perform some operations with files that contain software
for the Sinclair ZX81 computer.

These operations include:
- Read a WAV sound file containing a record of a ZX81 loading sound and convert it to a .P memory 
  file. This is the default operation.

- Read a .P file and convert it to a WAV file ready to be played back to the ZX81.

- Read a .P file and convert it to a TZX tape image using 1.20 specification, ready to be played back
  to the ZX81.

- Play a .P file thru the computer speaker.

Besides, there are options to show the video buffer contents or the BASIC listing.

The format and available options are as follows:

zx81putil [options] input_filename
  OPTIONS:
   -o outputfile  : overrides default file name for output.
   -w2p           : converts a WAV file into P file (default option)
   -p2w           : converts a P file into a WAV file
   -tzx           : converts a P file into a TZX file
   -play          : plays a P file thru the computer audio system
   -scr           : shows the screen buffer contents as saved into the file
   -bas           : shows the BASIC listing as saved into the file
   input_filename : WAV or P file to convert/play (default: WAV file is assumed)


The SDL.dll is included with the zx81putil files, so you won't need to download it as well.

Notes:

- The input WAV file must be an uncompressed PCM file, 44100Hz sampling frequency and 16 bits. It may
  be stereo or mono.

- If a WAV to P conversion ends with no errors (check the summarized information displayed during
  the conversion process) but no .P file is written, then specify the .P filename yourself with
  the -o option. This may happen because the default output filename is taken from the ZX81 filename
  saved in the recording. If such filename contains invalid characters for the target filesystem (such
  as * or ?), no file is outputted.

- If a WAV to P conversion aborts with a read error, a time tag will be printed so you kown where the
  error happened. It might be because a dropout in the audio signal, or a misreading because of a
  weak signal. In any case, the time tag allows you to load the sound file in a sound editor, go to
  the time specified in the tag and examine/fix whatever is happening at that position.
  The time tag is formatted as [min:secs] with secs being rounded up to the milisecond.

- For the sake of readness, the BASIC listing ensures that REM lines that appears to contain machine
  code are shown as sequences of hexadecimal bytes enclosed between brackets. However, if there's a
  carriage return code (0x76 in ZX81) in the middle of the REM sentence, that will be interpreted as
  the end of the REM sentence, and the listing will revert to normal token display mode.

- Whenever ZX81 characters need to be displayed (BASIC listing and screen dump), non-ASCII characters
  such as graphic blocks are displayed as '?'

- The TZX file generated confirms to v1.20 specification. More precisely, the generated file uses ID
  0x19 . It's possible that a TZX player cannot recognize this ID.
