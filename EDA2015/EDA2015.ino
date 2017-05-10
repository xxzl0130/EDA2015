/*
 Name:		EDA2015.ino
 Created:	2017/5/9 12:23:55
 Author:	zaxs0130
*/

#include <PWM.h>
#include <LiquidCrystal_I2C.h>
#include <Queue.h>
#include <PID.h>
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

#define PWM1		9
#define PWM2		10
#define PWM_RES		65536L
#define PWM_FREQ	10000L

#define MaxSampleVol 5.0
#define MaxBatVol   25
#define MaxPwrVol   35
#define MaxCurrent  2.5
#define BatVolTh	24

const double CurrentGain = MaxCurrent / (MaxSampleVol / 5.0 * 1023);
const double BatVolGain = MaxBatVol / (MaxSampleVol / 5.0 * 1023);
const double PwrVolGain = MaxPwrVol / (MaxSampleVol / 5.0 * 1023);

//LCD1602
LiquidCrystal_I2C Lcd(0x27, 16, 2);
char lcdStr[17];
enum { Input = 0, Output = 1, Auto = 2 } mode;
const char modeStr[3][8] = { " Input", "OUTPUT"," Auto " };

/*
��ʾ����ʽ��
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
// Ŀ���������ǰ��������ص�ѹ
void print2LCD(double target, double cur, double vol);
bool keyPressed(uchr pin, uchr val = LOW);
template <typename T>

void setup()
{
	pinMode(BatVolPin, INPUT);
	pinMode(PwrVolPin, INPUT);
	pinMode(CurrentPin, INPUT);
	pinMode(ModePin, INPUT);
	pinMode(UpPin, INPUT);
	pinMode(DownPin, INPUT);

	attachInterrupt(ModePin, changeMode, LOW);
	InitTimersSafe();
	SetPinFrequency(PWM1, PWM_FREQ);
	SetPinFrequency(PWM2, PWM_FREQ);

	Lcd.init();
	Lcd.backlight();//��������
	Lcd.noBlink();//�޹��
	Lcd.setCursor(0, 0);
}

void loop()
{
	// �л�ģʽ
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

// ��ȡ����������ֵ��ȡSampleCnt�β���
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

// �жϷ��������ı�ģʽ
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

// �����1602,
void print2LCD(double target, double cur, double vol)
{
	sprintf(lcdStr, " %.3fA  %.3fA", target, cur);
	Lcd.setCursor(0, 0);
	Lcd.print(lcdStr);
	sprintf(lcdStr, " %s  %.2f", modeStr[mode], vol);
	Lcd.setCursor(0, 1);
	Lcd.print(lcdStr);
}

bool keyPressed(uchr pin, uchr val)
{
	if (digitalRead(pin) == val)
	{
		delay(10);
		if (digitalRead(pin) == val)
		{
			return true;
		}
	}
	return false;
}

// ���ģʽ
void inputMode()
{
	PID pid(100, 1, 1, 2, -2, PWM_RES);
	long pos = 0,det;
	double current = 0.0, target = 1.0,batVol;
	while (mode == Input)
	{
		// ��鰴��
		if (keyPressed(UpPin))
		{
			target += 0.05;
		}
		if (keyPressed(DownPin))
		{
			target -= 0.05;
		}
		target = constrain(target, 1.0, 2.0);
		// ����ص�ѹ����
		batVol = analogRead(BatVolPin) * BatVolGain;
		if (batVol > BatVolTh)
		{
			// �ر�����
			pos = 0;
			current = 0.0;
		}
		else
		{
			current = getRMS(CurrentPin, CurrentGain);
			det = pid.update(target - current, current);
			// ����޷�
			pos = constrain(det + pos, 0, PWM_RES);
			pwmWriteHR(PWM1, pos);
		}
		print2LCD(target, current, batVol);
	}
}

void outputMode()
{

}

void autoMode()
{

}