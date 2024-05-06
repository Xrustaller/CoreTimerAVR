#include <Wire.h>
#include "DTM1650.h"

DTM1650::DTM1650(void) {
	localI2CDisplayAddress = DTM1650_DISPLAY_BASE;
	localI2CControlAddress = DTM1650_CONTROL_BASE;
	localNumDigits = DTM1650_NUM_DIGITS;
}

void DTM1650::send_control(unsigned char data, uint8_t segment) {
	segment = segment % DTM1650_NUM_DIGITS;
	Wire.beginTransmission(localI2CControlAddress + segment);
	Wire.write(data);
	Wire.endTransmission();
	DTM1650_CONTROL_STORE[segment] = data;
}

void DTM1650::send_control(unsigned char data) {
	send_control(data, 0);
}

void DTM1650::send_digit(unsigned char data, uint8_t segment) {
	segment = segment % DTM1650_NUM_DIGITS;
	data = data & 0x7F;
	data = data | (DTM1650_DIGITS_STORE[segment] & 0x80);
	Wire.beginTransmission(localI2CDisplayAddress + segment);
	Wire.write(data);
	Wire.endTransmission();
	DTM1650_DIGITS_STORE[segment] = data;
}

void DTM1650::send_digit(unsigned char data) {
	send_digit(data, 0);
}

void DTM1650::set_dot(uint8_t segment, bool onoff) {
	segment = segment % DTM1650_NUM_DIGITS;
	unsigned char digit = DTM1650_DIGITS_STORE[segment];
	digit = (onoff) ? digit | 0x80 : digit & 0x7f;
	Wire.beginTransmission(localI2CDisplayAddress + segment);
	Wire.write(digit);
	Wire.endTransmission();
	DTM1650_DIGITS_STORE[segment] = digit;
}

void DTM1650::set_brightness(unsigned char brightness, uint8_t segment) {
	if (brightness > 0x07) {
		brightness = 0x07;
	}
	segment = segment % DTM1650_NUM_DIGITS;
	send_control(((DTM1650_CONTROL_STORE[segment] & DTM1650_MASK_BRIGHTNESS) | (brightness << 4)), segment);
}

void DTM1650::set_brightness(unsigned char brightness) {
	set_brightness(brightness, 0);
}

void DTM1650::clear_display(void) {
	for (uint8_t i = 0; i < localNumDigits; i++) {
		send_digit(0x00, i);
		DTM1650_DIGITS_STORE[i] = 0x00;
	}
}

void DTM1650::display_on(void) {
	for (uint8_t i = 0; i < localNumDigits; i++) {
		send_control(((DTM1650_CONTROL_STORE[i] & DTM1650_MASK_ONOFF) | DTM1650_BIT_ONOFF), i);
	}
}

void DTM1650::display_off(void) {
	for (uint8_t i = 0; i < localNumDigits; i++) {
		send_control((DTM1650_CONTROL_STORE[i] & DTM1650_MASK_ONOFF), i);
	}
}

void DTM1650::colon_on(void) {
	send_control((DTM1650_CONTROL_STORE[0] & DTM1650_MASK_COLON) | DTM1650_BIT_COLON);
}

void DTM1650::colon_off(void) {
	send_control(DTM1650_CONTROL_STORE[0] & DTM1650_MASK_COLON);
}

void DTM1650::write_num(uint16_t num, uint8_t position) {
	position = position % DTM1650_NUM_DIGITS;
	num = num & 0x0F;
	send_digit(DTM1650_Digit_Table[num], position);
}

void DTM1650::write_num(float num) {
	float temp1;
	uint16_t temp;
	uint8_t digit;
	set_dot(1, false);
	set_dot(2, false);
	if (num < 0)
	{
		temp1 = num * -100;
		temp = (uint16_t)temp1;
		digit = temp % 10;
		send_digit(DTM1650_Digit_Table[digit], 3);
		temp = temp / 10;
		digit = temp % 10;
		send_digit(DTM1650_Digit_Table[digit], 2);
		set_dot(1, true);
		temp = temp / 10;
		digit = temp % 10;
		send_digit(DTM1650_Digit_Table[digit], 1);
		
		send_digit(DTM1650_S_MINUS, 0);
	}
	else
	{
		if(num < 100)
		{
			temp1 = num * 100;
			temp = (uint16_t)temp1;
			digit = temp % 10;
			send_digit(DTM1650_Digit_Table[digit], 3);
			temp = temp / 10;
			digit = temp % 10;
			send_digit(DTM1650_Digit_Table[digit], 2);
			set_dot(1, true);
			temp = temp / 10;
			digit = temp % 10;
			send_digit(DTM1650_Digit_Table[digit], 1);
			temp = temp / 10;
			digit = temp % 10;
			send_digit(DTM1650_Digit_Table[digit], 0);
		}
		else if (num < 1000)
		{
			temp1 = num * 10;
			temp = (uint16_t)temp1;
			digit = temp % 10;
			send_digit(DTM1650_Digit_Table[digit], 3);
			temp = temp / 10;
			digit = temp % 10;
			send_digit(DTM1650_Digit_Table[digit], 2);
			temp = temp / 10;
			digit = temp % 10;
			send_digit(DTM1650_Digit_Table[digit], 1);
			set_dot(2, true);
			temp = temp / 10;
			digit = temp % 10;
			send_digit(DTM1650_Digit_Table[digit], 0);
		}
		else
		{
			temp1 = num;
			temp = (uint16_t)temp1;
			digit = temp % 10;
			send_digit(DTM1650_Digit_Table[digit], 3);
			temp = temp / 10;
			digit = temp % 10;
			send_digit(DTM1650_Digit_Table[digit], 2);
			temp = temp / 10;
			digit = temp % 10;
			send_digit(DTM1650_Digit_Table[digit], 1);
			temp = temp / 10;
			digit = temp % 10;
			send_digit(DTM1650_Digit_Table[digit], 0);
		}
	}
}

void DTM1650::write_num(uint16_t num) {
	uint8_t digit;
	digit = num % 10;
	send_digit(DTM1650_Digit_Table[digit], 3);
	num = num / 10;
	digit = num % 10;
	send_digit(DTM1650_Digit_Table[digit], 2);
	num = num / 10;
	digit = num % 10;
	send_digit(DTM1650_Digit_Table[digit], 1);
	num = num / 10;
	digit = num % 10;
	send_digit(DTM1650_Digit_Table[digit], 0);
}

void DTM1650::write_void_num(uint16_t num) {
	uint8_t digit;

	digit = num % 10;
	send_digit(DTM1650_Digit_Table[digit], 3);

	if (num < 10)
	{
		return;
	}

	num = num / 10;
	digit = num % 10;
	send_digit(DTM1650_Digit_Table[digit], 2);

	if (num < 10)
	{
		return;
	}

	num = num / 10;
	digit = num % 10;
	send_digit(DTM1650_Digit_Table[digit], 1);

	if (num < 10)
	{
		return;
	}

	num = num / 10;
	digit = num % 10;
	send_digit(DTM1650_Digit_Table[digit], 0);
}

void DTM1650::write_time(uint16_t minute, uint8_t second) {
	uint8_t digit;
	digit = second % 10;
	send_digit(DTM1650_Digit_Table[digit], 3);
	second = second / 10;
	digit = second % 10;
	send_digit(DTM1650_Digit_Table[digit], 2);
	set_dot(1, true);
	digit = minute % 10;
	send_digit(DTM1650_Digit_Table[digit], 1);
	minute = minute / 10;
	digit = minute % 10;
	send_digit(DTM1650_Digit_Table[digit], 0);
}

void DTM1650::write_longtime(uint16_t minute, uint8_t second) {
	uint8_t digit;
	if (minute > 99)
	{
		set_dot(1, false);
		set_dot(2, true);
		second = second / 10;
		digit = second % 10;
		send_digit(DTM1650_Digit_Table[digit], 3);
		digit = minute % 10;
		send_digit(DTM1650_Digit_Table[digit], 2);
		minute = minute / 10;
	}
	else
	{
		set_dot(1, true);
		set_dot(2, false);
		digit = second % 10;
		send_digit(DTM1650_Digit_Table[digit], 3);
		second = second / 10;
		digit = second % 10;
		send_digit(DTM1650_Digit_Table[digit], 2);
	}
	digit = minute % 10;
	send_digit(DTM1650_Digit_Table[digit], 1);
	minute = minute / 10;
	digit = minute % 10;
	send_digit(DTM1650_Digit_Table[digit], 0);
}

void DTM1650::init(void) {
	for (uint8_t i = 0; i < localNumDigits; i++) {
		DTM1650_DIGITS_STORE[i] = 0x00;
		DTM1650_CONTROL_STORE[i] = 0x00;
	}
	clear_display();
	display_on();
}
