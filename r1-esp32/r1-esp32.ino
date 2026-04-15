#include <Bluepad32.h>

#define LF_MOTOR_DIR 32
#define LF_MOTOR_PWM 33
#define LF_MOTOR_REVERSE 0

#define RF_MOTOR_DIR 25
#define RF_MOTOR_PWM 26
#define RF_MOTOR_REVERSE 0

#define LB_MOTOR_INA 27
#define LB_MOTOR_INB 14
#define LB_MOTOR_PWM 12
#define LB_MOTOR_REVERSE 0

#define LM_MOTOR_INA 17
#define LM_MOTOR_INB 16
#define LM_MOTOR_PWM 4
#define LM_MOTOR_REVERSE 0

#define RB_MOTOR_INA 19
#define RB_MOTOR_INB 18
#define RB_MOTOR_PWM 5
#define RB_MOTOR_REVERSE 0

#define RM_MOTOR_INA 23
#define RM_MOTOR_INB 22
#define RM_MOTOR_PWM 2
#define RM_MOTOR_REVERSE 0

#define SPEED_MULTIPLIER 256

// Range [-255, 255]
void motor_speed1(uint8_t dir_pin, uint8_t pwm_pin, int speed, bool reverse) {
    digitalWrite(dir_pin, speed >= 0 ? !reverse : reverse);
    analogWrite(pwm_pin, constrain(speed >= 0 ? speed : -speed, 0, 255));
}
void motor_speed2(uint8_t ina_pin, uint8_t inb_pin, uint8_t pwm_pin, int speed, bool reverse) {
    if (speed == 0) {
        digitalWrite(ina_pin, 1);
        digitalWrite(inb_pin, 1);
    } else {
        digitalWrite(ina_pin, speed > 0 ? !reverse : reverse);
        digitalWrite(inb_pin, speed > 0 ? reverse : !reverse);
    }
    analogWrite(pwm_pin, constrain(speed >= 0 ? speed : -speed, 0, 255));
}
void lf_motor_speed(int speed) {
    motor_speed1(LF_MOTOR_DIR, LF_MOTOR_PWM, speed, LF_MOTOR_REVERSE);
}
void rf_motor_speed(int speed) {
    motor_speed1(RF_MOTOR_DIR, RF_MOTOR_PWM, speed, RF_MOTOR_REVERSE);
}
void lm_motor_speed(int speed) {
    motor_speed2(LM_MOTOR_INA, LM_MOTOR_INB, LM_MOTOR_PWM, speed, LM_MOTOR_REVERSE);
}
void rm_motor_speed(int speed) {
    motor_speed2(RM_MOTOR_INA, RM_MOTOR_INB, RM_MOTOR_PWM, speed, RM_MOTOR_REVERSE);
}
void lb_motor_speed(int speed) {
    motor_speed2(LB_MOTOR_INA, LB_MOTOR_INB, LB_MOTOR_PWM, speed, LB_MOTOR_REVERSE);
}
void rb_motor_speed(int speed) {
    motor_speed2(RB_MOTOR_INA, RB_MOTOR_INB, RB_MOTOR_PWM, speed, RB_MOTOR_REVERSE);
}
void motor_failsafe() {
    digitalWrite(LF_MOTOR_DIR, 0);
    analogWrite(LF_MOTOR_PWM, 0);

    digitalWrite(RF_MOTOR_DIR, 0);
    analogWrite(RF_MOTOR_PWM, 0);

    digitalWrite(LB_MOTOR_INA, 0);
    digitalWrite(LB_MOTOR_INB, 0);
    analogWrite(LB_MOTOR_PWM, 0);

    digitalWrite(RB_MOTOR_INA, 0);
    digitalWrite(RB_MOTOR_INB, 0);
    analogWrite(RB_MOTOR_PWM, 0);
}

ControllerPtr myControllers[BP32_MAX_GAMEPADS];

// This callback gets called any time a new gamepad is connected.
// Up to 4 gamepads can be connected at the same time.
void onConnectedController(ControllerPtr ctl) {
    bool foundEmptySlot = false;
    for (int i = 0; i < BP32_MAX_GAMEPADS; i++) {
        if (myControllers[i] == nullptr) {
            Serial.printf("CALLBACK: Controller is connected, index=%d\n", i);
            // Additionally, you can get certain gamepad properties like:
            // Model, VID, PID, BTAddr, flags, etc.
            ControllerProperties properties = ctl->getProperties();
            Serial.printf("Controller model: %s, VID=0x%04x, PID=0x%04x\n", ctl->getModelName().c_str(), properties.vendor_id,
                           properties.product_id);
            myControllers[i] = ctl;
            foundEmptySlot = true;
            break;
        }
    }
    if (!foundEmptySlot) {
        Serial.println("CALLBACK: Controller connected, but could not found empty slot");
    }
}

void onDisconnectedController(ControllerPtr ctl) {
    bool foundController = false;

    for (int i = 0; i < BP32_MAX_GAMEPADS; i++) {
        if (myControllers[i] == ctl) {
            Serial.printf("CALLBACK: Controller disconnected from index=%d\n", i);
            myControllers[i] = nullptr;
            foundController = true;
            break;
        }
    }

    if (!foundController) {
        Serial.println("CALLBACK: Controller disconnected, but not found in myControllers");
    }
}

void failsafe() {
    motor_failsafe();
}

void setup() {
    pinMode(LF_MOTOR_DIR, OUTPUT);
    pinMode(LF_MOTOR_PWM, OUTPUT);

    pinMode(RF_MOTOR_DIR, OUTPUT);
    pinMode(RF_MOTOR_PWM, OUTPUT);

    pinMode(LB_MOTOR_INA, OUTPUT);
    pinMode(LB_MOTOR_INB, OUTPUT);
    pinMode(LB_MOTOR_PWM, OUTPUT);

    pinMode(RB_MOTOR_INA, OUTPUT);
    pinMode(RB_MOTOR_INB, OUTPUT);
    pinMode(RB_MOTOR_PWM, OUTPUT);

    failsafe();

    Serial.begin(115200);
    Serial.printf("Firmware: %s\n", BP32.firmwareVersion());
    const uint8_t* addr = BP32.localBdAddress();
    Serial.printf("BD Addr: %2X:%2X:%2X:%2X:%2X:%2X\n", addr[0], addr[1], addr[2], addr[3], addr[4], addr[5]);

    // Setup the Bluepad32 callbacks
    BP32.setup(&onConnectedController, &onDisconnectedController);
    BP32.enableVirtualDevice(false);
}

void loop() {
    ControllerPtr controller = nullptr;
    for (int i = 0; i < BP32_MAX_GAMEPADS; i++) {
        if (myControllers[i] && myControllers[i]->isGamepad()) {
            controller = myControllers[i];
            break;
        }
    }

    bool dataUpdated = BP32.update();
    if (dataUpdated) {
        if (!controller || !controller->isConnected()) {
            failsafe();
        } else {
            int32_t left = (controller->axisY() - 4) * SPEED_MULTIPLIER / 512;
            int32_t right = (controller->axisRY() - 4) * SPEED_MULTIPLIER / 512;
            lf_motor_speed(left);
            lm_motor_speed(left);
            lb_motor_speed(left);
            rf_motor_speed(right);
            rm_motor_speed(right);
            rb_motor_speed(right);
        }
    }
    delay(10);
}