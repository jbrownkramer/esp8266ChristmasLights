// code to bit bang the protocol for the Walgreen's Christmas lights
class WalgreenLights
{
  private:
    int  pin;  // the pin to write to
    int  numLights;

    #define GPIO_OUT ((volatile uint32_t*) 0x60000300);
    int pinMask;
    
    void SendPacket(unsigned char val)
    {
      register volatile uint32_t *port = GPIO_OUT;
      register int mask = pinMask;
      
      /////////////////////////////////////////////
      //  **********   WARNING  **********
      // This code runs with INTERRUPTS DISABLED
      ////////////////////////////////////////////
      //uint8_t oldSREG = SREG;  // get interrupt register state
      cli();    // disable interrupts

      // 3 pulses of H 6.25us + L 6.67us 
      for (int i = 0; i < 3; i++)
      {
        *port |= mask;  // HIGH
        //digitalWrite(pin, HIGH);
        delayMicroseconds(6);
        *port &= ~mask;  // LOW
        //digitalWrite(pin, LOW);
        delayMicroseconds(6);
      }
      
      // guess at LSB first
      for (int i = 0; i < 4; i++)
      {
        *port |= mask;  // HIGH
        //digitalWrite(pin, HIGH);
        delayMicroseconds((val & 1) ? 3 : 0);
        //digitalWrite(pin, LOW);
        *port &= ~mask;  // LOW
        delayMicroseconds(3);
        val >>= 1;
      }

      //SREG = oldSREG;  // restore interrupts

      // hold for at least one full packet time
      // this allows the downstream neighbor to forward on
      // his state to the next guy
      // could be 60us if we wanted to get really tight
      // 100us seems like plenty
      // default system waits 1.13ms between packets
      delayMicroseconds(100);
    }


    void InitString(void)
    {
      // these delay values were determined by monitoring a set of lights
      digitalWrite(pin, HIGH);
      delay(150);
      digitalWrite(pin, LOW);
      delay(225);
      
      for (int i = 0; i < numLights; i++)
      {
        SendPacket(0);
      }
    }

    // set the value on one color of lights only
    // all others at 'other' (default 0 = off)
    // offsets: 0=White, 1=Orange, 2=Blue, 3=Green, 4=Red
    void OneColor(int offset, unsigned char val, unsigned char other = 0)
    {
      for (int i = 0; i < numLights; i++)
      {
        if (i % 5 == offset)
          SendPacket(val);
        else
          SendPacket(other);
      }
    }

  public:
    WalgreenLights(int _pin, int _numLights)
    {
      pin = _pin;
      numLights = _numLights;

      //pinPort = (pin < 8) ? &PORTD : &PORTB;
      pinMask = (pin < 8) ? 1 << pin : 1 << (pin - 8);
      
      pinMode(pin, OUTPUT);
      digitalWrite(pin, LOW);
      
      InitString();
    }
    
    ~WalgreenLights(void)
    {
      digitalWrite(pin, LOW);
      pinMode(pin, INPUT);
    }
    
    // valPtr points to an array of char
    // each char holds the value (0-15) for
    // the bulbs in the string
    void SendValue(unsigned char *valPtr)
    {
      for (int i = 0; i < numLights; i++)
      {
        SendPacket(*valPtr++);
      }
    }
    
    // set the value on the red lights only
    // all others at 'other' (default 0 = off)
    void RedOnly(unsigned char val, unsigned char other = 0)
    {
      OneColor(4, val, other);
    }
    void GreenOnly(unsigned char val, unsigned char other = 0)
    {
      OneColor(3, val, other);
    }
    void BlueOnly(unsigned char val, unsigned char other = 0)
    {
      OneColor(2, val, other);
    }
    void OrangeOnly(unsigned char val, unsigned char other = 0)
    {
      OneColor(1, val, other);
    }
    void WhiteOnly(unsigned char val, unsigned char other = 0)
    {
      OneColor(0, val, other);
    }
    
    void AllTo(unsigned char val)
    {
      OneColor(0, val, val);
    }
    
    void AllOn(void)
    {
      AllTo(15);
    }
    
    void AllOff(void)
    {
      AllTo(0);
    }
};
