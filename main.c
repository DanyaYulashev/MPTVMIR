#include <avr/io.h>
#include <avr/interrupt.h>

#define ICR_VAL 27648
#define MIN_POS (ICR_VAL/20)
#define MAX_POS (ICR_VAL/10)

uint16_t Pos2 = 0;

ISR (INT0_vect)
{
	if ((PIND & (1 << 0)) != 0)
	{
		EICRA = (1 << ISC01);
		if((PIND & (1 << 1)) != 0) Pos2+=5;
		else Pos2-=5;
	}
	else
	{
		EICRA = (1 << ISC01) | (1 << ISC00);
		if((PIND & (1 << 1)) != 0) Pos2-=5;
		else Pos2+=5;
	}
}

uint8_t read_adc_H(uint8_t channel)
{
    ADMUX = (1 << REFS0) | channel;
    ADCSRA |= (1 << ADSC);
    while(!(ADCSRA & (1 << ADIF)));
    ADCSRA |= (1 << ADIF);
    return(ADCH);
}
uint8_t read_adc_L(uint8_t channel)
{
    ADMUX = (1 << REFS0) | channel;
    ADCSRA |= (1 << ADSC);
    while(!(ADCSRA & (1 << ADIF)));
    ADCSRA |= (1 << ADIF);
    return(ADCL);
}

int main(void) {
uint16_t Pos1 = 0;
DDRB = (1<<5) | (1<<6); ADCSRA = (1 << ADEN);
//быстрая ШИМ, предел счёта: ICR, предделитель: 8
TCCR1A = (1<<COM1A1) | (1<<COM1B1) | (1<<WGM11);
TCCR1B = (1<<WGM13) | (1<<WGM12) | (1<<CS11);
EIMSK = (1 << INT0);
EICRA = (1 << ISC00) | (1 < ISC01);
ICR1 = ICR_VAL;
OCR1A = MIN_POS;
OCR1B = MAX_POS;
sei();
while (1)
{
    Pos1 = (read_adc_H(3) << 8) | (read_adc_L);
    OCR1A = Pos1;
    OCR1B = Pos2;
}
}