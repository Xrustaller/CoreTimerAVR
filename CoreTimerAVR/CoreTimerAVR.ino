#define MIN_TIME 3 // Минимальное время съема данных (сек)
#define MAX_TIME 0 // Максимальное время съема данных (0 это максимум 999 сек)

#define BTN_PIN_START_RESET 2  // Кнопка СТАРТ-СТОП
#define BTN_PIN_PLUS 3  // Кнопка ПЛЮС
#define BTN_PIN_MINUS 4  // Кнопка МИНУС

#define PIN_AUTO 5  // Переключатель включения по включению таймера (Включение: подтянуть к +5V, Выключение: Подтянуть к GND)

#define PIN_T1 6  // От переключателя выбора (при выборе T1 - замыкание на землю) 
#define PIN_T2 7  // От переключателя выбора (при выборе T2 - замыкание на землю)

#define PIN_T1_ON 8  // От секундомера T1 при его включении (Сделать внешнюю подтяжку на +5V, когда таймер запущен должна быть подтяжка на землю)
#define PIN_T2_ON 11  // От таймера T2 при его включении (Сделать внешнюю подтяжку на +5V, когда таймер запущен должна быть подтяжка на землю)

#define BEEP_PIN 9  // Пин пьезо (при выборе 9 пина, 10 - недоступен из-за шим)

#define DTM1650_BRIGHTNESS 8 // Яркость дисплея (От 1 до 8)
#define SIM_WAIT 0b1110110
#define SIM_ON0 0b1010010
#define SIM_ON1 0b1100100

#define BUS_ID 1

#include <Wire.h>
#include <GyverButton.h>
#include <DTM1650.h>
#include <ModbusRtu.h>

GButton button_start(BTN_PIN_START_RESET);
GButton button_plus(BTN_PIN_PLUS);
GButton button_minus(BTN_PIN_MINUS);

Modbus bus(BUS_ID, 0);
int8_t state = 0;

DTM1650 display;

unsigned long timer_sim;
bool bool_sim = false;

bool is_start = false;
bool is_auto_start = false;

uint16_t temp[2] = { 0, 5 };

void draw() {

    if (millis() - timer_sim > 400) {
        display.write_void_num(temp[1]);

        if (is_start || is_auto_start) {
            timer_sim = millis();
            display.send_digit(0x00, 0);
            if (bool_sim) {
                display.send_digit(SIM_ON0, 0);
            }
            else {
                display.send_digit(SIM_ON1, 0);
            }
            bool_sim = !bool_sim;
        }
        else
        {
            display.send_digit(SIM_WAIT, 0);
        }
    }
}

void setup() {
    bus.begin(9600);
    Wire.begin();

    button_start.setClickTimeout(50);
    button_plus.setClickTimeout(50);
    button_minus.setClickTimeout(50);

    pinMode(PIN_AUTO, INPUT);
    pinMode(PIN_T1, INPUT_PULLUP);
    pinMode(PIN_T2, INPUT_PULLUP);
    pinMode(PIN_T1_ON, INPUT);
    pinMode(PIN_T2_ON, INPUT);

    display.init();
    display.set_brightness(DTM1650_BRIGHTNESS);
    draw();
}

void button_tick() {
    button_start.tick();
    button_plus.tick();
    button_minus.tick();
}

void timer_tick() {
	const bool auto_pin = static_cast<bool>(digitalRead(PIN_AUTO));
    const bool t1_pin = !digitalRead(PIN_T1); // Инвертирован потому что включается нулем
    const bool t2_pin = !digitalRead(PIN_T2); // Инвертирован потому что включается нулем
    const bool t1_on_pin = !digitalRead(PIN_T1_ON); // Инвертирован потому что включается нулем
    const bool t2_on_pin = !digitalRead(PIN_T2_ON); // Инвертирован потому что включается нулем

    bitWrite(temp[0], 0, auto_pin);
    bitWrite(temp[0], 1, t1_pin);
    bitWrite(temp[0], 2, t2_pin);
    bitWrite(temp[0], 5, t1_on_pin);
    bitWrite(temp[0], 6, t2_on_pin);

    if (is_start) {
	    if (button_start.isHolded()) {
            tone(BEEP_PIN, 500, 1000);
            is_start = false;
            bool_sim = false;
            bitWrite(temp[0], 4, false);
            bitWrite(temp[0], 7, false);
            timer_sim = millis();
            // Остановка эксперимента
	    }
        delay(20);
        return;
    }

	if (is_auto_start) {
		// Если выключили таймер 1 или 2
		if ((!t1_pin || !t1_on_pin) && (!t2_pin || !t2_on_pin)) {
			tone(BEEP_PIN, 500, 1000);
			is_auto_start = false;
			bool_sim = false;
			bitWrite(temp[0], 3, false);
			bitWrite(temp[0], 7, false);
			timer_sim = millis();
			// Остановка эксперимента
			button_start.resetStates();
		}
        delay(20);
        return;
	}

    // Если ничего не запущено
    if (button_plus.isClick()) {
#if MAX_TIME == 0
        if (temp[1] < 999) {
            temp[1]++;
        }
#else
        if (temp[1] < MAX_TIME) {
            temp[1]++;
        }
#endif
    }
    else if (button_minus.isClick()) {
        if (temp[1] > MIN_TIME) {
            temp[1]--;
        }
        if (temp[1] < 10) {
            display.send_digit(0, 2);
        }
        else if (temp[1] < 100) {
            display.send_digit(0, 1);
        }
    }

    // Запуск по кнопке
    if (button_start.isHolded()) {
        is_start = true;
        bitWrite(temp[0], 4, true);
        bitWrite(temp[0], 7, true);
        tone(BEEP_PIN, 500, 1000);
        // Запуск эксперимента
        delay(20);
    }

    // Автоматический запуск
    if (auto_pin) {
        if ((t1_pin && t1_on_pin) || (t2_pin && t2_on_pin)) {
            // Включен один из таймеров
            is_auto_start = true;
            bitWrite(temp[0], 3, true);
            bitWrite(temp[0], 7, true);
            tone(BEEP_PIN, 500, 1000);
            // Запуск эксперимента
        }
        delay(20);
    }
}

void loop() {

    button_tick();

    timer_tick();

    state = bus.poll(temp, 2);

    draw();

    delay(20);
}
