#include <Arduino.h>
#include <MIDI.h>

#include "midi_looper.h"

byte midi_in_message_type;
byte midi_in_channel;
byte midi_in_d1;
byte midi_in_d2;

int LOOP_SWITCH_PIN = 24;
bool isLooping = false;

MIDI_CREATE_INSTANCE(HardwareSerial, Serial1, MIDI);

void setup() {
  MIDI.begin(15);
  MIDI.turnThruOff();

  Serial.begin(57600);
  Serial.println("MIDI Input Test");

  pinMode(LOOP_SWITCH_PIN, INPUT);

  initializeMidiLooper();
}

void midiRead() {
  if (MIDI.read()) {
    midi_in_message_type = MIDI.getType();
    switch (midi_in_message_type) {
      case _MIDI_CLOCK_BYTE:
        onCaptureMidiClock();
        break;
      case _MIDI_START_BYTE:
        onMidiStart();
        break;
      case _MIDI_STOP_BYTE:
        onMidiStop();
        break;
      default:
        midi_in_channel = MIDI.getChannel();
        midi_in_d1 = MIDI.getData1();
        midi_in_d2 = MIDI.getData2();
        onMidiEvent(midi_in_channel, midi_in_d1, midi_in_d2, midi_in_message_type);
    }
  }
}

void playBackLoop() {
  if (MIDI.read()) {
    midi_in_message_type = MIDI.getType();
    if(midi_in_message_type == _MIDI_CLOCK_BYTE) {
      onPlaybackMidiClock();
    }
  }
}

void loop() {
  if (digitalReadFast(LOOP_SWITCH_PIN) == HIGH) {
    isLooping = true;
  } else {
    isLooping = false;
  }

  if(isLooping) {
    playBackLoop();
  } else {
    midiRead();
  }
}