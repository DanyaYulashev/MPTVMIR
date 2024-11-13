#include <avr/io.h>
#include <util/delay.h>

#define DEV_ADDR 0b1100000 // адрес модуля по умолчанию
#define W 0                // признак записи данных в матрицу
#define R 1                // признак чтения данных из матрицы
// регистр состояния диодов столбца
#define COLUMN_DATA_REG 0x01
// регистр обновления состояния диодов столбца
#define UPDATE_COLUMN_REG 0x0C

void twiStart();
void twiStop();
void twiWrite(uint8_t data);
void matrDrawPict(uint8_t devAddr, uint8_t *dataBuf);
void matrInit(uint8_t devAddr);

int main(void)
{
    uint8_t pict[][8] = {{0xFF, 0b00011000, 0b00011000, 0xFF, 0x00, 0xFF, 0b10011001, 0b10011001},
                         {0b00011000, 0b00011000, 0xFF, 0x00, 0xFF, 0b10011001, 0b10011001, 0b10011001},
                         {0b00011000, 0xFF, 0x00, 0xFF, 0b10011001, 0b10011001, 0b10011001, 0x00},
                         {0xFF, 0x00, 0xFF, 0b10011001, 0b10011001, 0b10011001, 0x00, 0xFF},
                         {0x00, 0xFF, 0b10011001, 0b10011001, 0b10011001, 0x00, 0xFF, 0x01},
                         {0xFF, 0b10011001, 0b10011001, 0b10011001, 0x00, 0xFF, 0x01, 0x01},
                         {0b10011001, 0b10011001, 0b10011001, 0x00, 0xFF, 0x01, 0x01, 0x01},
                         {0b10011001, 0b10011001, 0x00, 0xFF, 0x01, 0x01, 0x01, 0x00},
                         {0b10011001, 0x00, 0xFF, 0x01, 0x01, 0x01, 0x00, 0xFF},
                         {0x00, 0xFF, 0x01, 0x01, 0x01, 0x00, 0xFF, 0x01},
                         {0xFF, 0x01, 0x01, 0x01, 0x00, 0xFF, 0x01, 0x01},
                         {0x01, 0x01, 0x01, 0x00, 0xFF, 0x01, 0x01, 0x01},
                         {0x01, 0x01, 0x00, 0xFF, 0x01, 0x01, 0x01, 0x00},
                         {0x01, 0x00, 0xFF, 0x01, 0x01, 0x01, 0x00, 0xFF},
                         {0x00, 0xFF, 0x01, 0x01, 0x01, 0x00, 0xFF, 0b10000001},
                         {0xFF, 0x01, 0x01, 0x01, 0x00, 0xFF, 0b10000001, 0b10000001},
                         {0x01, 0x01, 0x01, 0x00, 0xFF, 0b10000001, 0b10000001, 0xFF}
                         {0b10000001, 0xFF, 0x00, 0xFF, 0b00011000, 0b00011000, 0xFF, 0x00},
                         {0xFF, 0x00, 0xFF, 0b00011000, 0b00011000, 0xFF, 0x00, 0xFF},
                         {0x00, 0xFF, 0b00011000, 0b00011000, 0xFF, 0x00, 0xFF,0b10011001}
                         {0xFF, 0b10000001, 0b10000001, 0xFF, 0x00, 0xFF, 0b00011000, 0b00011000},
                         {0b10000001, 0b10000001, 0xFF, 0x00, 0xFF, 0b00011000, 0b00011000, 0xFF},
                         {0b10000001, 0xFF, 0x00, 0xFF, 0b00011000, 0b00011000, 0xFF, 0x00},
                         {0xFF, 0x00, 0xFF, 0b00011000, 0b00011000, 0xFF, 0x00, 0xFF},
                         {0x00, 0xFF, 0b00011000, 0b00011000, 0xFF, 0x00, 0xFF,0b10011001}};
    matrInit(DEV_ADDR);
    while (1)
    {
        for(uint8_t i = 0; i < 17; i++)
        {
            matrDrawPict(DEV_ADDR, pict[i]);
            _delay_ms(300);
        }
        _delay_ms(1000);
    }
}
void twiStart(void)
{ // состояние СТАРТ
    TWCR = (1 << TWINT) | (1 << TWSTA) | (1 << TWEN);
    while (!(TWCR & (1 << TWINT)));
}
void twiStop(void)
{ // состояние СТОП
    TWCR = (1 << TWINT) | (1 << TWEN) | (1 << TWSTO);
}
void twiWrite(uint8_t data)
{ // передача данных в матрицу
    TWDR = data;
    TWCR = (1 << TWINT) | (1 << TWEN);
    while (!(TWCR & (1 << TWINT)));
}
// передача изображения по столбцам на матрицу с МК
void matrDrawPict(uint8_t devAddr, uint8_t *dataBuf)
{
    uint8_t i;
    twiStart();
    twiWrite((DEV_ADDR << 1) | W);
    for (i = 0; i <= 7; i++)
    {
        // очередную часть рисунка кладём в очередной регистр
        twiWrite(COLUMN_DATA_REG + i);
        twiWrite(dataBuf[i]);
    }
    // чтобы рисунок был выведен, обновляем регистр 0x0C
    twiWrite(UPDATE_COLUMN_REG);
    twiWrite(0xFF);
    twiStop();
}
void matrInit(uint8_t devAddr)
{
    TWBR = 32; // 200 кГц частота обмена
    /* настройки по умолчанию (матрица 8х8, без
    эквалайзера, 40мА тока) нам подходят, поэтому ничего
    не меняем и не перенастраиваем */
}
