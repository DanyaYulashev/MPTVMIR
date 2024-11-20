#include <avr/io.h>
#include <util/delay.h>
#include <avr/interrupt.h>
uint8_t cashe = 0;
#define _BV(b) (1 << (b))
#define MUX (_BV(REFS0))
#define BIT_IS_CLEAR(Reg, b) ((Reg & _BV(b)) == 0)
#define E 6
#define RS 7

uint8_t readAdc(uint8_t channel)
{
    ADMUX = MUX | (1 << ADLAR) | channel;
    _delay_us(10);
    ADCSRA |= _BV(ADSC);
    while (BIT_IS_CLEAR(ADCSRA, ADIF))
        ;
    ADCSRA |= _BV(ADIF);
    cashe = ADCL;
    return ADCH;
}

int main(void)
{
    // Инициализация портов ввода/вывода
    DDRB = _BV(5);
    DDRG = _BV(3);
    lcdInit();
    // Инициализация АЦП
    ADCSRA = _BV(ADEN) | _BV(ADPS2) | _BV(ADPS1) | _BV(ADPS0);
    // Инициализация таймера 1. Быстрая ШИМ 10 бит
    TCCR1A = _BV(COM1A1) | _BV(WGM10);
    TCCR1B = _BV(WGM12) | _BV(CS10);
    /* переменные магнитного поля, ошибки регулирования и
    управления (здесь будут ещё переменные, например,
    значение ошибки регулирования на предыдущем шаге)*/
    float field, error = 0, ref, integral = 0, derivative = 0, last_error = 0;
    float control;

    while (1)
    {
        // АЦП 8 бит и предделитель на 128
        ref = (readAdc(3) + readAdc(3) + readAdc(3)) / 3;
        /* здесь (мб) должно быть ограничение задания, чтоб
        шайба не улетала*/
        field = (readAdc(1) + readAdc(1) + readAdc(1)) / 3;
        ref *= 0.25;
        error = ref - field;
        derivative = (error - last_error);
        last_error = error;
        integral += error;
        control = error * 1.5 + integral * 0.00025 + derivative * 100.0;
        if (control > 255) // ограничение управления сверху
            control = 255;
        if (control < -255) // ограничение управления снизу
            control = -255;
        if (control >= 0.00)
        { // установка сигнала управления
            PORTG &= ~(1 << 3);
            OCR1AL = (uint8_t)(control);
        }
        else
        {
            PORTG |= (1 << 3);
            OCR1AL = (uint8_t)(-control);
        }
    }
}
