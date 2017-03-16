#include "Arduino.h"
#include "stm32l4_wiring_private.h"

#include "ws2812b.h"

// Lets begin
ws2812b::ws2812b()
{
}

// Set the pin used to communicate
void ws2812b::setPin(int pin)
{
  pinMode(pin, OUTPUT);
  digitalWrite(pin, LOW);
  GPIO = (GPIO_TypeDef*)g_APinDescription[pin].GPIO;
  bit = g_APinDescription[pin].bit;
}

// Set the color of each LED
void ws2812b::setLEDs(uint32_t* ledValues, int leds)
{
  for (int j=0;j<leds;j++)
  {
    sendBinary(ledValues[j]);
  }
}

// Build the color value as RGB
uint32_t ws2812b::ColorValue(byte Green, byte Red, byte Blue)
{
  uint32_t grb;
  grb = (uint32_t)Green << 16 | (uint32_t)Red << 8 | Blue;
  return grb;
}

// Release the hounds
void ws2812b::sendBinary(uint32_t grb)
{
  int ps = 6; // pulse short 6*3=18us (.4us)
  int pl = 24; // pulse long 24*3=72us (.8us)
  int pt = 30; // total pulse (80 cycles/1us) 3 cycles per (1.25us)
  int i; //bits to send
  int w; // temp loop length
  int w1; // temp loop

  uint32_t GRBt = grb;
  noInterrupts();  //timing is critical 
  __asm__ volatile (
      "     mov %[i], #24                   \n\t" // number of bits to send
      "     lsl %[GRBt], #8                 \n" // remove 8 bits from 32
      "0:   lsls %[GRBt], %[GRBt], #1       \n\t" // send bit by carry (1)
      "     str  %[bit], [%[port], 0x18]    \n\t" // set the pin high (2)
      "     bcs k                           \n\t" // high value ? (2)
      "     mov %[w], %[ps]                 \n\t" // short value (low) (1)
      "     b x                             \n\t" // skip over (2)
      "k:   mov %[w], %[pl]                 \n\t" // long value (high) (1)
      "x:   mov %[w1], %[w]                 \n\t" // set loop count (1)
      "3:   subs %[w1], #1                  \n\t" // count down (1)
      "     bne 3b                          \n\t" // again (2)
      "     str  %[bit], [%[port], 0x28]    \n\t" // set pin low (2)
      "     mov %[w1], %[pt]                \n\t" // total time (1)
      "     sub %[w1], %[w]                 \n\t" // set remaining (1)
      "     sub %[w1], #1                   \n\t" // shorten low (don't know why)
      "4:   subs  %[w1], #1                 \n\t" // count down (1)
      "     bne 4b                          \n\t" // again (2)
      "     subs %[i], #1                   \n\t" // bit gone (1)
      "     bne 0b                          \n\t" // get another bit (2)
      : [w]     "+r"  (w),      // high low wait value
        [w1]    "+r"  (w1),     // loop counter
        [i]     "+r"  (i),      // bits to send 24
        [GRBt]  "+r"  (GRBt)    // green red blue values
      : [port]  "r" (GPIO),     // port mapping
        [bit]   "r"  (bit),     // bit used for port
        [ps]    "r" (ps),       // short pulse value
        [pl]    "r" (pl),       // long pulse value
        [pt]    "r" (pt));      // total pulse value
  interrupts();
}

