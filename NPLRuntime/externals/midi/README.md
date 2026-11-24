The MIDI renderer uses the OPL3 emulator and built-in instrument bank from
[dos-like, revision 3bebf4c](https://github.com/mattiasgustavsson/dos-like/blob/3bebf4cbbe02ee3a67ddc8eb76b8cdb9c1dd2ee6/source/libs/opl.h).
The local copy uses a fixed 18-voice chip, remembers channel pitch bend, checks
note indices, preserves pitch during note release, supports an allocation-free
song reset, and tunes the frequency table for 48 kHz output at A4 = 440 Hz.
The BSD license notices are at the end of `opl.h`.

[sokol_audio.h, revision 754d326](https://github.com/floooh/sokol/blob/754d32619fef272efeeaaa5ba629b68ba9f3c74e/sokol_audio.h)
supplies the audio callback on Apple, Android, Windows and WebAssembly. This
revision includes the OpenSL ES backend used at the Android client's minimum
SDK level. A local option lets the iOS host retain its recording session category and
control session deactivation. The zlib/libpng license is included in the header.
