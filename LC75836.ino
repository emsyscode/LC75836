/*
This code is not clean and far from perfect, that's just
a reference to extract ideas and adapt to your solution.
you can replace the BIN values with HEX... I leave it in BIN
because it is easier to relate the segment number with
the position of the bit in BIN.
Of course, a library can be created for this purpose! But I won't 
take the time to do that, I'll leave it up to you!
*/

/*
 * Note: DD ... Direction data
 * •CCB address...............46H
 * •D1 to D140.................Display data (At the LC75836, the display data D33 to D36, D69 to D72, D105 to D108, D133 to D136 must be set to 0.
 * •P0 to P2......................Segment output port/general-purpose output port switching control data
 * •DR..............................1/2-bias drive or 1/3-bias drive switching control data
 * •SC...............................Segments on/off control data
 * •BU..............................Normal mode/power-saving mode control dataNo. 5597-8/18 LC75836
 * 
 *  0, FC0, FC1, FC2, P0, P1, P2, OC, SC, BU, DD, DD; //Set of LC75836
 */

//#include <Arduino.h>
//#include <stdio.h>
//#include <math.h>
//#include <stdbool.h>

void send_char(unsigned char a);
void send_data(unsigned char a);
void segments();
void buttonReleasedInterrupt();  

#define LCD_in 8  // This is the pin number 8 on Arduino UNO
#define LCD_clk 9 // This is the pin number 9 on Arduino UNO
#define LCD_CE 10 // This is the pin number 10 on Arduino UNO

//unsigned int numberSeg = 0;  // Variable to supporte the number of segment
//unsigned int numberByte = 0; // Variable to supporte the number byte 
unsigned int shiftBit=0;
unsigned int nBitOnBlock=0; // Used to count number of bits and split to 8 bits... (number of byte)
unsigned int nByteOnBlock=0; 
unsigned int sequencyByte=0x00;
byte Aa,Ab,Ac,Ad,Ae,Af,Ag;
byte blockBit =0x00;

//atoi(to integer)
//constants won't change. They're used here to set pin numbers:
//const int buttonPin = 7;  // the number of the pushbutton pin
const int ledPin = 12;    // the number of the LED pin

#define BUTTON_PIN 2 //Att check wich pins accept interrupts... Uno is 2 & 3
volatile byte buttonReleased = false;

// variables will change:
int buttonState = 0;  // variable for reading the pushbutton status

bool forward = false;
bool backward = false;
bool isRequest = true;
bool allOn=false;
bool cycle=false;
/*
#define BIN(x) \
( ((0x##x##L & 0x00000001L) ? 0x01 : 0) \
| ((0x##x##L & 0x00000010L) ? 0x02 : 0) \
| ((0x##x##L & 0x00000100L) ? 0x04 : 0) \
| ((0x##x##L & 0x00001000L) ? 0x08 : 0) \
| ((0x##x##L & 0x00010000L) ? 0x10 : 0) \
| ((0x##x##L & 0x00100000L) ? 0x20 : 0) \
| ((0x##x##L & 0x01000000L) ? 0x40 : 0) \
| ((0x##x##L & 0x10000000L) ? 0x80 : 0))
*/

//ATT: On the Uno and other ATMEGA based boards, unsigned ints (unsigned integers) are the same as ints in that they store a 2 byte value.
//Long variables are extended size variables for number storage, and store 32 bits (4 bytes), from -2,147,483,648 to 2,147,483,647.

//*************************************************//
void setup() {
  pinMode(LCD_clk, OUTPUT);
  pinMode(LCD_in, OUTPUT);
  pinMode(LCD_CE, OUTPUT);

  pinMode(13, OUTPUT);
  
// initialize the LED pin as an output:
//pinMode(ledPin, OUTPUT);
// initialize the pushbutton pin as an input:
//pinMode(buttonPin, INPUT);  //Next line is the attach of interruption to pin 2
pinMode(BUTTON_PIN, INPUT_PULLUP);

 attachInterrupt(digitalPinToInterrupt(BUTTON_PIN),
                  buttonReleasedInterrupt,
                  FALLING);

//Dont insert any print inside of interrupt function!!!
//If you run the search function, please active the terminal to be possible print lines,
//Other way the run will be blocked!
//
  Serial.begin(115200);
  
  /*CS12  CS11 CS10 DESCRIPTION
  0        0     0  Timer/Counter1 Disabled 
  0        0     1  No Prescaling
  0        1     0  Clock / 8
  0        1     1  Clock / 64
  1        0     0  Clock / 256
  1        0     1  Clock / 1024
  1        1     0  External clock source on T1 pin, Clock on Falling edge
  1        1     1  External clock source on T1 pin, Clock on rising edge
 */
  
// Note: this counts is done to a Arduino 1 with Atmega 328... Is possible you need adjust
// a little the value 62499 upper or lower if the clock have a delay or advnce on hours.

  digitalWrite(LCD_CE, LOW);
  delayMicroseconds(5);
  digitalWrite(13, LOW);
  delay(500);
  digitalWrite(13, HIGH);
  delay(500);
  digitalWrite(13, LOW);
  delay(500);
  digitalWrite(13, HIGH);
  delay(500);
}
void send_char(unsigned char a){
 unsigned char transmit = 15; //define our transmit pin
 unsigned char data = 170; //value to transmit, binary 10101010
 unsigned char mask = 1; //our bitmask
  data=a;
  // the validation of data happen when clk go from LOW to HIGH.
  // This lines is because the clk have one advance in data, see datasheet of sn74HC595
  // case don't have this signal instead of "." will se "g"
  digitalWrite(LCD_CE, LOW); // When strobe is low, all output is enable. If high, all output will be set to low.
  delayMicroseconds(5);
  digitalWrite(LCD_clk,LOW);// need invert the signal to allow 8 bits is is low only send 7 bits
  delayMicroseconds(5);
  for (mask = 00000001; mask>0; mask <<= 1) { //iterate through bit mask
  digitalWrite(LCD_clk,LOW);// need invert the signal to allow 8 bits is is low only send 7 bits
  delayMicroseconds(5);
    if (data & mask){ // if bitwise AND resolves to true
      digitalWrite(LCD_in, HIGH);
      //Serial.print(1);
    }
    else{ //if bitwise and resolves to false
      digitalWrite(LCD_in, LOW);
      //Serial.print(0);
    }
    digitalWrite(LCD_clk,HIGH);// need invert the signal to allow 8 bits is is low only send 7 bits
    delayMicroseconds(5);
    //
    digitalWrite(LCD_CE, HIGH); // When strobe is low, all output is enable. If high, all output will be set to low.
  delayMicroseconds(5);
  }
}
// I h've created 3 functions to send bit's, one with strobe, other without strobe and one with first byte with strobe followed by remaing bits.
void send_char_without(unsigned char a){
 //
 unsigned char data = 0x00; //value to transmit, binary 10101010
 unsigned char mask = 1; //our bitmask
  data=a;
  //Serial.println(":");
  for (mask = 00000001; mask>0; mask <<= 1) { //iterate through bit mask
  digitalWrite(LCD_clk, LOW);
  delayMicroseconds(5);
    if (data & mask){ // if bitwise AND resolves to true
      digitalWrite(LCD_in, HIGH);
      //Serial.print(1);
    }
    else{ //if bitwise and resolves to false
      digitalWrite(LCD_in, LOW);
      //Serial.print(0);
    }
    digitalWrite(LCD_clk,HIGH);// need invert the signal to allow 8 bits is is low only send 7 bits
    delayMicroseconds(5);
  }
}
void send_char_8bit_stb(unsigned char a){
 unsigned char data = 0x00; //value to transmit, binary 10101010
 unsigned char mask = 1; //our bitmask
 int i = -1;
  data=a;
  //Serial.println(":");
  digitalWrite(LCD_CE, LOW);
  delayMicroseconds(1);
  // This lines is because the clk have one advance in data, see datasheet of sn74HC595
  // case don't have this signal instead of "." will se "g"
      for (mask = 00000001; mask>0; mask <<= 1) { //iterate through bit mask
       i++;
       digitalWrite(LCD_clk, LOW);
      delayMicroseconds(5);
        if (data & mask){ // if bitwise AND resolves to true
          digitalWrite(LCD_in, HIGH);
          //Serial.print(1);
        }
        else{ //if bitwise and resolves to false
          digitalWrite(LCD_in, LOW);
          //Serial.print(0);
        }
        digitalWrite(LCD_clk,HIGH);// need invert the signal to allow 8 bits is is low only send 7 bits
        delayMicroseconds(1);
            if (i==7){
            //Serial.println(i);
            digitalWrite(LCD_CE, HIGH);
            delayMicroseconds(2);
            }
      }
}
//
void backlightOff(){
  //Bit function: 0, FC0, FC1, FC2, P0, P1, P2, OC, SC, BU, DD, DD; //Set of LC75836
 for(int i=0; i<4;i++){   // 
      digitalWrite(LCD_CE, LOW); //
      delayMicroseconds(1);
      send_char_8bit_stb(0B01000110); //(0x46) firts 8 bits is address, every fixed as (0B01000010), see if clk finish LOW or HIGH Very important!
      delayMicroseconds(1);
      // On the 75836 the message have first 16*8 bits more 8 times to performe 128 bits(last byte is control: 0BXXX00000)
      // We define the P1 as 1, this set P1 & P2 as GPIO pin! The pin 1 & 2 will be used to control backight and dimmer (bit 1 and 5)
          send_char_without(0B00010000);  send_char_without(0B11111111);  //   8:1     -16:9// 
          send_char_without(0B11111111);  send_char_without(0B11111111);  //  24:17    -32:25// 
          send_char_without(0B11111111);  //  40:33    //the next switch send reamaining bits -41:48// 
              switch (i){ //Last 3 bits is "DD" data direction, and is used
                case 0: send_char_without(0B00000010); break;  //The bit P1 will set the segments start on S2 (pin 3 of LC75836)
                case 1: send_char_without(0B10000000); break;
                case 2: send_char_without(0B01000000); break;
                case 3: send_char_without(0B11000000); break;
              }
      // to mark the 4 groups of 36 bits, 00, 01, 10, 11.
      delayMicroseconds(1);
      digitalWrite(LCD_CE, LOW); // 
      }
}
void backlightOn(){
  //Bit function: 0, FC0, FC1, FC2, P0, P1, P2, OC, SC, BU, DD, DD; //Set of LC75836
 for(int i=0; i<4;i++){   // 
      digitalWrite(LCD_CE, LOW); //
      delayMicroseconds(1);
      send_char_8bit_stb(0B01000110); //(0x46) firts 8 bits is address, every fixed as (0B01000010), see if clk finish LOW or HIGH Very important!
      delayMicroseconds(1);
      // On the 75836 the message have first 16*8 bits more 8 times to performe 128 bits(last byte is control: 0BXXX00000)
      // We define the P1 as 1, this set P1 & P2 as GPIO pin! The pin 1 & 2 will be used to control backight and dimmer (bit 1 and 5)
          send_char_without(0B00010001);  send_char_without(0B11111111);  //   8:1     -16:9// 
          send_char_without(0B11111111);  send_char_without(0B11111111);  //  24:17    -32:25// 
          send_char_without(0B11111111);  //  40:33    //the next switch send reamaining bits -41:48// 
              switch (i){ //Last 3 bits is "DD" data direction, and is used
                case 0: send_char_without(0B00000010); break;  //The bit P1 will set the segments start on S2 (pin 3 of LC75836)
                case 1: send_char_without(0B10000000); break;
                case 2: send_char_without(0B01000000); break;
                case 3: send_char_without(0B11000000); break;
              }
      // to mark the 4 groups of 36 bits, 00, 01, 10, 11.
      delayMicroseconds(1);
      digitalWrite(LCD_CE, LOW); // 
      }
}
void backlightDimmer(){
  //Bit function: 0, FC0, FC1, FC2, P0, P1, P2, OC, SC, BU, DD, DD; //Set of LC75836
 for(int i=0; i<4;i++){   // 
      digitalWrite(LCD_CE, LOW); //
      delayMicroseconds(1);
      send_char_8bit_stb(0B01000110); //(0x46) firts 8 bits is address, every fixed as (0B01000010), see if clk finish LOW or HIGH Very important!
      delayMicroseconds(1);
      // On the 75836 the message have first 16*8 bits more 8 times to performe 128 bits(last byte is control: 0BXXX00000)
      // We define the P1 as 1, this set P1 & P2 as GPIO pin! The pin 1 & 2 will be used to control backight and dimmer (bit 1 and 5)
          send_char_without(0B00000001);  send_char_without(0B11111111);  //   8:1     -16:9// 
          send_char_without(0B11111111);  send_char_without(0B11111111);  //  24:17    -32:25// 
          send_char_without(0B11111111);  //  40:33    //the next switch send reamaining bits -41:48// 
              switch (i){ //Last 3 bits is "DD" data direction, and is used
                case 0: send_char_without(0B00000010); break;  //The bit P1 will set the segments start on S2 (pin 3 of LC75836)
                case 1: send_char_without(0B10000000); break;
                case 2: send_char_without(0B01000000); break;
                case 3: send_char_without(0B11000000); break;
              }
      // to mark the 4 groups of 36 bits, 00, 01, 10, 11.
      delayMicroseconds(1);
      digitalWrite(LCD_CE, LOW); // 
      }
}
void allON(){
  //Bit function: 0, FC0, FC1, FC2, P0, P1, P2, OC, SC, BU, DD, DD; //Set of LC75836
 for(int i=0; i<4;i++){   // 
      digitalWrite(LCD_CE, LOW); //
      delayMicroseconds(1);
      send_char_8bit_stb(0B01000110); //(0x46) firts 8 bits is address, every fixed as (0B01000010), see if clk finish LOW or HIGH Very important!
      delayMicroseconds(1);
      // On the 75836 the message have first 16*8 bits more 8 times to performe 128 bits(last byte is control: 0BXXX00000)
          
          send_char_without(0B11111111);  send_char_without(0B11111111);  //   8:1     -16:9// 
          send_char_without(0B11111111);  send_char_without(0B11111111);  //  24:17    -32:25// 
          send_char_without(0B11111111);  //  40:33    //the next switch send reamaining bits -41:48// 
              switch (i){ //Last 3 bits is "DD" data direction, and is used
                case 0: send_char_without(0B00000010); break; //This is activation P1 to start segments on S2
                case 1: send_char_without(0B10000000); break;
                case 2: send_char_without(0B01000000); break;
                case 3: send_char_without(0B11000000); break;
              }
      // to mark the 4 groups of 36 bits, 00, 01, 10, 11.
      delayMicroseconds(1);
      digitalWrite(LCD_CE, LOW); // 
      }
}
void allOFF(){
  //Bit function: 0, FC0, FC1, FC2, P0, P1, P2, OC, SC, BU, DD, DD;
 for(int i=0; i<4;i++){   // 
      digitalWrite(LCD_CE, LOW); //
      delayMicroseconds(1);
      send_char_8bit_stb(0B01000110); //(0x46) firts 8 bits is address, every fixed as (0B01000010), see if clk finish LOW or HIGH Very important!
      delayMicroseconds(1);
      // On the 75836 the message have first 16*8 bits more 8 times to performe 128 bits(last byte is control: 0BXXX00000)
          send_char_without(0B00000000);  send_char_without(0B00000000);  //   8:1      -16:9// 
          send_char_without(0B00000000);  send_char_without(0B00000000);  //  24:17    -32:25// 
          send_char_without(0B00000000);    //  40:33   //the next switch send reamaining bits -41:48//  
              switch (i){ //Last 3 bits is "DD" data direction, and is used
                case 0: send_char_without(0B00000010); break; //This is activation P1 to start segments on S2
                case 1: send_char_without(0B10000000); break;
                case 2: send_char_without(0B01000000); break;
                case 3: send_char_without(0B11000000); break;
              }
      // to mark the 4 groups of 36 bits, 00, 01, 10, 11.
      delayMicroseconds(1);
      digitalWrite(LCD_CE, LOW); // 
  }
}
void searchOfSegments(){
  int group = 0x00;
  byte nBit =0x00;
  byte nMask = 0b00000001;
  unsigned int block =0;
  byte nSeg=0x00;
  Serial.println();
  Serial.println("We start the test of segments!");
  for(block=0; block<4; block++){  //This is the last 2 bit's marked as DD, group: 0x00, 0x01, 0x10, 0x11;
      for( group=0; group<5; group++){   // Do until n bits 5*36 bits
          for (nMask = 0b00000001; nMask>0; nMask <<= 1){
            Aa=0x00; Ab=0x00; Ac=0x00; Ad=0x00; Ae=0x00;
                  switch (group){
                    case 0: Aa=nMask; break;
                    case 1: Ab=nMask; break;
                    case 2: Ac=nMask; break;
                    case 3: Ad=nMask; break;
                    case 4: Ae=nMask; break;
                  }
           nSeg++;   //This point is where we incremente the segment number! Please check datasheet of driver to get number of bit by block.
           if((nSeg >=0) && (nSeg<41)){
            blockBit=0;
            }
            if((nSeg >=41) && (nSeg<80)){
            blockBit=1;
            }
            if((nSeg >=81) && (nSeg<120)){
            blockBit=2;
            }
            if((nSeg >=121) && (nSeg<160)){
            blockBit=3;
            }
            if (nSeg >160){
              nSeg=0;
              group=0;
              block=0;
              break;
            }
      //This start the control of button to allow continue teste! 
                      while(1){
                            if(!buttonReleased){
                              delay(200);
                            }
                            else{
                              delay(15);
                               buttonReleased = false;
                               break;
                               }
                      }
            segments();
            Serial.print(nSeg, DEC); Serial.print(", group: "); Serial.print(group, DEC);Serial.print(", BlockBit: "); Serial.print(blockBit, HEX);Serial.print(", nMask: "); Serial.print(nMask, BIN);Serial.print("   \t");
            Serial.print(Ae, HEX);Serial.print(", ");Serial.print(Ad, HEX);Serial.print(", ");Serial.print(Ac, HEX);Serial.print(", ");Serial.print(Ab, HEX);Serial.print(", ");Serial.print(Aa, HEX); Serial.print("; ");
            Serial.println();
            delay (400);  
                }         
           }        
      }
}
void segments(){
  //Bit function: 0, FC0, FC1, FC2, P0, P1, P2, OC, SC, BU, DD, DD;
      digitalWrite(LCD_CE, LOW); //
      delayMicroseconds(1);
      send_char_8bit_stb(0B01000110); //(0x46) firts 8 bits is address, every fixed as (0B01000010), see if clk finish LOW or HIGH Very important!
      delayMicroseconds(1);
      // On the 75836 the message have first 5*8 bits (last byte is control Bit function: 0, FC0, FC1, FC2, P0, P1, P2, DR, SC, BU, DD, DD;)
          
          //The next switch finalize the burst of bits -41:48//  
              switch (blockBit){ //Last 2 bits is "DD" data direction, and is used to mark the 4 groups of 36 bits, 00, 01, 10, 11.                                 
                case 0: send_char_without(Aa | 0B00010001);  send_char_without(~(Ab));  //   1:8      -9:16// //Need activate bit 5 and 1 to control backight!
                        send_char_without(~(Ac));  send_char_without(~(Ad));  //  17:24    -25:32// 
                        send_char_without(~(Ae));  //  33:40    
                        send_char_without(0B00000010); break; //This is activation P1 to start segments on S2 //Block 00
                case 1: send_char_without(~(Aa));  send_char_without(~(Ab));  //   1:8      -9:16// //Need activate bit 5 and 1 to control backight!
                        send_char_without(~(Ac));  send_char_without(~(Ad));  //  17:24    -25:32// 
                        send_char_without(~(Ae));  //  33:40    
                        send_char_without(0B10000000); break; //Block 01
                case 2: send_char_without(~(Aa));  send_char_without(~(Ab));  //   1:8      -9:16// //Need activate bit 5 and 1 to control backight!
                        send_char_without(~(Ac));  send_char_without(~(Ad));  //  17:24    -25:32// 
                        send_char_without(~(Ae));  //  33:40 
                        send_char_without(0B01000000); break; //Block 10
                case 3: send_char_without(~(Aa));  send_char_without(~(Ab));  //   1:8      -9:16// //Need activate bit 5 and 1 to control backight!
                        send_char_without(~(Ac));  send_char_without(~(Ad));  //  17:24    -25:32// 
                        send_char_without(~(Ae));  //  33:40 
                        send_char_without(0B11000000); break; //Block 11
              }
      delayMicroseconds(1);
      digitalWrite(LCD_CE, LOW); //                   
}
//
void seg01_40__F(){
  digitalWrite(LCD_CE, LOW); //
      delayMicroseconds(1);
      send_char_8bit_stb(0B01000110); //(0x46) firts 8 bits is address, every fixed as (0B01000010), see if clk finish LOW or HIGH Very important!
      delayMicroseconds(1);
      // On the 75836 the message have first 16*8 bits more 8 times to performe 128 bits(last byte is control: 0BXXX00000)
          send_char_without(0B00010001);  send_char_without(0B00000100);  //   8:1     -16:9  // 
          send_char_without(0B00011011);  send_char_without(0B00000000);  //  24:17    -32:25 // 
          send_char_without(0B00000000);    //  40:33  36 until 40 must be 0, they are: (0, FC0, FC1, FC2) 
          //the next line send reamaining control bits -48:41//  
          send_char_without(0B00000000);
      // to mark the 4 groups of 36 bits, 00, 01, 10, 11.
      delayMicroseconds(1);
      digitalWrite(LCD_CE, LOW); // 
}
void seg41_80_HI(){
      digitalWrite(LCD_CE, LOW); //
      delayMicroseconds(1);
      send_char_8bit_stb(0B01000110); //(0x46) firts 8 bits is address, every fixed as (0B01000010), see if clk finish LOW or HIGH Very important!
      delayMicroseconds(1);
      // On the 75836 the message have first 5*8 bits more 1 byte control
          send_char_without(0B01000000);  send_char_without(0B00001001);  //  48:41   -56:49 // 
          send_char_without(0B00001110);  send_char_without(0B11100000);  //  64:57   -72:65 // 
          send_char_without(0B00000001);    //  80:73   
          //the next line send reamaining bits of control//  
          send_char_without(0B10000000);
      // to mark the 4 groups of 36 bits, 00, 01, 10, 11.
      delayMicroseconds(1);
      digitalWrite(LCD_CE, LOW); // 
}
void seg81_120_OL(){
  digitalWrite(LCD_CE, LOW); //
      delayMicroseconds(1);
      send_char_8bit_stb(0B01000110); //(0x46) firts 8 bits is address, every fixed as (0B01000010), see if clk finish LOW or HIGH Very important!
      delayMicroseconds(1);
      // On the 75836 the message have first 16*8 bits more 8 times to performe 128 bits(last byte is control: 0BXXX00000)
          send_char_without(0B11000000);  send_char_without(0B10010000);  //  88:81     -96:89  // 
          send_char_without(0B00000110);  send_char_without(0B00001010);  // 104:97    -112:105 // 
          send_char_without(0B00001110);    //  120:113 //  
          //the next line send reamaining bits of cntrol//  
          send_char_without(0B01000000);
      // to mark the 4 groups of 36 bits, 00, 01, 10, 11.
      delayMicroseconds(1);
      digitalWrite(LCD_CE, LOW); // 
}
void seg121_160_KS(){
  digitalWrite(LCD_CE, LOW); //
      delayMicroseconds(1);
      send_char_8bit_stb(0B01000110); //(0x46) firts 8 bits is address, every fixed as (0B01000010), see if clk finish LOW or HIGH Very important!
      delayMicroseconds(1);
      // On the 75836 the message have first 16*8 bits more 8 times to performe 128 bits(last byte is control: 0BXXX00000)
          send_char_without(0B00000000);  send_char_without(0B10101000);  //  128:121    -136:129// 
          send_char_without(0B00001010);  send_char_without(0B10101001);  //  144:137    -152:145// 
          send_char_without(0B00000000);    //  160:153   
          //the next line send reamaining bits of control//  
          send_char_without(0B11000000);
      // to mark the 4 groups of 36 bits, 00, 01, 10, 11.
      delayMicroseconds(1);
      digitalWrite(LCD_CE, LOW); // 
}
//
void seg01_40_anim0(){
  digitalWrite(LCD_CE, LOW); //
      delayMicroseconds(1);
      send_char_8bit_stb(0B01000110); //(0x46) firts 8 bits is address, every fixed as (0B01000010), see if clk finish LOW or HIGH Very important!
      delayMicroseconds(1);
      // On the 75836 the message have first 16*8 bits more 8 times to performe 128 bits(last byte is control: 0BXXX00000)
          send_char_without(0B00010001);  send_char_without(0B00000100);  //   8:1     -16:9  // 
          send_char_without(0B00011011);  send_char_without(0B00000000);  //  24:17    -32:25 // 
          send_char_without(0B00000000);    //  40:33  36 until 40 must be 0, they are: (0, FC0, FC1, FC2) 
          //the next line send reamaining control bits -48:41//  
          send_char_without(0B00000000);
      // to mark the 4 groups of 36 bits, 00, 01, 10, 11.
      delayMicroseconds(1);
      digitalWrite(LCD_CE, LOW); // 
}
void seg41_80_anim1(){
      digitalWrite(LCD_CE, LOW); //
      delayMicroseconds(1);
      send_char_8bit_stb(0B01000110); //(0x46) firts 8 bits is address, every fixed as (0B01000010), see if clk finish LOW or HIGH Very important!
      delayMicroseconds(1);
      // On the 75836 the message have first 5*8 bits more 1 byte control
          send_char_without(0B01000000);  send_char_without(0B00001001);  //  48:41   -56:49 // 
          send_char_without(0B00001110);  send_char_without(0B11100000);  //  64:57   -72:65 // 
          send_char_without(0B00000001);    //  80:73   
          //the next line send reamaining bits of control//  
          send_char_without(0B10000000);
      // to mark the 4 groups of 36 bits, 00, 01, 10, 11.
      delayMicroseconds(1);
      digitalWrite(LCD_CE, LOW); // 
}
void seg81_120_anim2(){
  digitalWrite(LCD_CE, LOW); //
      delayMicroseconds(1);
      send_char_8bit_stb(0B01000110); //(0x46) firts 8 bits is address, every fixed as (0B01000010), see if clk finish LOW or HIGH Very important!
      delayMicroseconds(1);
      // On the 75836 the message have first 16*8 bits more 8 times to performe 128 bits(last byte is control: 0BXXX00000)
          send_char_without(0B11000000);  send_char_without(0B10010000);  //  88:81     -96:89  // 
          send_char_without(0B00000110);  send_char_without(0B00001010);  // 104:97    -112:105 // 
          send_char_without(0B00001110);    //  120:113 //  
          //the next line send reamaining bits of cntrol//  
          send_char_without(0B01000000);
      // to mark the 4 groups of 36 bits, 00, 01, 10, 11.
      delayMicroseconds(1);
      digitalWrite(LCD_CE, LOW); // 
}
void seg121_160_anim3(){
  digitalWrite(LCD_CE, LOW); //
      delayMicroseconds(1);
      send_char_8bit_stb(0B01000110); //(0x46) firts 8 bits is address, every fixed as (0B01000010), see if clk finish LOW or HIGH Very important!
      delayMicroseconds(1);
      // On the 75836 the message have first 16*8 bits more 8 times to performe 128 bits(last byte is control: 0BXXX00000)
          send_char_without(0B00000000);  send_char_without(0B10101000);  //  128:121    -136:129// 
          send_char_without(0B00001010);  send_char_without(0B10101001);  //  144:137    -152:145// 
          send_char_without(0B00000000);    //  160:153   
          //the next line send reamaining bits of control//  
          send_char_without(0B11000000);
      // to mark the 4 groups of 36 bits, 00, 01, 10, 11.
      delayMicroseconds(1);
      digitalWrite(LCD_CE, LOW); // 
}
//
void seg01_40_animA0(){
  digitalWrite(LCD_CE, LOW); //
      delayMicroseconds(1);
      send_char_8bit_stb(0B01000110); //(0x46) firts 8 bits is address, every fixed as (0B01000010), see if clk finish LOW or HIGH Very important!
      delayMicroseconds(1);
      // On the 75836 the message have first 16*8 bits more 8 times to performe 128 bits(last byte is control: 0BXXX00000)
          send_char_without(0B00010001);  send_char_without(0B00000100);  //   8:1     -16:9  // 
          send_char_without(0B00011011);  send_char_without(0B00000000);  //  24:17    -32:25 // 
          send_char_without(0B00000000);    //  40:33  36 until 40 must be 0, they are: (0, FC0, FC1, FC2) 
          //the next line send reamaining control bits -48:41//  
          send_char_without(0B00000000);
      // to mark the 4 groups of 36 bits, 00, 01, 10, 11.
      delayMicroseconds(1);
      digitalWrite(LCD_CE, LOW); // 
}
void seg41_80_animB0(){
      digitalWrite(LCD_CE, LOW); //
      delayMicroseconds(1);
      send_char_8bit_stb(0B01000110); //(0x46) firts 8 bits is address, every fixed as (0B01000010), see if clk finish LOW or HIGH Very important!
      delayMicroseconds(1);
      // On the 75836 the message have first 5*8 bits more 1 byte control
          send_char_without(0B01000000);  send_char_without(0B00001001);  //  48:41   -56:49 // 
          send_char_without(0B00001110);  send_char_without(0B11100000);  //  64:57   -72:65 // 
          send_char_without(0B00000000);    //  80:73   
          //the next line send reamaining bits of control//  
          send_char_without(0B10000000);
      // to mark the 4 groups of 36 bits, 00, 01, 10, 11.
      delayMicroseconds(1);
      digitalWrite(LCD_CE, LOW); // 
}
void seg81_120_animC0(){
  digitalWrite(LCD_CE, LOW); //
      delayMicroseconds(1);
      send_char_8bit_stb(0B01000110); //(0x46) firts 8 bits is address, every fixed as (0B01000010), see if clk finish LOW or HIGH Very important!
      delayMicroseconds(1);
      // On the 75836 the message have first 16*8 bits more 8 times to performe 128 bits(last byte is control: 0BXXX00000)
          send_char_without(0B11000000);  send_char_without(0B10010000);  //  88:81     -96:89  // 
          send_char_without(0B00000110);  send_char_without(0B00001010);  // 104:97    -112:105 // 
          send_char_without(0B00001110);    //  120:113 //  
          //the next line send reamaining bits of cntrol//  
          send_char_without(0B01000000);
      // to mark the 4 groups of 36 bits, 00, 01, 10, 11.
      delayMicroseconds(1);
      digitalWrite(LCD_CE, LOW); // 
}
void seg121_160_animD0(){
  digitalWrite(LCD_CE, LOW); //
      delayMicroseconds(1);
      send_char_8bit_stb(0B01000110); //(0x46) firts 8 bits is address, every fixed as (0B01000010), see if clk finish LOW or HIGH Very important!
      delayMicroseconds(1);
      // On the 75836 the message have first 16*8 bits more 8 times to performe 128 bits(last byte is control: 0BXXX00000)
          send_char_without(0B00000000);  send_char_without(0B10101000);  //  128:121    -136:129// 
          send_char_without(0B00001010);  send_char_without(0B10101001);  //  144:137    -152:145// 
          send_char_without(0B00000000);    //  160:153   
          //the next line send reamaining bits of control//  
          send_char_without(0B11000000);
      // to mark the 4 groups of 36 bits, 00, 01, 10, 11.
      delayMicroseconds(1);
      digitalWrite(LCD_CE, LOW); // 
}
//
void seg01_40_animA1(){
  digitalWrite(LCD_CE, LOW); //
      delayMicroseconds(1);
      send_char_8bit_stb(0B01000110); //(0x46) firts 8 bits is address, every fixed as (0B01000010), see if clk finish LOW or HIGH Very important!
      delayMicroseconds(1);
      // On the 75836 the message have first 16*8 bits more 8 times to performe 128 bits(last byte is control: 0BXXX00000)
          send_char_without(0B00010001);  send_char_without(0B00000100);  //   8:1     -16:9  // 
          send_char_without(0B00011011);  send_char_without(0B00000000);  //  24:17    -32:25 // 
          send_char_without(0B00000000);    //  40:33  36 until 40 must be 0, they are: (0, FC0, FC1, FC2) 
          //the next line send reamaining control bits -48:41//  
          send_char_without(0B00000000);
      // to mark the 4 groups of 36 bits, 00, 01, 10, 11.
      delayMicroseconds(1);
      digitalWrite(LCD_CE, LOW); // 
}
void seg41_80_animB1(){
      digitalWrite(LCD_CE, LOW); //
      delayMicroseconds(1);
      send_char_8bit_stb(0B01000110); //(0x46) firts 8 bits is address, every fixed as (0B01000010), see if clk finish LOW or HIGH Very important!
      delayMicroseconds(1);
      // On the 75836 the message have first 5*8 bits more 1 byte control
          send_char_without(0B01000000);  send_char_without(0B00001001);  //  48:41   -56:49 // 
          send_char_without(0B00001110);  send_char_without(0B11100000);  //  64:57   -72:65 // 
          send_char_without(0B00000001);    //  80:73   
          //the next line send reamaining bits of control//  
          send_char_without(0B10000000);
      // to mark the 4 groups of 36 bits, 00, 01, 10, 11.
      delayMicroseconds(1);
      digitalWrite(LCD_CE, LOW); // 
}
void seg81_120_animC1(){
  digitalWrite(LCD_CE, LOW); //
      delayMicroseconds(1);
      send_char_8bit_stb(0B01000110); //(0x46) firts 8 bits is address, every fixed as (0B01000010), see if clk finish LOW or HIGH Very important!
      delayMicroseconds(1);
      // On the 75836 the message have first 16*8 bits more 8 times to performe 128 bits(last byte is control: 0BXXX00000)
          send_char_without(0B11000000);  send_char_without(0B10010000);  //  88:81     -96:89  // 
          send_char_without(0B00000110);  send_char_without(0B00001010);  // 104:97    -112:105 // 
          send_char_without(0B00001110);    //  120:113 //  
          //the next line send reamaining bits of cntrol//  
          send_char_without(0B01000000);
      // to mark the 4 groups of 36 bits, 00, 01, 10, 11.
      delayMicroseconds(1);
      digitalWrite(LCD_CE, LOW); // 
}
void seg121_160_animD1(){
  digitalWrite(LCD_CE, LOW); //
      delayMicroseconds(1);
      send_char_8bit_stb(0B01000110); //(0x46) firts 8 bits is address, every fixed as (0B01000010), see if clk finish LOW or HIGH Very important!
      delayMicroseconds(1);
      // On the 75836 the message have first 16*8 bits more 8 times to performe 128 bits(last byte is control: 0BXXX00000)
          send_char_without(0B00000000);  send_char_without(0B10101000);  //  128:121    -136:129// 
          send_char_without(0B00001010);  send_char_without(0B10101001);  //  144:137    -152:145// 
          send_char_without(0B00000000);    //  160:153   
          //the next line send reamaining bits of control//  
          send_char_without(0B11000000);
      // to mark the 4 groups of 36 bits, 00, 01, 10, 11.
      delayMicroseconds(1);
      digitalWrite(LCD_CE, LOW); // 
}
//
void seg01_40_animA2(){
  digitalWrite(LCD_CE, LOW); //
      delayMicroseconds(1);
      send_char_8bit_stb(0B01000110); //(0x46) firts 8 bits is address, every fixed as (0B01000010), see if clk finish LOW or HIGH Very important!
      delayMicroseconds(1);
      // On the 75836 the message have first 16*8 bits more 8 times to performe 128 bits(last byte is control: 0BXXX00000)
          send_char_without(0B00010001);  send_char_without(0B00000100);  //   8:1     -16:9  // 
          send_char_without(0B00011011);  send_char_without(0B00000000);  //  24:17    -32:25 // 
          send_char_without(0B00000000);    //  40:33  36 until 40 must be 0, they are: (0, FC0, FC1, FC2) 
          //the next line send reamaining control bits -48:41//  
          send_char_without(0B00000000);
      // to mark the 4 groups of 36 bits, 00, 01, 10, 11.
      delayMicroseconds(1);
      digitalWrite(LCD_CE, LOW); // 
}
void seg41_80_animB2(){
      digitalWrite(LCD_CE, LOW); //
      delayMicroseconds(1);
      send_char_8bit_stb(0B01000110); //(0x46) firts 8 bits is address, every fixed as (0B01000010), see if clk finish LOW or HIGH Very important!
      delayMicroseconds(1);
      // On the 75836 the message have first 5*8 bits more 1 byte control
          send_char_without(0B01000000);  send_char_without(0B00001001);  //  48:41   -56:49 // 
          send_char_without(0B00001110);  send_char_without(0B11100000);  //  64:57   -72:65 // 
          send_char_without(0B00001101);    //  80:73   
          //the next line send reamaining bits of control//  
          send_char_without(0B10000000);
      // to mark the 4 groups of 36 bits, 00, 01, 10, 11.
      delayMicroseconds(1);
      digitalWrite(LCD_CE, LOW); // 
}
void seg81_120_animC2(){
  digitalWrite(LCD_CE, LOW); //
      delayMicroseconds(1);
      send_char_8bit_stb(0B01000110); //(0x46) firts 8 bits is address, every fixed as (0B01000010), see if clk finish LOW or HIGH Very important!
      delayMicroseconds(1);
      // On the 75836 the message have first 16*8 bits more 8 times to performe 128 bits(last byte is control: 0BXXX00000)
          send_char_without(0B11001000);  send_char_without(0B10010000);  //  88:81     -96:89  // 
          send_char_without(0B00000110);  send_char_without(0B00001010);  // 104:97    -112:105 // 
          send_char_without(0B00001110);    //  120:113 //  
          //the next line send reamaining bits of cntrol//  
          send_char_without(0B01000000);
      // to mark the 4 groups of 36 bits, 00, 01, 10, 11.
      delayMicroseconds(1);
      digitalWrite(LCD_CE, LOW); // 
}
void seg121_160_animD2(){
  digitalWrite(LCD_CE, LOW); //
      delayMicroseconds(1);
      send_char_8bit_stb(0B01000110); //(0x46) firts 8 bits is address, every fixed as (0B01000010), see if clk finish LOW or HIGH Very important!
      delayMicroseconds(1);
      // On the 75836 the message have first 16*8 bits more 8 times to performe 128 bits(last byte is control: 0BXXX00000)
          send_char_without(0B00000000);  send_char_without(0B10101000);  //  128:121    -136:129// 
          send_char_without(0B00001010);  send_char_without(0B10101001);  //  144:137    -152:145// 
          send_char_without(0B00000000);    //  160:153   
          //the next line send reamaining bits of control//  
          send_char_without(0B11000000);
      // to mark the 4 groups of 36 bits, 00, 01, 10, 11.
      delayMicroseconds(1);
      digitalWrite(LCD_CE, LOW); // 
}
//
void seg01_40_animA3(){
  digitalWrite(LCD_CE, LOW); //
      delayMicroseconds(1);
      send_char_8bit_stb(0B01000110); //(0x46) firts 8 bits is address, every fixed as (0B01000010), see if clk finish LOW or HIGH Very important!
      delayMicroseconds(1);
      // On the 75836 the message have first 16*8 bits more 8 times to performe 128 bits(last byte is control: 0BXXX00000)
          send_char_without(0B00010001);  send_char_without(0B00000100);  //   8:1     -16:9  // 
          send_char_without(0B00011011);  send_char_without(0B00000000);  //  24:17    -32:25 // 
          send_char_without(0B00000000);    //  40:33  36 until 40 must be 0, they are: (0, FC0, FC1, FC2) 
          //the next line send reamaining control bits -48:41//  
          send_char_without(0B00000000);
      // to mark the 4 groups of 36 bits, 00, 01, 10, 11.
      delayMicroseconds(1);
      digitalWrite(LCD_CE, LOW); // 
}
void seg41_80_animB3(){
      digitalWrite(LCD_CE, LOW); //
      delayMicroseconds(1);
      send_char_8bit_stb(0B01000110); //(0x46) firts 8 bits is address, every fixed as (0B01000010), see if clk finish LOW or HIGH Very important!
      delayMicroseconds(1);
      // On the 75836 the message have first 5*8 bits more 1 byte control
          send_char_without(0B01000000);  send_char_without(0B00001001);  //  48:41   -56:49 // 
          send_char_without(0B00001110);  send_char_without(0B11100000);  //  64:57   -72:65 // 
          send_char_without(0B00000010);    //  80:73   
          //the next line send reamaining bits of control//  
          send_char_without(0B10000000);
      // to mark the 4 groups of 36 bits, 00, 01, 10, 11.
      delayMicroseconds(1);
      digitalWrite(LCD_CE, LOW); // 
}
void seg81_120_animC3(){
  digitalWrite(LCD_CE, LOW); //
      delayMicroseconds(1);
      send_char_8bit_stb(0B01000110); //(0x46) firts 8 bits is address, every fixed as (0B01000010), see if clk finish LOW or HIGH Very important!
      delayMicroseconds(1);
      // On the 75836 the message have first 16*8 bits more 8 times to performe 128 bits(last byte is control: 0BXXX00000)
          send_char_without(0B11000110);  send_char_without(0B10010000);  //  88:81     -96:89  // 
          send_char_without(0B00000110);  send_char_without(0B00001010);  // 104:97    -112:105 // 
          send_char_without(0B00001110);    //  120:113 //  
          //the next line send reamaining bits of cntrol//  
          send_char_without(0B01000000);
      // to mark the 4 groups of 36 bits, 00, 01, 10, 11.
      delayMicroseconds(1);
      digitalWrite(LCD_CE, LOW); // 
}
void seg121_160_animD3(){
  digitalWrite(LCD_CE, LOW); //
      delayMicroseconds(1);
      send_char_8bit_stb(0B01000110); //(0x46) firts 8 bits is address, every fixed as (0B01000010), see if clk finish LOW or HIGH Very important!
      delayMicroseconds(1);
      // On the 75836 the message have first 16*8 bits more 8 times to performe 128 bits(last byte is control: 0BXXX00000)
          send_char_without(0B00000000);  send_char_without(0B10101000);  //  128:121    -136:129// 
          send_char_without(0B00001010);  send_char_without(0B10101001);  //  144:137    -152:145// 
          send_char_without(0B00000000);    //  160:153   
          //the next line send reamaining bits of control//  
          send_char_without(0B11000000);
      // to mark the 4 groups of 36 bits, 00, 01, 10, 11.
      delayMicroseconds(1);
      digitalWrite(LCD_CE, LOW); // 
}
//
void seg01_40_animA4(){
  digitalWrite(LCD_CE, LOW); //
      delayMicroseconds(1);
      send_char_8bit_stb(0B01000110); //(0x46) firts 8 bits is address, every fixed as (0B01000010), see if clk finish LOW or HIGH Very important!
      delayMicroseconds(1);
      // On the 75836 the message have first 16*8 bits more 8 times to performe 128 bits(last byte is control: 0BXXX00000)
          send_char_without(0B00010001);  send_char_without(0B00000100);  //   8:1     -16:9  // 
          send_char_without(0B00011011);  send_char_without(0B00000000);  //  24:17    -32:25 // 
          send_char_without(0B00000000);    //  40:33  36 until 40 must be 0, they are: (0, FC0, FC1, FC2) 
          //the next line send reamaining control bits -48:41//  
          send_char_without(0B00000000);
      // to mark the 4 groups of 36 bits, 00, 01, 10, 11.
      delayMicroseconds(1);
      digitalWrite(LCD_CE, LOW); // 
}
void seg41_80_animB4(){
      digitalWrite(LCD_CE, LOW); //
      delayMicroseconds(1);
      send_char_8bit_stb(0B01000110); //(0x46) firts 8 bits is address, every fixed as (0B01000010), see if clk finish LOW or HIGH Very important!
      delayMicroseconds(1);
      // On the 75836 the message have first 5*8 bits more 1 byte control
          send_char_without(0B01000000);  send_char_without(0B00001001);  //  48:41   -56:49 // 
          send_char_without(0B00001110);  send_char_without(0B11100000);  //  64:57   -72:65 // 
          send_char_without(0B00000000);    //  80:73   
          //the next line send reamaining bits of control//  
          send_char_without(0B10000000);
      // to mark the 4 groups of 36 bits, 00, 01, 10, 11.
      delayMicroseconds(1);
      digitalWrite(LCD_CE, LOW); // 
}
void seg81_120_animC4(){
  digitalWrite(LCD_CE, LOW); //
      delayMicroseconds(1);
      send_char_8bit_stb(0B01000110); //(0x46) firts 8 bits is address, every fixed as (0B01000010), see if clk finish LOW or HIGH Very important!
      delayMicroseconds(1);
      // On the 75836 the message have first 16*8 bits more 8 times to performe 128 bits(last byte is control: 0BXXX00000)
          send_char_without(0B11010001);  send_char_without(0B10010001);  //  88:81     -96:89  // 
          send_char_without(0B00000110);  send_char_without(0B00001010);  // 104:97    -112:105 // 
          send_char_without(0B00001110);    //  120:113 //  
          //the next line send reamaining bits of cntrol//  
          send_char_without(0B01000000);
      // to mark the 4 groups of 36 bits, 00, 01, 10, 11.
      delayMicroseconds(1);
      digitalWrite(LCD_CE, LOW); // 
}
void seg121_160_animD4(){
  digitalWrite(LCD_CE, LOW); //
      delayMicroseconds(1);
      send_char_8bit_stb(0B01000110); //(0x46) firts 8 bits is address, every fixed as (0B01000010), see if clk finish LOW or HIGH Very important!
      delayMicroseconds(1);
      // On the 75836 the message have first 16*8 bits more 8 times to performe 128 bits(last byte is control: 0BXXX00000)
          send_char_without(0B00000000);  send_char_without(0B10101000);  //  128:121    -136:129// 
          send_char_without(0B00001010);  send_char_without(0B10101001);  //  144:137    -152:145// 
          send_char_without(0B00000000);    //  160:153   
          //the next line send reamaining bits of control//  
          send_char_without(0B11000000);
      // to mark the 4 groups of 36 bits, 00, 01, 10, 11.
      delayMicroseconds(1);
      digitalWrite(LCD_CE, LOW); // 
}
//
void anime(void){
  for(uint8_t i = 0; i < 12; i++){
  seg01_40_animA0();
  seg41_80_animB0();
  seg81_120_animC0();
  seg121_160_animD0();
    delay(250);
  seg01_40_animA1();
  seg41_80_animB1();
  seg81_120_animC1();
  seg121_160_animD1();
    delay(175);
  seg01_40_animA2();
  seg41_80_animB2();
  seg81_120_animC2();
  seg121_160_animD2();
    delay(100);
  seg01_40_animA3();
  seg41_80_animB3();
  seg81_120_animC3();
  seg121_160_animD3();
    delay(80);
  seg01_40_animA4();
  seg41_80_animB4();
  seg81_120_animC4();
  seg121_160_animD4();
    delay(50);
   }
}
void loop() {
    for(unsigned int c=0; c<3; c++){
      // Note: Because the first block is responsible for the GPIO pins and
      // sending as is, in the remaining 3 blocks you see the off segments.
           allON();  // All On
           delay(500);
           allOFF(); // All Off
           delay(500);
    }
    for(uint8_t s = 0x00; s < 4; s++){
        backlightOff();
        delay(800);
        backlightOn();
        delay(800);
        backlightDimmer();
        delay(800);
        
            seg01_40__F();
            seg41_80_HI();
            seg81_120_OL();
            seg121_160_KS();
            delay(1500);
    }     
       anime();
 
  //Uncomment this two lines to proceed identification of segments on this driver... adapt to other if necessary!
  //Please don't forget of activation of Serial Monitor of IDE Arduino, to allow printing of running correctley,other way will block!
  allON();
  //allOFF(); //Comment or uncommment to allow the using option 1 seg Off or 1 seg On!
  searchOfSegments(); //Uncomment this line if you want run the find segments
}

void buttonReleasedInterrupt() {
  buttonReleased = true; // This is the line of interrupt button to advance one step on the search of segments!
}
