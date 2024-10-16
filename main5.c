#include <avr/io.h>
#include <util/delay.h>
#include <avr/interrupt.h>

uint16_t dist = 0;


ISR(INT7_vect) {
    //если фронт сигнала на ECHO, то сбрасываем таймер
    if((PINE & (1<<7)) != 0) {
    TCNT1H = 0;
    TCNT1L = 0;
    }
    else { //если срез сигнала на ECHO, то забираем значение
    dist = TCNT1L;
    dist |= (TCNT1H << 8);
    }
}

uint8_t find_non_zero(uint16_t d)
{
	uint8_t i = 1; uint16_t s = 10000;
	while(d/s == 0)
	{
		i++;
		s/=10;
	}
	return(i);
}

uint8_t Digit (uint16_t d, uint8_t m)
{
	uint8_t i = 5, a;
	while(i)
	{
		a = d%10;
		if (i-- == m) break;
		d/=10;
	}
	return(a);
}

int main(void)
{
    const uint8_t led[] = {0x3F, 0x06, 0x5B, 0x4F, 0x66, 0x6D, 0x7D, 0x07, 0x7F, 0x6F};
    DDRA = 0xFF;
	DDRC = 0xFF;
    DDRE |= (1 << 3) | (1 << 4) | (1 << 5) | (1 << 6);
    EIMSK |= (1 << INT7);
    EICRB |= (1 << ISC71);
    TCCR1A = TCCR1A;
    TCCR1B = (1 << CS12) | (1 << CS10)
    TCCR3A = (1 << COM3A1) | (1 << COM3B1) | (1 << COM3C1) | (1 << WGM30);
    TCCR3B = (1 << WGM32) | (1 << CS31);
    OCR3AH = 0; OCR3BH = 0; OCR3CH = 0;
    sei();
    while(1)
    {
        PORTE |= (1 << 6);
        _delay_us(15);
        PORTE &= ~(1 << 6);
        delay_ms(30);
        
        dist *= 1.57;

        if(dist <= 10) dist = 10;
        if(dist >= 100) dist = 100;

        
    }
}