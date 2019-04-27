/*
  MIDI note player

  This sketch shows how to use the serial transmit pin (pin 1) to send MIDI note data.
  If this circuit is connected to a MIDI synth, it will play
  the notes F#-0 (0x1E) to F#-5 (0x5A) in sequence.

  The circuit:
   digital in 1 connected to MIDI jack pin 5
   MIDI jack pin 2 connected to ground
   MIDI jack pin 4 connected to +5V through 220-ohm resistor
  Attach a MIDI cable to the jack, then to a MIDI synth, and play music.


  /*
  Creation: 5 Aug 2017
  ©-2017 Patrick-Gilles Maillot

  TtapMIDI-X32: a Stomp pedal for sending Tempo TAP delay as SYSEX MIDI messages to an X32 digital mixer from
  Behringer (or any compatible MIDI devive).

  The program waits for a HW pin to becon=me active (when the user presses a switch). These events
  are used to generate a delay sent as SYSEX MIDI array to the connected device.

  This project started from an Arduino project/sketch file: MIDI note player / created 13 Jun 2006

  The circuit for MIDI out:
    Leonardo digital pin 1 connected to MIDI jack pin 5
    MIDI jack pin 2 connected to ground
    MIDI jack pin 4 connected to +5V through 220-ohm resistor
  Attach a MIDI cable to the jack, then to a MIDI synth, and play music.

  The circuit for the switch:
    Pushbutton attached to Leonardo digital pin pin 2 and to ground
*/
#include <SoftwareSerial.h>

#include "MIDI.h"

unsigned char S_array[] = {                         // MIDI SYSEX or command array data
  0xF0, 0x00, 0x20, 0x32, 0x32,
  0x2F, 0x66, 0x78, 0x2F,
  0x34,
  0x2F, 0x70, 0x61, 0x72, 0x2F,
  0x30, 0x31,
  0x20,
  0x33, 0x30, 0x30, 0x30,
  0xF7
};
//
const int     S_size = sizeof(S_array) / sizeof(char);  // MIDI SYSEX or command array size
int             clockCount = 0;                            // Клоков за текущее время
int             mipsDelta = 3000;
unsigned long Time1 = 0;                                   // Start time
double bpm = 0;
int Dtime;                                    // Delta time
int ledPin = 13;
//
MIDI_CREATE_INSTANCE(HardwareSerial, Serial, midi_IO);
//SoftwareSerial mySerial(10, 11);

void setup() {
  // called once at program start
  //  Set MIDI baud rate:
  //  Serial.begin(9600);
  while (! Serial); // Wait untilSerial is ready - Leonardo
  //  Serial.println("Enter LED Number 0 to 7 or 'x' to clear");


  Serial.begin(31250);
  midi_IO.begin(MIDI_CHANNEL_OMNI); // Initialize the Midi Library.

  midi_IO.setHandleControlChange(MyHandleControlChange);
  midi_IO.setHandleClock(tap);
  pinMode(ledPin, OUTPUT);
  // Set pins directions
  //  pinMode(B_pin, INPUT_PULLUP);
  //  pinMode(L_pin, OUTPUT);
  //  S_state = digitalRead(B_pin);
  //  digitalWrite(L_pin, HIGH);
  //  Time1 = Dtime = Tcount = 0;
}

void loop() {

//  tap();
  midi_IO.read();
  //  midi_IO.turnThruOn();


}

void sendTap() {
  Dtime = 60 / bpm * 1000;
  if (bpm < 60)
    Dtime = 1500;
  // Dtime as valid X32 data 0 to 3000 ms
  if (Dtime < 1) Dtime = 0;
  if (Dtime > 3000) Dtime = 3000;
  // set value of FX number in SYSEX buffer - второе устройство в реке.
  S_array[9] = 0x32;
  // set value of Ftime into SYSEX buffer
  S_array[18] = 0x30 + Dtime / 1000;
  S_array[19] = 0x30 + (Dtime - (Dtime / 1000) * 1000) / 100;
  S_array[20] = 0x30 + (Dtime - (Dtime / 100) * 100) / 10;
  S_array[21] = 0x30 + (Dtime - (Dtime / 10) * 10);
  // reset time press counter
  // play all bytes from SYSEX data array:
  for (int i = 0; i < S_size; i ++) {
    Serial.write(S_array[i]);
  }
}

void tap()

{
//  if (Serial.read() == 0xf8)  {
    if (Time1 == 0) {
      Time1 = millis();
      //      Serial.print("time:");
      //      Serial.print(Time1);
    }
    else {
      if ((millis() - Time1) < mipsDelta ) {

        clockCount++;
        // поимгаем встроенным диодом в так песни
        if (clockCount % 24 == 0) {
          digitalWrite(ledPin, HIGH);
        }
        else
          digitalWrite(ledPin, LOW);
      }
      else
      {

        //        Serial.print("clockCount:");
        //        Serial.println(clockCount);
        double tempBPM = round(round(clockCount / 72 * 50) / 50 * 60);
        if (tempBPM != bpm)
        {
//          Serial.print(tempBPM);
          bpm = tempBPM;
          sendTap();
        }

        //        Serial.print("bpm:");
        //        Serial.println(bpm);
        clockCount = 0;
        Time1 = 0;
      }

    }
//  }

}

void MyHandleControlChange(byte channel, byte number, byte value) {

  switch (number) {
    //Включение режима Октава вверх на Whammy при получении CC_TG_60
    case 4:
      if (value > 0)
        midi_IO.sendControlChange(18, 127, 2);
      else
        midi_IO.sendControlChange(18, 0, 2);

      break;
    //Управление педалью экспрессии
    case 16 :
    //передача данных по управлению педалью экспрессии на Whammy
    //      midi_THRU.sendControlChange(11, value, 4);//передача данных по управлению педалью экспрессии на Whammy
    default:
      break;
  }
}
