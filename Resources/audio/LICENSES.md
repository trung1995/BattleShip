# Audio assets

All files in this folder are placeholder sounds generated for this project with FFmpeg's
built-in signal sources (`sine`, `anoisesrc`) and filters; no third-party recordings are used.
They are dedicated to the public domain (CC0 1.0).

MP3 is used because it is the one format both cocos2d-x 4.0 audio backends decode: the macOS
backend (ExtAudioFile) does not read Ogg Vorbis, and the Windows backend reads only Ogg and MP3.

| File | Used for |
|---|---|
| shot.mp3 | a shot is fired |
| miss.mp3 | miss |
| hit.mp3 | hit |
| sunk.mp3 | ship sunk |
| help.mp3 | Help ability |
| victory.mp3 / defeat.mp3 | game over |
| click.mp3 | UI click |
| menu.mp3 / battle.mp3 | looping music |
