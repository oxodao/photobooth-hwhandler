// Inspired by RobSmithDev's Controller script (https://youtu.be/b3iqji1DUG0)
// Licensed under GPLv3

#include <RF24.h>

#define AMT_BUTTONS 3
#define AMT_BUZZERS 4 // Amount of buzzers you created, not the ones used in a game
#define STARTING_PIN 2

//   9 -> CE  (nRF24)
//  10 -> CSN (nRF24)
//  11 -> MO  (nRF24)
//  12 -> MI  (nRF24)
//  13 -> SCK (nRF24)

RF24 radio(9, 10); // CE, CSN

struct BuzzerState {
  unsigned long last_contact;

  bool connected;
  bool enabled;
  bool answered;
};

struct State {
  bool button_pressed[AMT_BUTTONS];

  unsigned long last_loop_time = 0;

  int amt_buzzers; // The actual amount of buzzers connected to play
  BuzzerState buzzers[AMT_BUZZERS];
};

State currentState;

//#region Button related stuff
void setInitialState() {
  currentState.amt_buzzers = 0;
  currentState.last_loop_time = 0;

  for (int i = 0; i < AMT_BUTTONS; i++) {
    currentState.button_pressed[i] = false;    
    pinMode(i+STARTING_PIN, INPUT_PULLUP);
  }
}

void checkButton(int btn) {
  bool is_pressed = digitalRead(btn + STARTING_PIN) == LOW;
  if (is_pressed && currentState.button_pressed[btn] == false) {
    Serial.write("BTN_");
    Serial.println(btn);
    currentState.button_pressed[btn] = true;
  } else if (!is_pressed) {
    currentState.button_pressed[btn] = false;
  }
}
//#endregion

//#region Buzzer related stuff
/*
bool findEmptyChannel()
{
    Serial.println("SCANNING_EMPTY_CHANNELS");
    char buffer[10];

    // Scan all channels looking for a quiet one.  We skip every 10
    for (int channel = 125; channel > 0; channel -= 10)
    {
        radio.setChannel(channel);
        delay(20);

        unsigned int inUse = 0;
        unsigned long testStart = millis();
        // Check for 400 ms per channel
        while (millis() - testStart < 400)
        {
            if ((radio.testCarrier()) || (radio.testRPD()))
            {
                inUse++;
            }

            delay(1);
        }

        // Low usage?
        if (inUse < 10)
        {
            itoa(channel, buffer, 10);
            Serial.write("CHANNEL_SELECTED ");
            Serial.println(buffer);
            return true;
        }
    }
    return false;
}

void sendAck()
{
    // Update the ACK for the next payload
    unsigned char payload[AMT_BUZZERS];
    for (int i = 0; i < AMT_BUZZERS; i++)
    {
        payload[i] = (buttonEnabled[i] ? 128 : 0) | ledStatus[button];
    }

    radio.writeAckPayload(1, &payload, 4);
}

void checkRadioMessageReceived() {
  if (radio.available()) {
    unsigned char buffer;
    radio.read(&buffer, 1);

    // Grab the button number from the data
    unsigned char buttonNumber = buffer & 0x7F; // Get the button number
    if ((buttonNumber >= 1) && (buttonNumber <= currentState.amt_buzzers)) {
      buttonNumber--;

      // Update the last contact time for this button
      lastContact[buttonNumber] = lastLoopTime;

      // And that it's connected
      buttonConnected[buttonNumber] = true;

      // If the button was pressed, was enabled, hasn't answered and the system is ready for button presses
      if ((buffer & 128) && (buttonEnabled[buttonNumber]) && (!hasAnswered[buttonNumber]) && (isReady)) {
        // No longer ready
        isReady = false;

        // Signal the button was pressed
        hasAnswered[buttonNumber] = true;
        Serial.write(buttonNumber);
        Serial.write("\n");


        // Change button status
        for (unsigned char btn = 0; btn < AMT_BUTTONS; btn++)
          ledStatus[btn] = (btn == buttonNumber) ? lsOn : lsOff;

        // Turn off the ready light
        digitalWrite(LED_STATUS, LOW);
      }
    }

    sendAck();
  }
}
*/
//#endregion

void setup()
{
    Serial.begin(57600);
    while(!Serial){}

    Serial.println("STARTING_UP");
    setInitialState();

    radio.begin();
    radio.setPALevel(RF24_PA_LOW);
    radio.enableDynamicPayloads();
    radio.enableAckPayload();
    radio.setDataRate(RF24_250KBPS);
    radio.setRetries(4, 8);
    radio.maskIRQ(false, false, false);  // not using the IRQs

    if (!radio.isChipConnected()) {
      Serial.println("NO_RF24");
    } else {
      Serial.println("OK_RF24");
/*
      // Now setup the pipes for the four buttons
      char pipe[6] = "0QBTN";
      radio.openWritingPipe((uint8_t*)pipe);
      pipe[0] = '1';
      radio.openReadingPipe(1, (uint8_t*)pipe);
      
      for (char channel = 0; channel < AMT_BUTTONS; channel++) {
        pinMode(BTN_LEDS[channel], OUTPUT);
        digitalWrite(BTN_LEDS[channel], LOW);
      }

      // Start listening for messages
      radio.startListening();

      // Find an empty channel to run on
      while (!findEmptyChannel()) {};

      // Start listening for messages
      radio.startListening();
      */
    }
}

void loop()
{
    currentState.last_loop_time = millis();

    for (int i = 0; i < AMT_BUTTONS; i++){
      checkButton(i);
    }

    // @TODO read serial to check if `BUZZER_RESET` or `BUZZER_READY` has been sent and do the stuff that was in the original script

    // @TODO read serial to check `START_BUZZERS X` with currentState.amt_buzzers = X
    /*
    if (currentState.amt_buzzers > 0) {
      for (int i = 0; i < currentState.amt_buzzers; i++) {
        if (currentState.buzzers[i] == NULL) {
          continue;
        }

        if (currentState.buzzers[i]->connected){
          if (currentState.last_loop_time - currentState.buzzers[i]->last_contact > 1000) {
            // If the current buzzer is connected but we haven't heard from it for a second, let's say its disconnected
            currentState.buzzers[i]->connected = false;
            Serial.write("DISCONNECT_BTN_");
            Serial.println(i);
          } else {
            // Send status to Serial
          }
        }
      }
    }

    checkRadioMessageReceived();*/
}
