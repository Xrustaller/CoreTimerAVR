#ifndef DTM1650_H
#define DTM1650_H

#define DTM1650_VERSION			1.0
#define DTM1650_CONTROL_BASE		0x24	// Address of the control register of the left-most digit
#define DTM1650_DISPLAY_BASE		0x34	// Address of the left-most digit 
#define DTM1650_NUM_DIGITS		4	// 4 segments

#define DTM1650_MASK_BRIGHTNESS		0b10001111
#define DTM1650_MASK_COLON			0b11110111
#define DTM1650_BIT_COLON			0b00001000
#define DTM1650_MASK_ONOFF			0b11111110
#define DTM1650_BIT_ONOFF			0b00000001

#define DTM1650_BRIGHTNESS_MAX		8
#define DTM1650_BRIGHTNESS_MIN		1


#define DTM1650_S_MINUS 0x40

/*
Brightness settings
MSB				LSB
B7 B6 B5 B4 B3 B2 B1 B0 Explanation
 ×  0  0  0     ×  ×	8 brightness
 ×  0  0  1     ×  ×	1 brightness
 ×  0  1  0     ×  ×	2 brightness
 ×  0  1  1     ×  ×	3 Brightness
 ×  1  0  0     ×  ×	4 brightness
 ×  1  0  1     ×  ×	5 brightness
 ×  1  1  0     ×  ×	6 brightness
 ×  1  1  1     ×  ×	7 brightness
On / off the display position
MSB				LSB
B7 B6 B5 B4 B3 B2 B1 B0 Explanation
 ×              ×  ×  0 Off Display
 ×              ×  ×  1 On Display
 */

 /*
	 0x01
 0x20		0x02
	 0x40
 0x10		0x04
	 0x08
			 0x80
  */

const char DTM1650_Digit_Table[] = {
	0x3F,	// 0
	0x06,	// 1
	0x5B,	// 2
	0x4F,	// 3
	0x66,	// 4
	0x6D,	// 5
	0x7D,	// 6
	0x07,	// 7
	0x7F,	// 8
	0x6F,	// 9

	0x77,
	0x7C,
	0x39,
	0x5E,
	0x79,
	0x71
};

class DTM1650 {

private:

	uint8_t localI2CDisplayAddress;
	uint8_t localI2CControlAddress;
	uint8_t localNumDigits = DTM1650_NUM_DIGITS;
	unsigned char DTM1650_DIGITS_STORE[DTM1650_NUM_DIGITS];
	unsigned char DTM1650_CONTROL_STORE[DTM1650_NUM_DIGITS];

public:

	DTM1650(void);

	void init(void);
	void send_control(unsigned char Command);
	void send_control(unsigned char Command, uint8_t Segment);
	void send_digit(unsigned char Data);
	void send_digit(unsigned char Data, uint8_t Segment);
	void set_dot(uint8_t Segment, bool onoff);
	void set_brightness(unsigned char Brightness);
	void set_brightness(unsigned char Brightness, uint8_t Segment);
	void display_on(void);
	void display_off(void);
	void colon_on(void);
	void colon_off(void);
	void clear_display(void);
	void write_num(float num);
	void write_num(uint16_t num);
	void write_num(uint16_t num, uint8_t position);
	void write_void_num(uint16_t num);
	void write_time(uint16_t minute, uint8_t second);
	void write_longtime(uint16_t minute, uint8_t second);

};

#endif