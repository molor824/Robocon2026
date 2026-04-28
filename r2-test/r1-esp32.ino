#include <Bluepad32.h>

#define LF_MOTOR_DIR 2
#define LF_MOTOR_PWM 17
#define LF_MOTOR_REVERSE 0

#define RF_MOTOR_DIR 4
#define RF_MOTOR_PWM 21
#define RF_MOTOR_REVERSE 0

#define LB_MOTOR_INA 19
#define LB_MOTOR_INB 18
#define LB_MOTOR_PWM 5
#define LB_MOTOR_REVERSE 0

#define LM_MOTOR_INA 27
#define LM_MOTOR_INB 14
#define LM_MOTOR_PWM 12
#define LM_MOTOR_REVERSE 0

#define RB_MOTOR_INA 25
#define RB_MOTOR_INB 13
#define RB_MOTOR_PWM 26
#define RB_MOTOR_REVERSE 0

#define RM_MOTOR_INA 32
#define RM_MOTOR_INB 15
#define RM_MOTOR_PWM 33
#define RM_MOTOR_REVERSE 0

#define SPEED_MULTIPLIER -128
#define TURN_SPEED_MULTIPLIER -64

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
    int output_pins[] = {
        LF_MOTOR_DIR, LF_MOTOR_PWM,
        RF_MOTOR_DIR, RF_MOTOR_PWM,

        LM_MOTOR_INA, LM_MOTOR_INB, LM_MOTOR_PWM,
        RM_MOTOR_INA, RM_MOTOR_INB, RM_MOTOR_PWM,

        LB_MOTOR_INA, LB_MOTOR_INB, LB_MOTOR_PWM,
        RB_MOTOR_INA, RB_MOTOR_INB, RB_MOTOR_PWM,
    };
    for (int i = 0; i < sizeof(output_pins) / sizeof(int); i++) {
        pinMode(output_pins[i], OUTPUT);
    }

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
            int forward = (controller->axisRY() - 4) * SPEED_MULTIPLIER / 512;
            int side = (controller->axisX() - 4) * SPEED_MULTIPLIER / 512;
            int left = forward - side;
            int right = forward + side;
            // int left = (controller->axisY() - 4) * SPEED_MULTIPLIER / 512;
            // int right = (controller->axisRY() - 4) * SPEED_MULTIPLIER / 512;
            lf_motor_speed(left);
            lm_motor_speed(left);
            lb_motor_speed(left);
            rf_motor_speed(right);
            rm_motor_speed(right);
            rb_motor_speed(right);

            printf("left: %d, right: %d\n", left, right);
        }
    }
    delay(10);
}