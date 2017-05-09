/*
 Name:		EDA2015.ino
 Created:	2017/5/9 12:23:55
 Author:	zaxs0130
*/

#include <PWM.h>
#include <LiquidCrystal_I2C.h>
#include <Queue.h>
#include <math.h>
#include <stdio.h>

typedef unsigned int uint;
typedef unsigned long ulong;
typedef unsigned char uchr;

#define SampleCnt   1024

#define BatVolPin   A0
#define PwrVolPin   A1
#define CurrentPin  A2
#define CurSetPin   A3

#define SDA         A4
#define SCL         A5

#define ModePin     2
#define UpPin       3
#define DownPin     4

#define PWM1 9
#define PWM2 10

#define MaxSampleVol 5.0
#define MaxBatVol   25
#define MaxPwrVol   35
#define MaxCurrent  2.5
const double CurrentGain = MaxCurrent / (MaxSampleVol / 5.0 * 1023);
const double BatVolGain = MaxBatVol / (MaxSampleVol / 5.0 * 1023);
const double PwrVolGain = MaxPwrVol / (MaxSampleVol / 5.0 * 1023);

//LCD1602
LiquidCrystal_I2C Lcd(0x27, 16, 2);
char lcdStr[17];
enum { Input = 0, Output = 1, Auto = 2 } mode;
const char modeStr[3][8] = { " Input", "OUTPUT"," Auto " };

/*
显示屏样式：
0123456789012345
******************
* 1.024A  1.024A *0x80
* OUTPUT  18.75V *0xC0
******************
*/

double getRMS(uchr pin, double gain);
void changeMode();
void inputMode();
void outputMode();
void autoMode();
// 目标电流，当前电流，电池电压
void print2LCD(double target, double cur, double vol);

void setup()
{
	pinMode(BatVolPin, INPUT);
	pinMode(PwrVolPin, INPUT);
	pinMode(CurrentPin, INPUT);
	pinMode(ModePin, INPUT_PULLUP);
	pinMode(UpPin, INPUT_PULLUP);
	pinMode(DownPin, INPUT_PULLUP);

	attachInterrupt(ModePin, changeMode, LOW);

	Lcd.init();
	Lcd.backlight();//开启背光
	Lcd.noBlink();//无光标
	Lcd.setCursor(0, 0);
}

void loop()
{
	switch (mode)
	{
	case Input:
		inputMode();
		break;
	case Output:
		outputMode();
		break;
	case Auto:
		autoMode();
		break;
	default:
		mode = Auto;
		autoMode();
		break;
	}
}

double getRMS(uchr pin, double gain)
{
	double s = 0, t;
	for (int i = 0; i < SampleCnt; ++i)
	{
		t = analogRead(pin) * gain;
		s += t * t;
	}
	return sqrt(s / SampleCnt);
}

void changeMode()
{
	uint t = micros();
	if (digitalRead(ModePin) == HIGH)
	{
		return;
	}
	while (micros() - t < 10);
	if (digitalRead(ModePin) == LOW)
	{
		switch (mode)
		{
		case Input:
			mode = Output;
			break;
		case Output:
			mode = Auto;
			break;
		case Auto:
			mode = Input;
			break;
		default:
			mode = Input;
			break;
		}
	}
}

void print2LCD(double target, double cur, double vol)
{
	sprintf(lcdStr, " %.3fA  %.3fA", target, cur);
	Lcd.setCursor(0, 0);
	Lcd.print(lcdStr);
	sprintf(lcdStr, " %s  %.2f", modeStr[mode], vol);
	Lcd.setCursor(0, 1);
	Lcd.print(lcdStr);
}

