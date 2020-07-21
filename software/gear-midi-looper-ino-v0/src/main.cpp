#include <Arduino.h>

/* MIDI Input Test - for use with Teensy or boards where Serial is separate from MIDI
 * As MIDI messages arrive, they are printed to the Arduino Serial Monitor.
 *
 * Where MIDI is on "Serial", eg Arduino Duemilanove or Arduino Uno, this does not work!
 *
 * This example code is released into the public domain.
 */
 
#include <MIDI.h>

MIDI_CREATE_INSTANCE(HardwareSerial, Serial1, MIDI);

const int _MIDI_CLOCK_BYTE = 0xF8;
const int _MIDI_START_BYTE = 0xFA;
const int _MIDI_STOP_BYTE = 0xFC;
const int _MAX_CLOCK_COUNTER_SIZE = 64;
const int _MAX_EVENT_BUFFER_SIZE = 128;

typedef struct midi_event_t {
  int lastClock;
  uint32_t timeSinceLastClock;
  byte midiEventType;
  int channel;
  int note;
  int velocity;
};

// playhead / clock counting
volatile int clockCounter = 0;
uint32_t lastClockTime = 0;
midi_event_t events[_MAX_CLOCK_COUNTER_SIZE][_MAX_EVENT_BUFFER_SIZE];
int lastRecordedEventCounter[_MAX_CLOCK_COUNTER_SIZE];

unsigned long t = 0;

void onMidiClock() {
  lastClockTime = micros();

  if(clockCounter < _MAX_CLOCK_COUNTER_SIZE) {
    clockCounter++;
    return;
  }

  Serial.println(String("RESETTING CLOCK COUNTER"));
  clockCounter = 0;
  for(int i = 0; i < _MAX_CLOCK_COUNTER_SIZE; i++) {
    lastRecordedEventCounter[i] = 0;
  }
}

void onMidiStart() {
  Serial.println(String("MIDI Start"));
}

void onMidiStop() {
  Serial.println(String("MIDI Stop"));
}

void onMidiNoteOn(int channel, int note, int velocity) {
  Serial.println(String("Note On:  ch=") + channel + ", note=" + note + ", velocity=" + velocity);
  midi_event_t e;
  e.lastClock = clockCounter;
  e.timeSinceLastClock = micros() - lastClockTime;
  e.note = note;
  e.channel = channel;
  e.velocity = velocity;
  e.midiEventType = midi::NoteOn;
  events[clockCounter][lastRecordedEventCounter[clockCounter]] = e;

  if(lastRecordedEventCounter[clockCounter] < _MAX_EVENT_BUFFER_SIZE) {
    lastRecordedEventCounter[clockCounter]++;
  } else {
    lastRecordedEventCounter[clockCounter] = 0;
  }
}

void onMidiNoteOff(int channel, int note, int velocity) {
  Serial.println(String("Note Off: ch=") + channel + ", note=" + note + ", velocity=" + velocity);
  midi_event_t e;
  e.lastClock = clockCounter;
  e.timeSinceLastClock = micros() - lastClockTime;
  e.note = note;
  e.channel = channel;
  e.velocity = velocity;
  e.midiEventType = midi::NoteOff;
  events[clockCounter][lastRecordedEventCounter[clockCounter]] = e;

  if(lastRecordedEventCounter[clockCounter] < _MAX_EVENT_BUFFER_SIZE) {
    lastRecordedEventCounter[clockCounter]++;
  } else {
    lastRecordedEventCounter[clockCounter] = 0;
  }
}

void setup() {
  MIDI.begin(MIDI_CHANNEL_OMNI);
  Serial.begin(57600);
  Serial.println("MIDI Input Test");

  for(int i = 0; i < _MAX_CLOCK_COUNTER_SIZE; i++) {
    lastRecordedEventCounter[i] = 0;
  }
}

int type, note, velocity, channel, d1, d2;
void loop() {
  if (MIDI.read()) {                    // Is there a MIDI message incoming ?
    byte type = MIDI.getType();
    switch (type) {
      case midi::NoteOn:
        channel = MIDI.getChannel();
        note = MIDI.getData1();
        velocity = MIDI.getData2();
        if (velocity > 0) {
          onMidiNoteOn(channel, note, velocity);
        } else {
          onMidiNoteOff(channel, note, velocity);
        }
        break;
      case midi::NoteOff:
        note = MIDI.getData1();
        velocity = MIDI.getData2();
        channel = MIDI.getChannel();
        onMidiNoteOff(channel, note, velocity);
        break;
      case _MIDI_CLOCK_BYTE:
        onMidiClock();
        break;
      case _MIDI_START_BYTE:
        onMidiStart();
        break;
      case _MIDI_STOP_BYTE:
        onMidiStop();
        break;
      default:
        d1 = MIDI.getData1();
        d2 = MIDI.getData2();
        Serial.println(String("Message, type=") + type + ", data = " + d1 + " " + d2);
    }
    t = millis();
  }
  if (millis() - t > 10000) {
    t += 10000;
    Serial.println("(inactivity)");
  }
}