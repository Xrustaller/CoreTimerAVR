#define MIN_TIME 3 // Минимальное время съема данных (сек)
#define MAX_TIME 0 // Максимальное время съема данных (0 это максимум 999 сек)

#define BTN_PIN_START_RESET 2  // Кнопка СТАРТ-СТОП
#define BTN_PIN_PLUS 3  // Кнопка ПЛЮС
#define BTN_PIN_MINUS 4  // Кнопка МИНУС

#define PIN_AUTO 5  // Переключатель включения по включению таймера (Включение: подтянуть к +5V, Выключение: Подтянуть к GND)

#define PIN_T1 6  // От переключателя выбора (при выборе T1 - замыкание на землю) 
#define PIN_T2 7  // От переключателя выбора (при выборе T2 - замыкание на землю)

#define PIN_T1_ON 8  // От секундомера T1 при его включении (Сделать внешнюю подтяжку на землю, когда таймер запущен должен подаваться сигнал +5V )
#define PIN_T2_ON 11  // От таймера T2 при его включении (Сделать внешнюю подтяжку на землю, когда таймер запущен должен подаваться сигнал +5V )

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

Modbus bus(BUS_ID);
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

void timer_tick()
{
    bitWrite(temp[0], 0, digitalRead(PIN_AUTO));
    bitWrite(temp[0], 1, !digitalRead(PIN_T1));
    bitWrite(temp[0], 2, !digitalRead(PIN_T2));
    bitWrite(temp[0], 5, digitalRead(PIN_T1_ON));
    bitWrite(temp[0], 6, digitalRead(PIN_T2_ON));

    // Таймер запущен и зажата кнопка стоп
    if (is_start && button_start.isHolded())
    {
        tone(BEEP_PIN, 500, 1000);
        is_start = false;
        bool_sim = false;
        bitWrite(temp[0], 4, false);
        bitWrite(temp[0], 7, false);
        timer_sim = millis();
        // Остановка эксперимента
        return;
    }
    // Таймер запущен автоматически и выключили таймер
    if (is_auto_start && (digitalRead(PIN_T1) || !digitalRead(PIN_T1_ON)) && (digitalRead(PIN_T2) || !digitalRead(PIN_T2_ON))) {
        // Если таймеры выключены
        tone(BEEP_PIN, 500, 1000);
        is_auto_start = false;
        bool_sim = false;
        bitWrite(temp[0], 3, false);
        bitWrite(temp[0], 7, false);
        timer_sim = millis();
        // Остановка эксперимента
        button_start.resetStates();
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
        return;
    }

    if (button_minus.isClick()) {
        if (temp[1] > MIN_TIME) {
            temp[1]--;
        }
        if (temp[1] < 10) {
            display.send_digit(0, 2);
        }
        else if (temp[1] < 100) {
            display.send_digit(0, 1);
        }
        return;
    }

    // Автоматический запуск
    if (digitalRead(PIN_AUTO)) {
        button_start.resetStates();
        if ((!digitalRead(PIN_T1) && digitalRead(PIN_T1_ON)) || (!digitalRead(PIN_T2) && digitalRead(PIN_T2_ON))) {
            // Включен один из таймеров
            is_auto_start = true;
            bitWrite(temp[0], 3, true);
            bitWrite(temp[0], 7, true);
            tone(BEEP_PIN, 500, 1000);
            // Запуск эксперимента
        }
        return;
    }
    // Запуск по кнопке
    if (button_start.isHolded()) {
        is_start = true;
        bitWrite(temp[0], 4, true);
        bitWrite(temp[0], 7, true);
        tone(BEEP_PIN, 500, 1000);
        // Запуск эксперимента
    }
}

void loop() {

    button_tick();

    timer_tick();

    state = bus.poll(temp, 2);

    draw();
}
