
#ifndef _ws2812b_h
#define _ws2812b_h

class ws2812b {
  public:
    ws2812b();
    void setPin(int);
    void setLEDs(int *, int);
    uint32_t ColorValue(byte, byte, byte);
    
  private:
    void sendBinary(int);
    GPIO_TypeDef *GPIO;
    uint32_t bit;
};

#endif

