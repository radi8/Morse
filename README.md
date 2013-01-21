Morse
=====

Holger Schurig's morse programme

This is a place where I develop a bunch of re-usable Qt objects. Eventually I’ll use this for a morse trainer, but for the time being, I'm concentrate on the objects. So far I have:

MorseGenerator, non-GUI; converts “net” to “–. . –” and can this again convert to durations of sound and silence, according to WPM and factors for dits, dahs, inter-, character- and word-gaps.

AudioOutput, non-GUI: generate beeps of specified pitch and length using Qt’s MultiMedia QAudioOutput

SideScroller: shows morse code in a from-right-to-left scrolling widget
