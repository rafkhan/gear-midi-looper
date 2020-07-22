#ifndef MIDI_LOOPER_H
#define MIDI_LOOPER_H

#include <Arduino.h>
#include <MIDI.h>

typedef struct {
  byte midiEventType;
  byte channel;
  byte note;
  byte velocity;
} midi_event_t;

const int _MIDI_CLOCK_BYTE = 0xF8;
const int _MIDI_START_BYTE = 0xFA;
const int _MIDI_STOP_BYTE = 0xFC;
const int _MAX_CLOCK_COUNTER_SIZE = 24 * 4 * 4;
const int _MAX_CLOCK_EVENT_BUFFER_SIZE = 64;
const int _MAX_EVENT_BUFFER_SIZE = 4096;

// playhead / clock counting
volatile int clockCounter = 0;
uint32_t lastClockTime = 0;
volatile int lastRecordedEventCounter = 0;
midi_event_t events[_MAX_EVENT_BUFFER_SIZE];
midi_event_t clockEventLookup[_MAX_CLOCK_COUNTER_SIZE][_MAX_CLOCK_EVENT_BUFFER_SIZE];
int clockEventLookupCounters[_MAX_CLOCK_COUNTER_SIZE];
int recordedEventCountForClock[_MAX_CLOCK_COUNTER_SIZE];

void initializeMidiLooper() {
  for(int i = 0; i < _MAX_CLOCK_COUNTER_SIZE; i++) {
    clockEventLookupCounters[i] = 0;
  }
}

void onCaptureMidiClock() {
  lastClockTime = micros();
  lastRecordedEventCounter = 0;
  clockEventLookupCounters[clockCounter] = 0;

  // if not looping
  memset(clockEventLookup[clockCounter], 0, _MAX_CLOCK_EVENT_BUFFER_SIZE);

  if(clockCounter < _MAX_CLOCK_COUNTER_SIZE - 1) {
    clockCounter++;
    return;
  }

  Serial.println(String("RESETTING CLOCK COUNTER"));
  clockCounter = 0;
}

void onPlaybackMidiClock() {
  lastClockTime = micros();

  for(int i = 0; i < clockEventLookupCounters[clockCounter]; i++) {
    Serial.print(clockEventLookupCounters[clockCounter]);
    Serial.println(String(" events on this clock."));
    midi_event_t e = clockEventLookup[clockCounter][i];
    Serial.println(e.note);
    Serial.println(e.velocity);
  }

  if(clockCounter < _MAX_CLOCK_COUNTER_SIZE - 1) {
    clockCounter++;
    return;
  }

  Serial.println(String("RESETTING CLOCK COUNTER"));
  clockCounter = 0;
}


void addMidiEvent() {
  if(clockEventLookupCounters[clockCounter] < _MAX_CLOCK_COUNTER_SIZE - 1) {
    clockEventLookupCounters[clockCounter]++;
  } else {
    Serial.println(String("CLOCK EVENT RESET"));
    clockEventLookupCounters[clockCounter] = 0;
  }

  if(lastRecordedEventCounter < _MAX_EVENT_BUFFER_SIZE - 1) {
    lastRecordedEventCounter++;
  } else {
    Serial.println(String("EVENT OVERFLOW"));
    lastRecordedEventCounter = 0;
  }
}

void onMidiStart() {}
void onMidiStop() {}
void onMidiEvent(byte channel, byte note, byte velocity, byte midiEventType) {
  midi_event_t e;
  e.note = note;
  e.channel = channel;
  e.velocity = velocity;
  e.midiEventType = midiEventType;

  events[lastRecordedEventCounter] = e;
  clockEventLookup[clockCounter][clockEventLookupCounters[clockCounter]] = e;

  Serial.print(clockCounter);
  Serial.print(" ");
  Serial.print(note);
  Serial.print(" ");
  Serial.print(velocity);
  Serial.print(" ");
  Serial.print(lastRecordedEventCounter);
  Serial.print(" ");
  Serial.println(clockEventLookupCounters[clockCounter]);

  addMidiEvent();
}


#endif