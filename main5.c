#include <avr/io.h>
#include <util/delay.h>
#include <avr/interrupt.h>

uint16_t dist = 0;

ISR(INT7_vect)
{
    // если фронт сигнала на ECHO, то сбрасываем таймер
    if ((PINE & (1 << 7)) != 0)
    {
        TCNT1H = 0;
        TCNT1L = 0;
    }
    else
    { // если срез сигнала на ECHO, то забираем значение
        dist = TCNT1L;
        dist |= (TCNT1H << 8);
    }
}

uint8_t find_non_zero(uint16_t d)
{
    uint8_t i = 1;
    uint16_t s = 10000;
    while (d / s == 0)
    {
        i++;
        s /= 10;
    }
    return (i);
}

uint8_t Digit(uint16_t d, uint8_t m)
{
    uint8_t i = 5, a;
    while (i)
    {
        a = d % 10;
        if (i-- == m)
            break;
        d /= 10;
    }
    return (a);
}

int main(void)
{
    const uint8_t led[] = {0x3F, 0x06, 0x5B, 0x4F, 0x66, 0x6D, 0x7D, 0x07, 0x7F, 0x6F};
    const uint8_t close[] = {0x39, 0x38, 0x3F, 0x6D, 0x79};
    const uint8_t far[] = {0x00, 0x00, 0x71, 0x77, 0x31};
    uint8_t c = 0, shine = 0;
    DDRA = 0xFF;
    DDRC = 0xFF;
    DDRE |= (1 << 3) | (1 << 4) | (1 << 5) | (1 << 6);
    EIMSK |= (1 << INT7);
    EICRB |= (1 << ISC70);
    TCCR1A = TCCR1A;
    TCCR1B = (1 << CS12) | (1 << CS10);
    TCCR3A = (1 << COM3A1) | (1 << COM3B1) | (1 << COM3C1) | (1 << WGM30);
    TCCR3B = (1 << WGM32) | (1 << CS31);
    OCR3AH = 0;
    OCR3BH = 0;
    OCR3CH = 0;
    sei();
    while (1)
    {
        PORTE |= (1 << 6);
        _delay_us(25);
        PORTE &= ~(1 << 6);
        _delay_ms(200);

        dist = ((float)(dist * 1.57));
        shine = dist * 255 / 100;
        if ((dist <= 10) || (dist >= 100))
        {
            if (dist <= 10)
            {
                for (uint8_t i = 1; i <= 5; i++)
                {
                    PORTA |= (1 << i);
                    PORTC = close[i-1];
                    _delay_us(25);
                    PORTA &= ~(1 << i);
                }
                OCR3AL = 255;
                OCR3BL = 255;
                OCR3CL = 255;
            }
            if (dist >= 100)
            {
                for (uint8_t i = 1; i <= 5; i++)
                {
                    PORTA |= (1 << i);
                    PORTC = far[i-1];
                    _delay_us(25);
                    PORTA &= ~(1 << i);
                }
                OCR3AL = 255;
                OCR3BL = 0;
                OCR3CL = 0;
            }
        }
        else
        {
            OCR3AL = shine;
            OCR3BL = 255 - shine;
            OCR3CL = 255 - shine;
            c = find_non_zero(dist);
            for (uint8_t i = c; i <= 5; i++)
            {
                PORTA |= (1 << i);
                PORTC = led[Digit(dist, i)];
                _delay_us(25);
                PORTA &= ~(1 << i);
            }
            for(uint8_t i = 1; i < c; i++)
            {
                PORTA |= (1 << i);
                PORTC = 0x00;
                _delay_us(25);
                PORTA &= ~(1 << i);
            }
        }
        
        // _delay_ms(10);
    }
}