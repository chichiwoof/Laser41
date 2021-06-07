#include <Audio.h>
#include <Bounce.h>
#include <Arduino.h>
#include <Wire.h>
#include <SPI.h>
#include <SD.h>
#include <SerialFlash.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <Adafruit_I2CDevice.h>
#include <Mux.h>


// WAV FILE HEADER INFO
unsigned long ChunkSize = 0L;
unsigned long Subchunk1Size = 16;
unsigned int AudioFormat = 1;
unsigned int numChannels = 2;
unsigned long sampleRate = 44100;
unsigned int bitsPerSample = 16;
unsigned long byteRate = sampleRate*numChannels*(bitsPerSample/8);// samplerate x channels x (bitspersample / 8)
unsigned int blockAlign = numChannels*bitsPerSample/8;
unsigned long Subchunk2Size = 0L;
unsigned long recByteSaved = 0L;
unsigned long NumSamples = 0L;
byte byte1, byte2, byte3, byte4;
// END WAV FILE HEADER INFO

//Button debouncing
Bounce buttonRecord = Bounce(0, 8);
Bounce buttonStop =   Bounce(1, 8);  // 8 = 8 ms debounce time
Bounce buttonPlay =   Bounce(2, 8);

// OLED DEFS
#define SCREEN_WIDTH 128 // OLED display width, in pixels
#define SCREEN_HEIGHT 64 // OLED display height, in pixels

// Declaration for an SSD1306 display connected to I2C (SDA, SCL pins)
// The pins for I2C are defined by the Wire-library. 
// On an arduino UNO:       A4(SDA), A5(SCL)
// On an arduino MEGA 2560: 20(SDA), 21(SCL)
// On an arduino LEONARDO:   2(SDA),  3(SCL), ...
#define OLED_RESET     -1 // Reset pin # (or -1 if sharing Arduino reset pin)
#define SCREEN_ADDRESS 0x3C ///< See datasheet for Address; 0x3D for 128x64, 0x3C for 128x32
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

#define LOGO_HEIGHT   56
#define LOGO_WIDTH    56

static const unsigned char PROGMEM logo_sask[] =
{
B00000000, B00000000, B00000001, B11111111, B10000000, B00000000, B00000000,
B00000000, B00000000, B00011111, B11111111, B11111000, B00000000, B00000000,
B00000000, B00000000, B11111111, B11111111, B11111111, B00000000, B00000000,
B00000000, B00000011, B11111111, B11111111, B11111111, B11000000, B00000000,
B00000000, B00000111, B11111111, B11111111, B11111111, B11100000, B00000000,
B00000000, B00011111, B11111111, B11111111, B11111111, B11111000, B00000000,
B00000000, B00111111, B11111111, B11111111, B11111111, B11111100, B00000000,
B00000000, B01111111, B11111111, B11111111, B11111111, B11111110, B00000000,
B00000000, B11111111, B11111111, B11111111, B11111111, B11111111, B00000000,
B00000001, B11111111, B11111111, B11111111, B11111111, B11111111, B10000000,
B00000011, B11111111, B11111111, B11111111, B11111111, B11111111, B11000000,
B00000111, B11111111, B11111111, B11111111, B11111111, B11111111, B11100000,
B00000111, B11111111, B11111111, B11111111, B11111111, B11111111, B11100000,
B00001111, B11111111, B11111111, B11111111, B11111111, B11111111, B11110000,
B00011111, B11111111, B11111111, B11111111, B11111111, B11111111, B11110000,
B00011111, B11111111, B11111111, B11111111, B11111000, B00011111, B11111000,
B00111111, B11111111, B11111111, B11111111, B11110000, B00001111, B11111000,
B00111111, B11111111, B11111111, B11111111, B11110000, B00001111, B11111100,
B00111111, B11111111, B11111111, B11111111, B11100000, B00000111, B11111100,
B01111111, B11111111, B11111111, B11111111, B11100000, B00000111, B11111110,
B01111111, B11111111, B11111111, B11111111, B11100000, B00000011, B11111110,
B01111111, B11111111, B11111111, B11111111, B11000000, B00000011, B11111110,
B01111111, B11111111, B11111111, B11111111, B11000000, B00000011, B11111110,
B11111111, B11111111, B11111111, B11111111, B11000000, B00000001, B11111111,
B11111111, B11111111, B11111111, B11111111, B10000000, B00000101, B11111111,
B11111111, B11111111, B11111111, B11111111, B10000000, B00000110, B11111111,
B11111111, B11111111, B11111111, B11111111, B00000000, B00000111, B01111111,
B11111111, B11111111, B11111111, B11111110, B00000000, B00000111, B11111111,
B11111111, B11111111, B11111111, B11111110, B00000000, B01000111, B11111111,
B11111111, B11111111, B11111111, B11111100, B00000000, B01100111, B11111111,
B11111111, B11111111, B11111111, B11110000, B00000000, B01100111, B11111111,
B11111111, B11111111, B11111111, B11000000, B00000000, B01110111, B11111111,
B11111111, B11111111, B11111111, B00000000, B00000000, B11110011, B11111111,
B01111111, B11111111, B11111000, B00000000, B00000000, B01110111, B11111110,
B01111111, B11110000, B00000000, B00000000, B00000000, B11100111, B11111110,
B01111111, B10000000, B00000000, B00000000, B00000000, B11100111, B11111110,
B01111111, B10000000, B00000000, B00000000, B00111001, B11001111, B11111110,
B00111111, B11111100, B00000000, B00000000, B01110001, B11111111, B11111100,
B00111111, B11111111, B11111100, B01111111, B11110011, B11111111, B11111100,
B00111111, B11111111, B11111111, B11111111, B11000111, B11111111, B11111000,
B00011111, B11111111, B11111111, B11111111, B11111111, B11111111, B11111000,
B00011111, B11111111, B11111111, B11111111, B11111111, B11111111, B11110000,
B00001111, B11111111, B11111111, B11111111, B11111111, B11111111, B11110000,
B00000111, B11111111, B11111111, B11111111, B11111111, B11111111, B11100000,
B00000111, B11111111, B11111111, B11111111, B11111111, B11111111, B11100000,
B00000011, B11111111, B11111111, B11111111, B11111111, B11111111, B11000000,
B00000001, B11111111, B11111111, B11111111, B11111111, B11111111, B10000000,
B00000000, B11111111, B11111111, B11111111, B11111111, B11111111, B00000000,
B00000000, B01111111, B11111111, B11111111, B11111111, B11111110, B00000000,
B00000000, B00111111, B11111111, B11111111, B11111111, B11111100, B00000000,
B00000000, B00011111, B11111111, B11111111, B11111111, B11111000, B00000000,
B00000000, B00000111, B11111111, B11111111, B11111111, B11100000, B00000000,
B00000000, B00000001, B11111111, B11111111, B11111111, B10000000, B00000000,
B00000000, B00000000, B01111111, B11111111, B11111110, B00000000, B00000000,
B00000000, B00000000, B00011111, B11111111, B11111000, B00000000, B00000000,
B00000000, B00000000, B00000001, B11111111, B10000000, B00000000, B00000000
};
// END OLED DEFS

// GUItool: begin automatically generated code
AudioInputI2S            i2s1;           //xy=142,190
AudioRecordQueue         queue1;         //xy=334,107
AudioRecordQueue         queue2;         //xy=351,154
AudioPlaySdWav           audioSD;        //xy=514,238
AudioOutputI2S           i2s2;           //xy=706,241
AudioConnection          patchCord1(i2s1, 0, queue1, 0);
AudioConnection          patchCord2(i2s1, 1, queue2, 0);
AudioConnection          patchCord3(audioSD, 0, i2s2, 0);
AudioConnection          patchCord4(audioSD, 1, i2s2, 1);
AudioControlSGTL5000     sgtl5000_1;     //xy=365,311
// GUItool: end automatically generated code

// which input on the audio shield will be used?
const int myInput = AUDIO_INPUT_LINEIN;
//const int myInput = AUDIO_INPUT_MIC;

// Use these with the Teensy Audio Shield
#define SDCARD_CS_PIN    10
#define SDCARD_MOSI_PIN  7
#define SDCARD_SCK_PIN   14
// Use these with the Teensy 3.5 & 3.6 SD card
//#define SDCARD_CS_PIN    BUILTIN_SDCARD // 254?
//#define SDCARD_MOSI_PIN  11  // not actually used
//#define SDCARD_SCK_PIN   13  // not actually used

// Remember which mode we're doing
int mode = 0;  // 0=stopped, 1=recording, 2=playing
// The file where data is recorded
File frec;
//to count how long each sample takes to buff
elapsedMillis  msecs;

//declare functions before they are called for platformio
void stopPlaying();
void startPlaying();
void startRecording();
void stopRecording();
void continueRecording();
void continuePlaying();
void writeOutHeader();
void adjustMicLevel();
void playFile();
void testdrawbitmap();
void playOLED();
void stopOLED();
void recOLED();
void displayPot();



void playFile(const char *filename)
{
  playOLED();
  Serial.print("Playing file: ");
  Serial.println(filename);

  // Start playing the file.  This sketch continues to
  // run while the file plays.
  audioSD.play(filename);

  // A brief delay for the library read WAV info
   delay(25);

  // Simply wait for the file to finish playing.
  //while (audioSD.isPlaying()) {
    // uncomment these lines if you audio shield
    // has the optional volume pot soldered
    //float vol = analogRead(15);
    //vol = vol / 1024;
    // sgtl5000_1.volume(vol);
  //}
}

//////////// MUX STUFF /////
using namespace admux;
/*
 * Creates a Mux instance.
 * 1st argument is the SIG (signal) pin (Arduino analog input pin A0).
 * 2nd argument is the S0-S3 (channel control) pins (Arduino pins 8, 9, 10, 11).
 */
Mux potmux(Pin(24, INPUT, PinType::Analog), Pinset(29, 34, 35));
Mux mux(Pin(25, INPUT, PinType::Digital), Pinset(29, 34, 35));
int potData[8];
byte buttonData[8];
////////////// END MUX STUFF /////


void setup() {

   Serial.begin(9600);

  // SSD1306_SWITCHCAPVCC = generate display voltage from 3.3V internally
  if(!display.begin(SSD1306_SWITCHCAPVCC, SCREEN_ADDRESS)) {
    Serial.println(F("SSD1306 allocation failed"));
    for(;;); // Don't proceed, loop forever
  }
   // Show initial display buffer contents on the screen --
  // the library initializes this with an Adafruit splash screen.
  display.display();
  delay(2000); // Pause for 2 seconds

  // Clear the buffer
  display.clearDisplay();


  // record queue uses this memory to buffer incoming audio.
  AudioMemory(60); // 60

// Configure the pushbutton pins

 




  // Enable the audio shield, select input, and enable output
  sgtl5000_1.enable();
  sgtl5000_1.inputSelect(myInput);
  sgtl5000_1.volume(0.8);
  sgtl5000_1.lineInLevel(2,2);



  // Initialize the SD card
  SPI.setMOSI(SDCARD_MOSI_PIN);
  SPI.setSCK(SDCARD_SCK_PIN);
  if (!(SD.begin(SDCARD_CS_PIN))) {
    // stop here if no SD card, but print a message
    while (1) {
      Serial.println("Unable to access the SD card");
      delay(500);
    }
  }

display.clearDisplay();


}


void loop() {
  
  // for (byte o = 0; o < potmux.channelCount(); o++) {
  //   potdata = potmux.read(o) /* Reads from channel i (returns a value from 0 to 1023) */;
  // }


  for (byte i = 0; i < mux.channelCount(); i++) {
    delay(5);
   buttonData[i] = mux.read(i); /* Reads from channel i (returns HIGH or LOW) */;

    Serial.print("Push button at channel "); Serial.print(i); Serial.print(" is "); Serial.println(buttonData[i] == LOW ? "pressed" : "not pressed");
  }
  Serial.println();

  delay(15);


  for (int o = 0; o < potmux.channelCount(); o++) {
   potData[o] = potmux.read(o); /* Reads from channel i (returns a value from 0 to 1023) */;

    Serial.print("Potentiometer at channel "); Serial.print(o); Serial.print(" is at "); Serial.print((double) (potData[o]) * 100 / 1023); Serial.println("%");
  }
  Serial.println();

  delay(15);
  displayPot();

  // First, read the buttons
  buttonRecord.update();
  buttonStop.update();
  buttonPlay.update();

  // Respond to button presses
  if (buttonRecord.fallingEdge()) {
    Serial.println("Record Button Press");
    if (mode == 2) stopPlaying();
    if (mode == 0) startRecording();
  }
  if (buttonStop.fallingEdge()) {
    Serial.println("Stop Button Press");
    if (mode == 1) stopRecording();
    if (mode == 2) stopPlaying();
  }
  if (buttonPlay.fallingEdge()) {
    Serial.println("Play Button Press");
    if (mode == 1) stopRecording();
    if (mode == 0) startPlaying();
  }

  // If playing or recording, continue...
  if (mode == 1) {
    continueRecording();
  }
  if (mode == 2) {
    continuePlaying();
  }

}

void displayPot() {
  display.clearDisplay();
  display.setCursor(0,20);
  display.setTextSize(2);  // Draw 2X-scale text
  display.setTextColor(SSD1306_WHITE, SSD1306_BLACK); // Draw 'inverse' text
  display.print("SAMPLE ");
  int samp = potData[7] * 16 / 1022;
  display.println(samp);
  display.display();
}

void testdrawbitmap(void) {
  display.clearDisplay();

  display.drawBitmap(
    (display.width()  - LOGO_WIDTH ) / 2,
    (display.height() - LOGO_HEIGHT) / 2,
    logo_sask, LOGO_WIDTH, LOGO_HEIGHT, 1);
  display.display();
  // delay(1000);
}


void startRecording() {
  recOLED();
  Serial.println("StartRecording");
  if (SD.exists("REC2.WAV")) {
    SD.remove("REC2.WAV");
  }
  frec = SD.open("REC2.WAV", FILE_WRITE);
  if (frec) {
    Serial.println("File Open");
    queue1.begin();
    queue2.begin();
    mode = 1;
    recByteSaved = 0L; //this is from wav recorder
  }

}

// write all 512 bytes to the SD card   
void continueRecording() {
  if (queue1.available() >= 2 && queue2.available() >= 2) {
    byte buffer[512];
    byte bufferL[256];
    byte bufferR[256];
    memcpy(bufferL, queue1.readBuffer(), 256);
    memcpy(bufferR, queue2.readBuffer(), 256);
    queue1.freeBuffer();
    queue2.freeBuffer();
    int b = 0;
    for (int i = 0; i < 512; i += 4) {
      buffer[i] = bufferL[b];
      buffer[i + 1] = bufferL[b + 1];
      buffer[i + 2] = bufferR[b];
      buffer[i + 3] = bufferR[b + 1];
      b = b+2;
    }
    elapsedMicros usec = 0;
    frec.write(buffer, 512);  //256 or 512 (dudes code)
    recByteSaved += 512;
    Serial.print("SD write, us=");
    Serial.println(usec);
  }
}

void stopRecording() {
  Serial.println("StopRecording");
  queue1.end();
  queue2.end();
  // flush buffer
  
    while (queue1.available() > 0 && queue2.available() > 0) {
      queue1.readBuffer();
      queue1.freeBuffer();
      queue2.readBuffer();
      queue2.freeBuffer();
      recByteSaved += 256;
    }
    writeOutHeader();
    frec.close(); // close file
  
  mode = 0;
}

void startPlaying() {
  Serial.println("startPlaying");
  playFile("REC2.WAV");  // filenames are always uppercase 8.3 format
  // audioSD.play("GLUB4.WAV");
  mode = 2;
}

void continuePlaying() {
  if (!audioSD.isPlaying()) {
    audioSD.stop();
    display.clearDisplay();
    display.display();
    mode = 0;
  }
}

void stopPlaying() {
  stopOLED();
  Serial.println("stopPlaying");
  if (mode == 2) audioSD.stop();
  mode = 0;
}


void writeOutHeader() { // update WAV header with final filesize/datasize
//  NumSamples = (recByteSaved*8)/bitsPerSample/numChannels;
//  Subchunk2Size = NumSamples*numChannels*bitsPerSample/8; // number of samples x number of channels x number of bytes per sample
  Subchunk2Size = recByteSaved;
  ChunkSize = Subchunk2Size + 36;
  frec.seek(0);
  frec.write("RIFF");
  byte1 = ChunkSize & 0xff;
  byte2 = (ChunkSize >> 8) & 0xff;
  byte3 = (ChunkSize >> 16) & 0xff;
  byte4 = (ChunkSize >> 24) & 0xff;  
  frec.write(byte1);  frec.write(byte2);  frec.write(byte3);  frec.write(byte4);
  frec.write("WAVE");
  frec.write("fmt ");
  byte1 = Subchunk1Size & 0xff;
  byte2 = (Subchunk1Size >> 8) & 0xff;
  byte3 = (Subchunk1Size >> 16) & 0xff;
  byte4 = (Subchunk1Size >> 24) & 0xff;  
  frec.write(byte1);  frec.write(byte2);  frec.write(byte3);  frec.write(byte4);
  byte1 = AudioFormat & 0xff;
  byte2 = (AudioFormat >> 8) & 0xff;
  frec.write(byte1);  frec.write(byte2); 
  byte1 = numChannels & 0xff;
  byte2 = (numChannels >> 8) & 0xff;
  frec.write(byte1);  frec.write(byte2); 
  byte1 = sampleRate & 0xff;
  byte2 = (sampleRate >> 8) & 0xff;
  byte3 = (sampleRate >> 16) & 0xff;
  byte4 = (sampleRate >> 24) & 0xff;  
  frec.write(byte1);  frec.write(byte2);  frec.write(byte3);  frec.write(byte4);
  byte1 = byteRate & 0xff;
  byte2 = (byteRate >> 8) & 0xff;
  byte3 = (byteRate >> 16) & 0xff;
  byte4 = (byteRate >> 24) & 0xff;  
  frec.write(byte1);  frec.write(byte2);  frec.write(byte3);  frec.write(byte4);
  byte1 = blockAlign & 0xff;
  byte2 = (blockAlign >> 8) & 0xff;
  frec.write(byte1);  frec.write(byte2); 
  byte1 = bitsPerSample & 0xff;
  byte2 = (bitsPerSample >> 8) & 0xff;
  frec.write(byte1);  frec.write(byte2); 
  frec.write("data");
  byte1 = Subchunk2Size & 0xff;
  byte2 = (Subchunk2Size >> 8) & 0xff;
  byte3 = (Subchunk2Size >> 16) & 0xff;
  byte4 = (Subchunk2Size >> 24) & 0xff;  
  frec.write(byte1);  frec.write(byte2);  frec.write(byte3);  frec.write(byte4);
  frec.close();
  Serial.println("header written"); 
  Serial.print("Subchunk2: "); 
  Serial.println(Subchunk2Size); 
}

void playOLED(void) {
  display.clearDisplay();
  display.setCursor(20,20);
  display.setTextSize(2);  // Draw 2X-scale text
  display.setTextColor(SSD1306_BLACK, SSD1306_WHITE); // Draw 'inverse' text
  display.println("PLAYING");

  // display.setTextSize(2);             // Draw 2X-scale text
  // display.setTextColor(SSD1306_WHITE);
  // display.print(F("0x")); display.println(0xDEADBEEF, HEX);

  display.display();

}

void stopOLED(void) {
  display.clearDisplay();
  display.setCursor(20,20);
  display.setTextSize(2);  // Draw 2X-scale text
  display.setTextColor(SSD1306_WHITE, SSD1306_BLACK); // Draw 'inverse' text
  display.println("STOPPED");

  // display.setTextSize(2);             // Draw 2X-scale text
  // display.setTextColor(SSD1306_WHITE);
  // display.print(F("0x")); display.println(0xDEADBEEF, HEX);

  display.display();
}

void recOLED(void) {
  display.clearDisplay();
  testdrawbitmap();
  display.display();
}