#include <avr/io.h>
#include <util/delay.h>
#include <avr/interrupt.h>

void spiInit(void)
{ // инициализация аппаратной части SPI
    // выводы SS (CS), MOSI, SCK – на выход
    DDRB |= (1 << 2) | (1 << 3) | (1 << 5);
    // включить аппаратный SPI, режим 0, предделитель на 128
    SPCR = (1 << SPE) | (1 << MSTR) | (1 << SPR1) | (1 << SPR0);
}

/*функция для выделения из трёхразрядного числа d разряда
m; например, digit(14, 3) = 4; или digit(591, 1) = 5 */
uint8_t digit(uint16_t d, uint8_t m)
{
    uint8_t i = 3, a;
    while (i)
    {               // цикл по разрядам числа
        a = d % 10; // выделяем очередной разряд
        if (i-- == m)
            break; // выделен заданный разряд-уходим
        d /= 10;   // уменьшаем число в 10 раз
    }
    return (a);
}

void showMe(int16_t spiData)
{
    uint8_t i;
    // разряды числа идут в массив dig[]
    uint8_t dig[] = {0, 0, 0, 0}, j;
    // ‘0’,..., ‘9’, сегмент выключен, ‘-’
    uint8_t mas[] =
        {0x81, 0xF3, 0x49, 0x61, 0x33, 0x25, 0x05, 0xF1, 0x01, 0x21, 0xFF, 0x7F};
    uint16_t res; // будем переприсваивать число в res
    if (spiData < 0)
        res = -spiData; // убираем «минус»
    else
        res = spiData;
    for (i = 1; i <= 3; i++)
        dig[i] = digit(res, i);
    if (res != 0)
    { // если передан не ноль – выводим число
        j = 0;
        while (dig[j] == 0) // убираем нули (не 0428, а _428)
            j++;
        /* сначала посылается знак, потом - сотни, потом –
        десятки, а после всего – единицы */
        if (spiData < 0)
        {
            SPDR = mas[11]; // отрицательное число-рисуем «минус»
            while (!(SPSR & (1 << SPIF)));
        }
        else{
            SPDR = mas[10]; // положительное число-рисуем «пусто»
            while (!(SPSR & (1 << SPIF)));
        }
        for (i = 1; i <= 3; i++)
        {
            if (i < j)
                SPDR = mas[10]; // вместо ненужных нулей-пусто
            else
                SPDR = mas[dig[i]];
            while (!(SPSR & (1 << SPIF))); // посылаем значение SPDR
        }
    }
    else
    {
        for (i = 0; i <= 2; i++)
        {
            SPDR = mas[10];
            while (!(SPSR & (1 << SPIF)));
        }
        SPDR = mas[0]; // если res == 0, выводим на экран ___0
        while (!(SPSR & (1 << SPIF)));
    }
    PORTB &= ~(1 << 2); // ”защелкиваем” выводимое число
    _delay_us(20);
    PORTB |= (1 << 2);
    _delay_us(20);
    PORTB &= ~(1 << 2);
}

int main(void)
{
    spiInit();
    
    while (1)
    {
        for(int16_t i = -990; i <= 990; i+=10)
        {
            showMe(i);
            _delay_ms(100);
        }
        for(int16_t i = 990; i >= -990; i-=10)
        {
            showMe(i);
            _delay_ms(100);
        }
    }
}