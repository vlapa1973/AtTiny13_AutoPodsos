/*
 *   Автомат управления воздушной заслонкой ДВС
 *   = vlapa = 20221225 - 20240226
 *
                     -----------------
                1 - | RES(PB5)    VCC | - 8
       кнопка - 2 - | PB3         PB2 | - 7 SCK   - Led
      стартер - 3 - | PB4         PB1 | - 6 MISO  - привод
                4 - | GND         PB0 | - 5 MOSI  - привод
                     -----------------
 */

#include <avr/io.h>
#include <util/delay.h>

#define pin_UP PB1    //  привод
#define pin_DOWN PB0  //  привод
#define pin_IN PB4    //  стартер
#define pin_DELAY PB3 //  кнопка выбора времени работы
#define pin_LED PB2   //  пин светодиода индикации времени

uint32_t delayPrivodWork = 800;         //  время работы привода (0.8сек)
uint32_t delayPrivodPause = 1000;       //  пауза перед сбросом (1сек)
uint8_t delayPrivodCycle;               //  кол-во секунд между переключениями
const uint8_t delayPrivodCycleMax = 10; //  MAX кол-во секунд между переключениями
uint8_t cellMem = 1;                    //  ячейка памяти для счетчика

//==================================================
void EEPROM_write(uint8_t ucAddress, uint8_t ucData)
{
  while (EECR & (1 << EEPE))
    ;
  EECR = (0 << EEPM1) | (0 >> EEPM0);
  EEARL = ucAddress;
  EEDR = ucData;
  EECR |= (1 << EEMPE);
  EECR |= (1 << EEPE);
}

unsigned char EEPROM_read(uint8_t ucAddress)
{
  while (EECR & (1 << EEPE))
    ;
  EEARL = ucAddress;
  EECR |= (1 << EERE);
  return EEDR;
}

void visible(uint8_t count)
{
  for (uint8_t i = 0; i < count; i++)
  {
    PORTB &= (~(1 << pin_LED));
    _delay_ms(300);
    PORTB |= (1 << pin_LED);
    _delay_ms(300);
  }
}

//==================================================

int main(void)
{
  PORTB = 0b00011100;
  PINB = 0b00011000;
  DDRB = 0b00000111;

  PORTB &= (~(1 << pin_DOWN));
  PORTB |= (1 << pin_UP);
  _delay_ms(delayPrivodWork);
  PORTB &= (~(1 << pin_UP));
  PORTB &= (~(1 << pin_DOWN));
  _delay_ms(10);
  PORTB &= (~(1 << pin_UP));
  PORTB |= (1 << pin_DOWN);
  _delay_ms(delayPrivodWork);
  PORTB &= (~(1 << pin_UP));
  PORTB &= (~(1 << pin_DOWN));
  _delay_ms(10);

  if (EEPROM_read(0) != 0xAA)
  {
    EEPROM_write(0, 0xAA);
    EEPROM_write(cellMem, 1);
    delayPrivodCycle = 1;
  }
  else
  {
    delayPrivodCycle = EEPROM_read(cellMem);
    if (delayPrivodCycle > delayPrivodCycleMax)
    {
      delayPrivodCycle = 1;
      EEPROM_write(cellMem, delayPrivodCycle);
    }
    _delay_ms(1000);
    visible(delayPrivodCycle);
  }

  //==================================================
  while (1)
  {
    PORTB = 0b00011100;
    if (!((1 << pin_IN) & PINB))
    {
      _delay_ms(30);
      if (!((1 << pin_IN) & PINB))
      {
        PORTB |= (1 << pin_UP);
        PORTB &= (~(1 << pin_DOWN));
        _delay_ms(delayPrivodWork);
        PORTB &= (~(1 << pin_UP));
        PORTB &= (~(1 << pin_DOWN));

        while (!((1 << pin_IN) & PINB))
        {
          _delay_ms(10);
          continue;
        }

        if (((1 << pin_IN) & PINB))
        {
          _delay_ms(30);
          if ((1 << pin_IN) & PINB)
          {
            visible(delayPrivodCycle);
            // for (uint8_t k = 0; k < delayPrivodCycle; ++k)
            // {
            //   _delay_ms(delayPrivodPause);
            // }

            PORTB &= (~(1 << pin_UP));
            PORTB |= (1 << pin_DOWN);
            _delay_ms(delayPrivodWork);
            PORTB &= (~(1 << pin_UP));
            PORTB &= (~(1 << pin_DOWN));
          }
        }
      }
    }

    //*********************************
    if (!((1 << pin_DELAY) & PINB))
    {
      _delay_ms(30);
      if (!((1 << pin_DELAY) & PINB))
      {
        while (!((1 << pin_DELAY) & PINB))
        {
          _delay_ms(10);
          continue;
        }
        if (((1 << pin_DELAY) & PINB))
        {
          _delay_ms(30);
          if (((1 << pin_DELAY) & PINB))
          {
            // visible(delayPrivodCycle);
            // delayPrivodCycle = EEPROM_read(cellMem);
            delayPrivodCycle++;

            if (delayPrivodCycle > delayPrivodCycleMax)
            {
              delayPrivodCycle = 1;
            }

            EEPROM_write(cellMem, delayPrivodCycle);
            _delay_ms(1000);
            visible(delayPrivodCycle);
          }
        }
      }
    }
  }
}

//==================================================