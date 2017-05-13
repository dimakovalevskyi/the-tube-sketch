/*
    supported commands:
    TURN_ON                     1
    TURN_OFF                    0
    SET_MODE:MODE_SIMPLE        3:0            3:2
    SET_COLOR:125,1,63          4:125:1:63
    SET_SPEED:5                 5:1            5:15
*/
#define TURN_ON 1
#define TURN_OFF 0
#define SET_MODE 3
#define SET_COLOR 4
#define SET_SPEED 5

//pins for distance sensor
#define TRIG_PIN 7
#define ECHO_PIN 8

//pin for sound notifications
#define SOUND_PIN A0

//led pins
#define RED_PIN 3
#define GREEN_PIN 5
#define BLUE_PIN 6

//For enable working using RX/TX pins, replace for Serial1
#define mySerial Serial

//all available modes
#define MODE_SIMPLE 0
#define MODE_RAINBOW 1
#define MODE_BLINK 2
#define MODE_COPS 3
#define MODE_DISCO 4

#define NEED_FOR_SWITCH 4
int needForSwitch = NEED_FOR_SWITCH;

int counter = 0;

int changeSpeed = 5;

int currentMode = 0;
bool isTurnedOn = false;

//physical value on pin
//set every tick
int currentValueRed = 0;
int currentValueGreen = 0;
int currentValueBlue = 0;

//target of color, 
int targetValueRed = 0;
int targetValueGreen = 0;
int targetValueBlue = 0;

//used only for modes: MODE_SIMPLE and MODE_BLINK
int currentColorRed = 255;
int currentColorGreen = 255;
int currentColorBlue = 255;

void setMode (int newMode) {
    currentMode = newMode;
    
    mySerial.print("Mode was set to: ");
    mySerial.println(currentMode);
}

void applyColors() {
    targetValueRed = currentColorRed;
    targetValueGreen = currentColorGreen;
    targetValueBlue = currentColorBlue;
}

void turnOn() {
    isTurnedOn = true;
    mySerial.println("Turned on");
}
void turnOff() {
    isTurnedOn = false;
    mySerial.println("Turned off");

    targetValueRed = 0;
    targetValueGreen = 0;
    targetValueBlue = 0;
}

void checkForCommands() {
    int command;
    int subCommand;

    if (mySerial.available()) {
        command = mySerial.parseInt();

        if (command == TURN_ON && !isTurnedOn) turnOn();
        if (command == TURN_OFF && isTurnedOn) turnOff();

        if (command == SET_MODE) {
            subCommand = mySerial.parseInt();
            if (subCommand == MODE_SIMPLE) setMode(MODE_SIMPLE);
            if (subCommand == MODE_BLINK) setMode(MODE_BLINK);
        }

        if (command == SET_COLOR) {
            currentColorRed = mySerial.parseInt();
            currentColorGreen = mySerial.parseInt();
            currentColorBlue = mySerial.parseInt();
            
            if (currentColorRed < 0) currentColorRed = 0;
            if (currentColorGreen < 0) currentColorGreen = 0;
            if (currentColorBlue < 0) currentColorBlue = 0;

            if (currentColorRed > 255) currentColorRed = 255;
            if (currentColorGreen > 255) currentColorGreen = 255;
            if (currentColorBlue > 255) currentColorBlue = 255;

            mySerial.print("Color was set to:    r= ");
            mySerial.print(currentColorRed);
            mySerial.print("   g= ");
            mySerial.print(currentColorGreen);
            mySerial.print("   b= ");
            mySerial.println (currentColorBlue);
        }

        if (command == SET_SPEED) {
            changeSpeed = mySerial.parseInt();

            if (changeSpeed < 1) changeSpeed = 1;
            if (changeSpeed > 255) changeSpeed = 255;
            
            mySerial.print("Speed was set to: ");
            mySerial.println(changeSpeed);
        }
        
    }
}

void tick() {
    bool redDone = false;
    bool greenDone = false;
    bool blueDone = false;

    if (targetValueRed < currentValueRed) {
        if ((currentValueRed - changeSpeed) <= targetValueRed)
            currentValueRed = targetValueRed;
        else 
            currentValueRed -= changeSpeed;
    } else if (targetValueRed > currentValueRed) {
        if ((currentValueRed + changeSpeed) >= targetValueRed)
            currentValueRed = targetValueRed;
        else 
            currentValueRed += changeSpeed;
    } else {
        redDone = true;
    }
    
    if (targetValueGreen < currentValueGreen) {
        if ((currentValueGreen - changeSpeed) <= targetValueGreen)
            currentValueGreen = targetValueGreen;
        else 
            currentValueGreen -= changeSpeed;
    } else if (targetValueGreen > currentValueGreen) {
        if ((currentValueGreen + changeSpeed) >= targetValueGreen)
            currentValueGreen = targetValueGreen;
        else 
            currentValueGreen += changeSpeed;
    } else {
        greenDone = true;
    }
    
    if (targetValueBlue < currentValueBlue) {
        if ((currentValueBlue - changeSpeed) <= targetValueBlue)
            currentValueBlue = targetValueBlue;
       else 
            currentValueBlue -= changeSpeed;
    } else if (targetValueBlue > currentValueBlue) {
        if ((currentValueBlue + changeSpeed) >= targetValueBlue)
            currentValueBlue = targetValueBlue;
        else 
            currentValueBlue += changeSpeed;
    } else {
        blueDone = true;
    }

    if (isTurnedOn && redDone && greenDone && blueDone) {
        if (currentMode == MODE_BLINK) {
            if (currentValueRed == 0) {
                applyColors();
            } else {
                targetValueRed = 0;
                targetValueGreen = 0;
                targetValueBlue = 0;
            }
        } else if (currentMode == MODE_SIMPLE) {
            applyColors();
        }
    }
}

void checkForDistance() {
    digitalWrite(TRIG_PIN, HIGH);
    delayMicroseconds(10);
    digitalWrite(TRIG_PIN, LOW);
    delayMicroseconds(2);

    int time = pulseIn(ECHO_PIN, HIGH) / 58;

    mySerial.println(time);

    if (time <= 25) {
        needForSwitch--;

        if (needForSwitch <= 0) {
            needForSwitch = NEED_FOR_SWITCH;
            if (isTurnedOn) turnOff();
            else turnOn();
        }
    }
}

void setup() {
    mySerial.begin(9600);

    pinMode(TRIG_PIN, OUTPUT);
    pinMode(ECHO_PIN, INPUT);

    pinMode(RED_PIN, OUTPUT);
    pinMode(GREEN_PIN, INPUT);
    pinMode(BLUE_PIN, OUTPUT);
}
 
void loop(){
    checkForCommands();

    tick();

    if (counter >= 50) {
        checkForDistance();
        counter = 1;
    }

    counter++;
    
    analogWrite(RED_PIN, currentValueRed);
    analogWrite(GREEN_PIN, currentValueGreen);
    analogWrite(BLUE_PIN, currentValueBlue);

    delay(10);
}