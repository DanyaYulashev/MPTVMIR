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
    while (BIT_IS_CLEAR(ADCSRA, ADIF));
    ADCSRA |= _BV(ADIF);
    cashe = ADCL;
    return ADCH;
}

uint8_t Digit (uint16_t d, uint8_t m)
{
	uint8_t i = 3, a;
	while(i)
	{
		a = d%10;
		if (i-- == m) break;
		d/=10;
	}
	return(a);
}

void lcdCmd(uint8_t cmd) { // посыл команды на экран
DDRC = 0xFF; // все разряды PORTC на выход
DDRD |= ((1<<E)|(1<<RS));// разряды PORTD на выход
PORTD &= ~(1<<RS);// выбор регистра команд RS=0
PORTC = cmd; // записать команду в порт PORTC
PORTD |= (1<<E); // \ сформировать на
_delay_us(5); // | выводе E строб 1-0
PORTD &= ~(1<<E); // / передачи команды
_delay_ms(10); // задержка для завершения записи
}

void lcdInit(void) { // инициализация (ВКЛ) экрана
DDRC = 0xFF; // все разряды PORTC на выход
DDRD |= ((1<<E)|(1<<RS)); // разряды PORTD на выход
_delay_ms (100); // задержка для установления питания
lcdCmd(0x30); // \ вывод
lcdCmd(0x30); // | трех
lcdCmd(0x30); // / команд 0x30
lcdCmd(0x38); // 8 разр.шина, 2 строки, 5 × 7 точек
lcdCmd(0x0C); // включить ЖКИ
lcdCmd(0x06); // инкремент курсора, без сдвига экрана
lcdCmd(0x01); // очистить экран, курсор в начало
}

void lcdData(uint8_t data) { // посыл данных на экран
DDRC = 0xFF;
DDRD |= ((1<<E)|(1<<RS));
PORTD |= (1<<RS);
PORTC = data;
PORTD |= (1<<E);
_delay_us(5);
PORTD &= ~(1<<E);
_delay_ms(1);
}

int main(void)
{
    // Инициализация портов ввода/вывода
    DDRB = _BV(5);
    DDRG = _BV(3);
    lcdInit();
    // Инициализация АЦП
    ADCSRA = _BV(ADEN) | _BV(ADPS2) | _BV(ADPS1); //| _BV(ADPS0);
    // Инициализация таймера 1. Быстрая ШИМ 8 бит
    TCCR1A = _BV(COM1A1) | _BV(WGM10) | _BV(WGM11);
    TCCR1B = _BV(WGM12) | _BV(CS10);
    /* переменные магнитного поля, ошибки регулирования и
    управления (здесь будут ещё переменные, например,
    значение ошибки регулирования на предыдущем шаге)*/
    int16_t field, error = 0, ref, integral = 0, derivative = 0, last_error = 0;
    float control;
    
    while (1)
    {
        // АЦП 8 бит и предделитель на 128
        ref = (readAdc(3) + readAdc(3) + readAdc(3)) / 3;
        /* здесь (мб) должно быть ограничение задания, чтоб
        шайба не улетала*/
        field = (readAdc(1) + readAdc(1) + readAdc(1)) / 3;
        
        error = 80 - field;
        derivative = (error - last_error);
        last_error = error;
        integral += error;
        control = error * 10.6 + integral * 0 + derivative * 1.9;
        if (control > 1023) // ограничение управления сверху
            control = 1023;
        if (control < -1023) // ограничение управления снизу
            control = -1023;
        if(field >= 100)
        {
            PORTG |= (1 << 3);
            OCR1A = 0;
        }
        else
            if (control >= 0)
            { // установка сигнала управления
                PORTG &= ~(1 << 3);
                OCR1A = (uint16_t)(control);
            }
            else
            {
                PORTG |= (1 << 3);
                OCR1A = (uint16_t)(-control);
            }
        for(uint8_t i = 1; i <= 3; i++)
		{
			lcdData(Digit(field,i)+'0');
		}
        lcdData(' ');
        for(uint8_t i = 1; i <= 3; i++)
		{
			lcdData(Digit(80,i)+'0');
		}
        lcdData(' ');
        for(uint8_t i = 1; i <= 3; i++)
		{
			lcdData(Digit(control,i)+'0');
		}
        _delay_ms(50);
        lcdCmd(0x01);
    }
}
