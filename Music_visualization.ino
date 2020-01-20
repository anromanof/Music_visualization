#include <Audio.h>
#include <Wire.h>
#include <SPI.h>
#include <SD.h>
#include <SerialFlash.h>
#include <WS2812Serial.h>


// Audio Connections
AudioInputAnalog         adc1;
AudioOutputAnalog        dac1;
AudioAnalyzePeak         peak_finder;
AudioConnection          patchCord1(adc1, dac1);
AudioConnection          patchCord2(adc1, peak_finder);
AudioAnalyzeFFT1024      fft;
AudioConnection          patchCord3(adc1, fft);

// Switch between vizualizations 
const int SW = 12;


// LED Grid Setup
const int NUMLED = 8 * 32 ;
const int PIN = 1;
byte drawingMemory[NUMLED * 3]; //  3 bytes per LED
DMAMEM byte displayMemory[NUMLED * 12]; // 12 bytes per LED
WS2812Serial leds(NUMLED, displayMemory, drawingMemory, PIN, WS2812_GRB);


// Buffer for storing previous values in a circular buffer
const int HISTORY_SIZE = 32;
float history[HISTORY_SIZE];


//functions declaration
int led_index(int x, int y);
void update_history();
void visualize();
void visualization1();
void visualization2();


// this function is run once when the teensy starts up, in order to initialize
// things.
void setup() {
  leds.begin();  // initialize LED Grid

  //initialize history to be all 0's
  for (int i = 0; i < HISTORY_SIZE; i++) {
    history[i] = 0;
  }
  
  pinMode(SW, INPUT); //toggle
  AudioMemory(12);
  delay(1000); // power-up safety delay
  Serial.begin(115200);
}


// this function is called many times per second
void loop() {
  update_history();  // update the history array
  visualize();  // call the visualization function (below)
  leds.show();  // actually light up the LEDs
}


// Given x and y coordinates, return the index of the LED at that location
// (for use with setPixel)
int led_index(int x, int y) {
  if (y % 2 == 1)
    return (y*32 + x);
  else
    return (y*32 + (31 - x)); // TODO: replace with your code 
}


// this function will update the variable called history, so that it contains
// the most recent 32 values, with index 0 being the most recent.
void update_history() {
  if (peak_finder.available()) {
    for (int i = HISTORY_SIZE-1; i > 0; i--) {
      history[i] = history[i-1];
    }
    history[0] = peak_finder.read();
  }
}


// use visualization according to SW (switch position)
void visualize() {
  if (digitalRead(SW) == HIGH){
    visualization2();
  } else {
    visualization1();
  }
}


// this function broadcasts Amplitude(time) to the LED panel
void visualization1() {
  for (int h_i = 0; h_i < HISTORY_SIZE; ++h_i) {
    for (int y = 0; y < 8; ++y) {
      if (history[h_i] * 8 > (y + 1)) {
        leds.setPixel(led_index(h_i, y), 43, 30, 30);
      } else {
        leds.setPixel(led_index(h_i, y), 0, 0, 0);
      }
    }
     
  }
}

// this function broadcasts the frequency response of the audio signal
// using last 256 points to the LED panel
void visualization2() {
  if (fft.available()){
    for (int x = 0; x < 32; ++x) {
      float this_val = 256 * fft.read(x);
      for (int y = 0; y < 8; ++y) {
        if (this_val > (y + 1)) {
          leds.setPixel(led_index(x, y), 0, 40, 40);
        } else {
          leds.setPixel(led_index(x, y), 0, 0, 0);
        }
      }
    }
  }
}
