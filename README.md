# Minisynth

The goal is to create a simple midi synthesizer wich produces sinus
based sounds. It should at least support jack and pulse-audio.

## Features

* User defined envelope (Attack, Hold, Sustain, Release)
* User defined harmonics (up to 5)

### Basic functionality

* Interpret Note-On and Note-Off MIDI-Events. Respect the Velocity of
  Note-On events
* Mono output
* 128x polyphony

## Design goals

* Isolate sound system code (like creating a jack client) from
  synthesizer code, thus
  - Make it easy to add other sound systems (eg. portaudio)
  - Make it easy to change the synthesizer code
* Easyly understandable and readable code
