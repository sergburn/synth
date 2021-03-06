#include <Arduino.h>
#include <SoftwareSerial.h>

int pinLED = 13;

// OUT - Shift register 595
int pinData = 10;
int pinLatch = 11;
int pinClock = 12;
int pinTx = 8;
int pinRx = 9; // not used

// IN - MUX 4052
int pinX = 3;
int pinY = 2;
int pinA = 6;
int pinB = 7;

const int KNumKeys = 64;
byte keys[KNumKeys];

// First key of my keyboard 
const byte KMidiStart = 24; // C1

SoftwareSerial midiOut(pinRx, pinTx);

byte readMux4052()
{
  byte val = 0;

  for (int i = 3; i >= 0; i--)
  {
    digitalWrite(pinA, i & 1 ? HIGH : LOW);
    digitalWrite(pinB, i & 2 ? HIGH : LOW);

    val <<= 1;
    if (digitalRead(pinX) == HIGH)
    {
      val |= 1;
    }
    if (digitalRead(pinY) == HIGH)
    {
      val |= 16;
    }
  }
  return val;
}

byte invert(byte v)
{
  byte r = 0;
  for (int i = 0; i < 8; i++)
  {
    r <<= 1;
    r |= v & 1;
    v >>= 1;
  }
  return r;
}

void printNote(byte midiKey)
{
  const byte Octave = 12;
  
  byte octave = midiKey / Octave - 1;
  byte note = midiKey % Octave;

  const char* notes[] = {"C", "C#", "D", "D#", "E", "F", "F#", "G", "G#", "A", "A#", "H"};

  Serial.print(notes[note]);
  Serial.print(octave);
}

void sendMidiNote(byte midiKey, bool on)
{
  const byte KNoteOn = 0x90;
  const byte KVelo = 0x40; // default Velocity
  midiOut.write(KNoteOn);
  midiOut.write(midiKey);
  midiOut.write(on ? KVelo : 0);
  
  Serial.print(midiKey);
  Serial.println(on ? " ON" : " OFF");
}

void setup()
{
  Serial.begin(9600);
  pinMode(pinLED, OUTPUT);

  midiOut.begin(31250);
/**/
  pinMode(pinLatch, OUTPUT);
  pinMode(pinClock, OUTPUT);
  pinMode(pinData, OUTPUT);

  pinMode(pinA, OUTPUT);
  pinMode(pinB, OUTPUT);

  pinMode(pinX, INPUT);
  pinMode(pinY, INPUT);
/**/

  for (int i = 0; i < KNumKeys; i++)
  {
    keys[i] = LOW;
  }
  Serial.println("START");
}

void loop()
{
  delay(50);

//  Serial.print("STime: ");
//  Serial.println(millis());
/**/
#ifdef USE_SHIFTOUT
  for (int i = 0; i < 8; i++)
  {
    byte scan = 1 << i;
    digitalWrite(pinLatch, LOW);
    shiftOut(pinData, pinClock, LSBFIRST, scan);
    digitalWrite(pinLatch, HIGH);
#else
  digitalWrite(pinData, HIGH);
  for (int i = 7; i >= 0; i--)
  {
    digitalWrite(pinLatch, LOW); // output
    digitalWrite(pinClock, HIGH); // shift
    digitalWrite(pinLatch, HIGH); // output

    digitalWrite(pinClock, LOW);
    if (i == 7)
    {
      digitalWrite(pinData, LOW);
    }
#endif

    byte resp = readMux4052();
    // wiring requires to invert this value
    resp = invert(resp);
/**/
/**
    Serial.print("SCAN ");
    Serial.print(i);
/**/
/**
    Serial.print(" RESP ");
    Serial.println(resp);
/**/
/**/
    for (int j = 0; j < 8; j++)
    {
      byte key = i << 3 | j;

      byte pressed = bitRead(resp, j);
      if (pressed != keys[key])
      {
        keys[key] = pressed;
/**
        Serial.print(key);
        Serial.print(" ");
        printNote(KMidiStart + key);
        Serial.println(pressed ? " DN" : " UP");
/**/
        sendMidiNote(KMidiStart + key, pressed);
        
        if (key == 0)
        {
          digitalWrite(pinLED, pressed ? HIGH : LOW);
        }
      }
    }
  }
/**/
}
