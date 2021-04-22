/**
 *
 *
 *
 */

#include "n1202.h"

uint8_t _LCD_RAM[LCD_X*LCD_String]; // Память нашего LCD

// Задержка в микросекундах
void delay_us ( uint32_t us )
{
	volatile uint32_t delay = (us * (SystemCoreClock / 1000000)/8);
	while (delay--);
}
// Задержка в милисекундах
void delay_ms ( uint32_t ms )
{
	volatile uint32_t delay = (ms * (SystemCoreClock / 1000)/8);
	while (delay--);
}

// Передача данных на экран
void LCD_SendByte(SPI_HandleTypeDef*  hspi, uint8_t mode, uint8_t c)
{
	uint8_t SPI_Data[2];

	HAL_GPIO_WritePin(LCD_CS_GPIO_PORT, LCD_CS_PIN, GPIO_PIN_RESET);
	SPI_Data[0] = c;
	SPI_Data[1] = mode;
	HAL_SPI_Transmit(hspi, SPI_Data, 1, 5);
	//delay_us(1);
	HAL_GPIO_WritePin(LCD_CS_GPIO_PORT, LCD_CS_PIN, GPIO_PIN_SET);
}

// Очистка памяти дисплея
void LCD_Clear(void) {
	for (int index = 0; index < LCD_X*LCD_String; index++)
	{
		_LCD_RAM[index] = (0x00);
	}
}

// Обновляем данные на экране
void LCD_Update(SPI_HandleTypeDef*  hspi) 
{
  	for(uint8_t p = 0; p < 9; p++) 
	{
    	LCD_SendByte(hspi, LCD_C, SetYAddr | p);
    	LCD_SendByte(hspi, LCD_C, SetXAddr4);
    	LCD_SendByte(hspi, LCD_C, SetXAddr3);
    	for(uint8_t col=0; col < LCD_X; col++)
		{
    		LCD_SendByte(hspi, LCD_D, _LCD_RAM[(LCD_X * p) + col]);
		}
    }
}

// Рисование пикселя по координатам и цвету
void LCD_DrawPixel (uint8_t x, uint8_t y, uint8_t color) 
{
  	if ((x < 0) || (x >= LCD_X) || (y < 0) || (y >= LCD_Y)) 
  		return;

  	if (color) 
	  	_LCD_RAM[x + (y/8)*LCD_X] |= 1<<(y % 8);
  	else       
	  	_LCD_RAM[x + (y/8)*LCD_X] &= ~(1<<(y % 8));
}

// Рисование линии
void LCD_DrawLine(uint8_t x0, uint8_t y0, uint8_t x1, uint8_t y1, uint8_t color) 
{
  	int steep = abs(y1 - y0) > abs(x1 - x0);
  	if (steep) 
	{
  	  	swap(x0, y0);
  	  	swap(x1, y1);
  	}
  	if (x0 > x1) 
	{
  	  	swap(x0, x1);
  	  	swap(y0, y1);
  	}
  	int dx, dy;
  	dx = x1 - x0;
  	dy = abs(y1 - y0);
  	int err = dx / 2;
  	int ystep;
  	if (y0 < y1) 
	  	ystep = 1;
  	else
	  	ystep = -1;
  	for ( ; x0 <= x1; x0++ ) 
	{
  	  	if (steep) 
			LCD_DrawPixel(y0, x0, color);
  	  	else 
			LCD_DrawPixel(x0, y0, color);
		err -= dy;
  	  	if (err < 0) 
		{
  	    	y0 += ystep;
  	    	err += dx;
  	  	}
  	}
}

// Рисование вертикальной линии
void LCD_DrawFastVLine(uint8_t x, uint8_t y, uint8_t h, uint8_t color) 
{
  	LCD_DrawLine(x, y, x, y+h-1, color);
}

// Рисование горизонтальной линии
void LCD_DrawFastHLine(uint8_t x, uint8_t y, uint8_t w, uint8_t color) {
  	LCD_DrawLine(x, y, x+w-1, y, color);
}

// Рисование прямоугольника
void LCD_DrawRect(uint8_t x, uint8_t y, uint8_t w, uint8_t h, uint8_t color) {
  	LCD_DrawFastHLine(x, y, w, color);
  	LCD_DrawFastHLine(x, y+h-1, w, color);
  	LCD_DrawFastVLine(x, y, h, color);
  	LCD_DrawFastVLine(x+w-1, y, h, color);
}

// Рисование залитый прямоугольник
void LCD_FillRect(uint8_t x, uint8_t y, uint8_t w, uint8_t h, uint8_t color) 
{
  	for (int16_t i=x; i<x+w; i++) 
	{
    	LCD_DrawFastVLine(i, y, h, color);
  	}
}

// Заливка экрана
void LCD_FillScreen(uint8_t color) 
{
  	LCD_FillRect(0, 0, LCD_X, LCD_Y, color);
}

// Нарисовать букву
void LCD_DrawChar(uint8_t x, uint8_t y, uint8_t color, unsigned char c) 
{
  	if((x >= LCD_X) ||(y >= LCD_Y) || ((x + 4) < 0) || ((y + 7) < 0)) 
	  return;
  	if(c < 128)
	  	c = c - 32;
  	if(c >= 144 && c <= 175)
	  	c = c - 48;
  	if(c >= 128 && c <= 143)
	  	c = c + 16;
  	if(c >= 176 && c <= 191)
	  	c = c - 48;
  	if(c > 191)
	  	return;
  	for (uint8_t i = 0; i < 6; i++ )
	{
  	  	uint8_t line;
  	  	if (i == 5) 
		{
			line = 0x00;
		}
  	  	else 
		{
			line = font[(c * 5) + i];
  	  		for (uint8_t j = 0; j < 8; j++)
			{
				if (line & 0x01) 
				{
					LCD_DrawPixel(x + i, y + j, color);
				}
				else 
				{
					LCD_DrawPixel(x + i, y + j, !color);
				}
				line >>= 1;
			}
		}
  	}
}

void LCD_DrawChar2(uint8_t x, uint8_t y, uint8_t color, unsigned char c)
{
  	if((x >= LCD_X) ||(y >= LCD_Y) || ((x + 4) < 0) || ((y + 7) < 0))
	  return;

  	int numSymbol = -1;
  	int width = 7;
  	int size = 14;

  	switch(c)
  	{
  	case '0': numSymbol = 0;
  	break;
  	case '1': numSymbol = 1;
  	break;
  	case '2': numSymbol = 2;
  	break;
  	case '3': numSymbol = 3;
  	break;
  	case '4': numSymbol = 4;
  	break;
  	case '5': numSymbol = 5;
    break;
  	case '6': numSymbol = 6;
  	break;
  	case '7': numSymbol = 7;
  	break;
  	case '8': numSymbol = 8;
    break;
  	case '9': numSymbol = 9;
  	break;
  	case ' ': numSymbol = 10;
  	break;
  	case '.': numSymbol = 11;
  	break;
  	case '^': numSymbol = 12;
  	break;
  	case 'm': numSymbol = 13;
  	break;
  	case 'b': numSymbol = 14;
  	break;
  	case 'v': numSymbol = 15;
  	break;
  	default: return;
  	}

  	if(numSymbol == -1)
  		return;

  	int posY = 0;
  	int posX = 0;
  	for (uint8_t i = 0; i < size; i++ )
	{
  	  	uint8_t line;


			line = font7x14[numSymbol][i];
			if(i >= width)
			{
				posY = 8;
				posX = width;
			}
  	  		for (uint8_t j = 0; j < 8; j++)
			{
				if (line & 0x01)
				{
					LCD_DrawPixel(x + i - posX, y + posY + j, color);
				}
				else
				{
					LCD_DrawPixel(x + i - posX, y + posY + j, !color);
				}
				line >>= 1;
			}
  	}
}

// Вывод строки
void LCD_print(uint8_t x, uint8_t y, uint8_t color, char *str) 
{
  	unsigned char type = *str;
  	if(type >= 128)
  	{
	  	x = x - 3;
  	}
	while(*str)
	{
  	  	LCD_DrawChar(x, y, color, *str++);
  	  	unsigned char type = *str;
  	  	if (type >= 128)
  	  	{
			x = x + 3;
  	  	}
  	  	else
  	  	{
			x = x + 6;
  	  	}
  	}
}

void LCD_print2(uint8_t x, uint8_t y, uint8_t color, char *str)
{
	int width = 5;
  	unsigned char type = *str;
  	if(type >= 128)
  	{
	  	x = x - 3;
  	}
	while(*str)
	{
  	  	LCD_DrawChar2(x, y, color, *str++);
  	  	unsigned char type = *str;
  	  	if (type >= 128)
  	  	{
			x = x + width;
  	  	}
  	  	else
  	  	{
			x = x + 3 + width;
  	  	}
  	}
}

/*
// Вывод числовых значений
// закомментировал ибо  много памяти сжирает
void LCD_write(uint8_t x, uint8_t y, uint8_t color, float num){
  char c[10];
//	sprintf(c, "text %f\n", num);
	sprintf(c, "%5.2f", num);
  LCD_print(x, y, color, c);
}
*/
// Вывод картинки
void LCD_DrawBitmap(uint8_t x, uint8_t y, const char *bitmap, uint8_t w, uint8_t h, uint8_t color) {
	for (int16_t i=0; i < h; i++)
    {
		for (int16_t j=0; j < w; j++ )
		{
			if (bitmap[i*w + j] >= 0x01)
			{
				LCD_DrawPixel(x+j, y+i, color);
			}
		}
    }
}

// Очистка области
void LCD_ClearBitmap(uint8_t x, uint8_t y, uint8_t w, uint8_t h, uint8_t color) {
	for (int16_t i=0; i < h; i++)
    {
		for (int16_t j=0; j < w; j++ )
		{
			LCD_DrawPixel(x+j, y+i, color);
		}
    }
}
// пнициализируем дисплей
void LCD_Init(SPI_HandleTypeDef* hspi) {
  // пнициализация дисплея
	HAL_GPIO_WritePin(LCD_RESET_GPIO_PORT, LCD_RESET_PIN, GPIO_PIN_RESET); // Активируем ресет
	delay_ms(10);
	HAL_GPIO_WritePin(LCD_RESET_GPIO_PORT, LCD_RESET_PIN, GPIO_PIN_SET);   // Деактивируем ресет

	HAL_GPIO_WritePin(LCD_CS_GPIO_PORT, LCD_CS_PIN, GPIO_PIN_RESET);			 // Выбираем дисплей
	// Задержка
  	delay_ms(5);
  	HAL_GPIO_WritePin(LCD_CS_GPIO_PORT, LCD_CS_PIN, GPIO_PIN_SET);			 // Выбираем дисплей
	delay_ms(1);
	LCD_SendByte(hspi, LCD_C, 0xE2);  // Сброс чипа
	LCD_SendByte(hspi, LCD_C, 0xE2);  // Сброс чипа ещё раз, глюк аппаратного spi
hspi, 
	delay_ms(5);
  	// Устанавливаем энергию заряда сегмента
	LCD_SendByte(hspi, LCD_C, 0x3D);  // Умножитель энергии заряда
	LCD_SendByte(hspi, LCD_C, 0x02); 	// Не понятное значение умhspi, ножителя
  	// Команда и следом данные по контрастности
	LCD_SendByte(hspi, LCD_C, 0xE1);  // Additional VOP for contrast increase
 	LCD_SendByte(hspi, LCD_C, 0x90);  // from -127 thspi, o +127
  	// Устанавливаем режим работы Normal
 	LCD_SendByte(hspi, LCD_C, 0xA4);  // Power saver off

	LCD_SendByte(hspi, LCD_C, 0x2F);  // Booster ON Voltage regulator ON Voltage follover ON
	LCD_SendByte(hspi, LCD_C, 0xA0);  // Segment driver direction select: Normal
	LCD_SendByte(hspi, LCD_C, 0xAF);  // Включение дисплея
	//LCD_SendByte(LCD_C, 0xAD);
	delay_ms(10);
  	// Очищаем, обновляем

  	LCD_Clear();
  	LCD_Update(hspi);
}

void LCD_drawWiFi(int countDiv)
{
	if(countDiv > 4)
	{
#ifdef MY_DEBUG
		assert(!(countDiv > 4));
#else
		countDiv = 4;
#endif
	}
	else if(countDiv < 0)
	{
#ifdef MY_DEBUG
		assert(!(countDiv < 0));
#else
		countDiv = 0;
#endif
	}

	LCD_DrawFastVLine(5, 1, 9, 1);
	LCD_DrawFastHLine(1, 1, 9, 1);

	LCD_DrawPixel(2, 2, 1);
	LCD_DrawPixel(3, 3, 1);
	LCD_DrawPixel(4, 4, 1);

	LCD_DrawPixel(6, 4, 1);
	LCD_DrawPixel(7, 3, 1);
	LCD_DrawPixel(8, 2, 1);

	if(countDiv != 0)
	{
		int startX = 8;
		int stepX = 3;
		int startY = 7;
		int stepY = 2;
		int endY = 3;
		for(int i = 0; i < countDiv; i++)
		{
			int step = i * stepY;
			LCD_DrawFastVLine(startX + i*stepX, startY - step, endY + step, 1);
		}
	}
	else
	{
		int size = 7;
		for(int i = 0; i < size; i++)
		{
			for(int j = 0; j < size; j++)
			{
				if(i == j)
					LCD_DrawPixel(10 + i, 3 + j, 1);
				if(i + j == size - 1)
					LCD_DrawPixel(10 + i, 3 + j, 1);
			}
		}
	}
}

void LCD_ClearWifi() {
	int x = 1;
	int y = 1;
	int w = 18;
	int h = 11;
	int color = 0;
	for (int16_t i=0; i < h; i++)
    {
		for (int16_t j=0; j < w; j++ )
		{
			LCD_DrawPixel(x+j, y+i, color);
		}
    }
}

void LCD_drawStream(SPI_HandleTypeDef* hspi)
{
  	for(int i = 0; i < 6; i++)
  	{
  		int stepY = 10;
  		for(int j = 0; j < 4; j++)
  		{
  			LCD_DrawFastVLine(j*2 ,2 + j*2 + i*stepY, 4 + j*2, 1);
  			LCD_Update(hspi);
  		}

  		for(int j = 3; j >= 0; j--)
  		{
  			LCD_DrawFastVLine(16 - j*2 , 2 + j*2 + i*stepY, 4 + j*2, 1);
  			LCD_Update(hspi);
  		}


  	}
  	return;
}


void LCD_Battery(int countDiv)
{
	if(countDiv > 4)
	{
#ifdef MY_DEBUG
		assert(!(countDiv > 4));
#else
		countDiv = 4;
#endif
	}
	else if(countDiv < 0)
	{
#ifdef MY_DEBUG
		assert(!(countDiv < 0));
#else
		countDiv = 1;
#endif
	}

	LCD_DrawRect(96-18, 2, 15, 8, 1);
	LCD_DrawPixel(96-2, 3, 1);
	LCD_DrawPixel(96-2, 8, 1);
	LCD_DrawPixel(96-3, 3, 1);
	LCD_DrawPixel(96-3, 8, 1);

	LCD_DrawPixel(96-2, 3, 1);
	LCD_DrawPixel(96-2, 4, 1);
	LCD_DrawPixel(96-2, 5, 1);
	LCD_DrawPixel(96-2, 6, 1);
	LCD_DrawPixel(96-2, 7, 1);
	LCD_DrawPixel(96-2, 8, 1);

	int offsetX = 16;
	for(int i = 0; i < countDiv; i++)
	{
		LCD_FillRect(LCD_X - offsetX + i*3, 4, 2, 4, 1);
	}
}

void LCD_ClearBattery()
{
	int offsetX = 18;
	int x = LCD_X - offsetX;
	int y = 1;
	int w = 17;
	int h = 9;
	int color = 0;
	for (int16_t i=0; i < h; i++)
    {
		for (int16_t j=0; j < w; j++ )
		{
			LCD_DrawPixel(x+j, y+i, color);
		}
    }
}

int flag = 0;

void LCD_testDisplay(SPI_HandleTypeDef* hspi)
{
	//LCD_Clear();
	LCD_print(0, 30, 1, "17:34 24/03/2021");
	LCD_Update(hspi);

	LCD_ClearWifi();
	LCD_drawWiFi(4);

	LCD_DrawBitmap(19, 1, fillDrop, 9, 9, 1);
	LCD_DrawBitmap(27, 1, unFillDrop, 9, 9, 1);
	LCD_DrawBitmap(36, 1, halfFillDrop, 9, 9, 1);

	LCD_ClearBattery();
	LCD_Battery(4);

	LCD_Update(hspi);
	/*if(flag == 0)
	{
		LCD_DrawBitmap(19, 1, fillDrop, 9, 9, 1);
		LCD_DrawBitmap(27, 1, unFillDrop, 9, 9, 1);
		LCD_DrawBitmap(36, 1, halfFillDrop, 9, 9, 1);
		flag = 1;
	}
	else
	{
		LCD_ClearBitmap(19, 1, 7, 7, 0);
		LCD_ClearBitmap(27, 1, 7, 7, 0);
		LCD_ClearBitmap(36, 1, 7, 7, 0);
		flag = 0;
	}
	LCD_Update(hspi);
	*/
	HAL_Delay(1000);
	//Test Wifi
	LCD_ClearWifi();
	LCD_drawWiFi(4);
	LCD_print(0, 30, 1, "4 division");
	LCD_Update(hspi);
	HAL_Delay(1000);
	LCD_ClearWifi();
	LCD_drawWiFi(3);
	LCD_print(0, 30, 1, "3 division");
	LCD_Update(hspi);
	HAL_Delay(1000);
	LCD_ClearWifi();
	LCD_drawWiFi(2);
	LCD_print(0, 30, 1, "2 division");
	LCD_Update(hspi);
	HAL_Delay(1000);
	LCD_ClearWifi();
	LCD_drawWiFi(1);
	LCD_print(0, 30, 1, "1 division");
	LCD_Update(hspi);
	HAL_Delay(1000);
	LCD_ClearWifi();
	LCD_drawWiFi(0);
	LCD_print(0, 30, 1, "not signal");
	LCD_Update(hspi);
	HAL_Delay(1000);
	// Test Battery
	LCD_ClearBattery();
	LCD_Battery(4);
	LCD_print(0, 30, 1, "4 division");
	LCD_Update(hspi);
	HAL_Delay(1000);
	LCD_ClearBattery();
	LCD_Battery(3);
	LCD_print(0, 30, 1, "3 division");
	LCD_Update(hspi);
	HAL_Delay(1000);
	LCD_ClearBattery();
	LCD_Battery(2);
	LCD_print(0, 30, 1, "2 division");
	LCD_Update(hspi);
	HAL_Delay(1000);

	LCD_ClearBattery();
	LCD_Battery(1);
	LCD_print(0, 30, 1, "1 division");
	LCD_Update(hspi);
	HAL_Delay(1000);

	for(int i = 0; i < 3; i++)
	{
		LCD_ClearBattery();
		LCD_Update(hspi);
		HAL_Delay(500);
		LCD_Battery(1);
		LCD_Update(hspi);
		HAL_Delay(500);
	}

	HAL_Delay(1000);
}

void LCD_testBigFont(SPI_HandleTypeDef* hspi)
{
	LCD_print2(2, 30, 1, "0123.4567 m^");
	LCD_print2(40, 50, 1, "v89.12b");
	LCD_Update(hspi);
	HAL_Delay(10000);
}
