#include <Arduino.h>
#include <Servo.h>

// servo control pin
#define MOTOR_PIN 9
// PWM control pin
#define PWM1_PIN 5
#define PWM2_PIN 6
// 74HCT595N chip pin
#define SHCP_PIN 2 // The displacement of the clock
#define EN_PIN 7   // Can make control
#define DATA_PIN 8 // Serial data
#define STCP_PIN 4 // Memory register clock
// 超声波控制引脚
#define Trig_PIN 12
#define Echo_PIN 13
// 循迹控制引脚
#define LEFT_LINE_TRACKING A0
#define CENTER_LINE_TRACKING A1
#define RIGHT_LINE_TRACKING A2

// Function declarations
void RXpack_func();
void model1_func(byte orders);
void model2_func();
void model3_func();
void model4_func();
void Motor(int Dir, int Speed);
void motorleft();
void motorright();
float SR04(int Trig, int Echo);

Servo MOTORservo;

const int Forward = 92;       // forward
const int Backward = 163;     // back
const int Turn_Left = 149;    // left translation
const int Turn_Right = 106;   // Right translation
const int Top_Left = 20;      // Upper left mobile
const int Bottom_Left = 129;  // Lower left mobile
const int Top_Right = 72;     // Upper right mobile
const int Bottom_Right = 34;  // The lower right move
const int Stop = 0;           // stop
const int Contrarotate = 172; // Counterclockwise rotation
const int Clockwise = 83;     // Rotate clockwise
const int Moedl1 = 25;        // model1
const int Moedl2 = 26;        // model2
const int Moedl3 = 27;        // model3
const int Moedl4 = 28;        // model4
const int MotorLeft = 230;    // servo turn left
const int MotorRight = 231;   // servo turn right

int Left_Tra_Value;
int Center_Tra_Value;
int Right_Tra_Value;
int Black_Line = 400;

int leftDistance = 0;
int middleDistance = 0;
int rightDistance = 0;

byte RX_package[3] = {0};
uint16_t angle = 90;
byte order = Stop;
char model_var = 0;
int UT_distance = 0;

void setup()
{
  Serial.setTimeout(10);
  Serial.begin(115200);

  MOTORservo.attach(MOTOR_PIN);

  pinMode(SHCP_PIN, OUTPUT);
  pinMode(EN_PIN, OUTPUT);
  pinMode(DATA_PIN, OUTPUT);
  pinMode(STCP_PIN, OUTPUT);
  pinMode(PWM1_PIN, OUTPUT);
  pinMode(PWM2_PIN, OUTPUT);

  pinMode(Trig_PIN, OUTPUT);
  pinMode(Echo_PIN, INPUT);

  pinMode(LEFT_LINE_TRACKING, INPUT);
  pinMode(CENTER_LINE_TRACKING, INPUT);
  pinMode(RIGHT_LINE_TRACKING, INPUT);

  MOTORservo.write(angle);

  Motor(Stop, 0);
}

void loop()
{
  RXpack_func();
  switch (model_var)
  {
  case 0:
    model1_func(order);
    break;
  case 1:
    model2_func(); // OA model
    break;
  case 2:
    model3_func(); // follow model
    break;
  case 3:
    model4_func(); // Tracking model
    break;
  }
}

void model1_func(byte orders)
{
  switch (orders)
  {
  case Stop:
    Motor(Stop, 0);
    break;
  case Forward:
    Motor(Forward, 180);
    break;
  case Backward:
    Motor(Backward, 180);
    break;
  case Turn_Left:
    Motor(Turn_Left, 180);
    break;
  case Turn_Right:
    Motor(Turn_Right, 180);
    break;
  case Top_Left:
    Motor(Top_Left, 180);
    break;
  case Top_Right:
    Motor(Top_Right, 180);
    break;
  case Bottom_Left:
    Motor(Bottom_Left, 180);
    break;
  case Bottom_Right:
    Motor(Bottom_Right, 180);
    break;
  case Clockwise:
    Motor(Clockwise, 180);
    break;
  case Contrarotate:
    Motor(Contrarotate, 180);
    break;
  case MotorLeft:
    motorleft();
    break;
  case MotorRight:
    motorright();
    break;
  default:
    // Serial.println(".");
    order = 0;
    Motor(Stop, 0);
    break;
  }
}

void model2_func() // OA
{
  MOTORservo.write(90);
  UT_distance = SR04(Trig_PIN, Echo_PIN);
  Serial.println(UT_distance);
  middleDistance = UT_distance;

  if (middleDistance <= 25)
  {
    Motor(Stop, 0);
    for (int i = 0; i < 500; i++)
    {
      delay(1);
      RXpack_func();
      if (model_var != 1)
        return;
    }
    MOTORservo.write(10);
    for (int i = 0; i < 300; i++)
    {
      delay(1);
      RXpack_func();
      if (model_var != 1)
        return;
    }
    rightDistance = SR04(Trig_PIN, Echo_PIN); // SR04();
    Serial.print("rightDistance:  ");
    Serial.println(rightDistance);
    MOTORservo.write(90);
    for (int i = 0; i < 300; i++)
    {
      delay(1);
      RXpack_func();
      if (model_var != 1)
        return;
    }
    MOTORservo.write(170);
    for (int i = 0; i < 300; i++)
    {
      delay(1);
      RXpack_func();
      if (model_var != 1)
        return;
    }
    leftDistance = SR04(Trig_PIN, Echo_PIN); // SR04();
    Serial.print("leftDistance:  ");
    Serial.println(leftDistance);
    MOTORservo.write(90);
    if ((rightDistance < 20) && (leftDistance < 20))
    {

      Motor(Backward, 180);
      for (int i = 0; i < 1000; i++)
      {
        delay(1);
        RXpack_func();
        if (model_var != 1)
          return;
      }
      Motor(Contrarotate, 250);
      for (int i = 0; i < 500; i++)
      {
        delay(1);
        RXpack_func();
        if (model_var != 1)
          return;
      }
    }
    else if (rightDistance < leftDistance)
    {
      Motor(Stop, 0);
      for (int i = 0; i < 100; i++)
      {
        delay(1);
        RXpack_func();
        if (model_var != 1)
          return;
      }
      Motor(Backward, 180);
      for (int i = 0; i < 500; i++)
      {
        delay(1);
        RXpack_func();
        if (model_var != 1)
          return;
      }
      Motor(Contrarotate, 250);
      for (int i = 0; i < 500; i++)
      {
        delay(1);
        RXpack_func();
        if (model_var != 1)
          return;
      }
    } // turn right
    else if (rightDistance > leftDistance)
    {
      Motor(Stop, 0);
      for (int i = 0; i < 500; i++)
      {
        delay(1);
        RXpack_func();
        if (model_var != 1)
          return;
      }
      Motor(Backward, 180);
      for (int i = 0; i < 500; i++)
      {
        delay(1);
        RXpack_func();
        if (model_var != 1)
          return;
      }
      Motor(Clockwise, 250);
      for (int i = 0; i < 500; i++)
      {
        delay(1);
        RXpack_func();
        if (model_var != 1)
          return;
      }
    }
    else
    {
      Motor(Backward, 180);
      for (int i = 0; i < 500; i++)
      {
        delay(1);
        RXpack_func();
        if (model_var != 1)
          return;
      }
      Motor(Clockwise, 250);
      for (int i = 0; i < 500; i++)
      {
        delay(1);
        RXpack_func();
        if (model_var != 1)
          return;
      }
    }
  }
  else
  {
    Motor(Forward, 250);
  }
}

void model3_func() // follow model
{
  MOTORservo.write(90);
  UT_distance = SR04(Trig_PIN, Echo_PIN);
  Serial.println(UT_distance);
  if (UT_distance < 15)
  {
    Motor(Backward, 200);
  }
  else if (15 <= UT_distance && UT_distance <= 20)
  {
    Motor(Stop, 0);
  }
  else if (20 <= UT_distance && UT_distance <= 25)
  {
    Motor(Forward, 180);
  }
  else if (25 <= UT_distance && UT_distance <= 50)
  {
    Motor(Forward, 220);
  }
  else
  {
    Motor(Stop, 0);
  }
}

void model4_func() // tracking model
{
  MOTORservo.write(90);
  Left_Tra_Value = analogRead(LEFT_LINE_TRACKING);
  Center_Tra_Value = analogRead(CENTER_LINE_TRACKING);
  Right_Tra_Value = analogRead(RIGHT_LINE_TRACKING);
  if (Left_Tra_Value < Black_Line && Center_Tra_Value >= Black_Line && Right_Tra_Value < Black_Line)
  {
    Motor(Forward, 250);
  }
  else if (Left_Tra_Value >= Black_Line && Center_Tra_Value >= Black_Line && Right_Tra_Value < Black_Line)
  {
    Motor(Contrarotate, 220);
  }
  else if (Left_Tra_Value >= Black_Line && Center_Tra_Value < Black_Line && Right_Tra_Value < Black_Line)
  {
    Motor(Contrarotate, 250);
  }
  else if (Left_Tra_Value < Black_Line && Center_Tra_Value < Black_Line && Right_Tra_Value >= Black_Line)
  {
    Motor(Clockwise, 250);
  }
  else if (Left_Tra_Value < Black_Line && Center_Tra_Value >= Black_Line && Right_Tra_Value >= Black_Line)
  {
    Motor(Clockwise, 220);
  }
  else if (Left_Tra_Value >= Black_Line && Center_Tra_Value >= Black_Line && Right_Tra_Value >= Black_Line)
  {
    Motor(Stop, 0);
  }
}
void motorleft() // servo
{
  MOTORservo.write(angle);
  angle += 1;
  if (angle >= 180)
    angle = 180;
  delay(10);
}
void motorright() // servo
{
  MOTORservo.write(angle);
  angle -= 1;
  if (angle <= 1)
    angle = 1;
  delay(10);
}

void Motor(int Dir, int Speed) // motor drive
{
  digitalWrite(EN_PIN, LOW);
  analogWrite(PWM1_PIN, Speed);
  analogWrite(PWM2_PIN, Speed);

  digitalWrite(STCP_PIN, LOW);
  shiftOut(DATA_PIN, SHCP_PIN, MSBFIRST, Dir);
  digitalWrite(STCP_PIN, HIGH);
}

float SR04(int Trig, int Echo) // ultrasonic measured distance
{
  digitalWrite(Trig, LOW);
  delayMicroseconds(2);
  digitalWrite(Trig, HIGH);
  delayMicroseconds(10);
  digitalWrite(Trig, LOW);
  float distance = pulseIn(Echo, HIGH) / 58.00;
  delay(10);

  return distance;
}

void RXpack_func() // Receive data
{
  if (Serial.available() > 0)
  {
    delay(1); // delay 1MS
    if (Serial.readBytes(RX_package, 3))
    {
      if (RX_package[0] == 0xA5 && RX_package[2] == 0x5A) // The header and tail of the packet are verified
      {
        order = RX_package[1];
        Serial.println(order);
        if (order == Moedl1)
        {
          model_var = 0;
        }
        else if (order == Moedl2)
        {
          model_var = 1;
        }
        else if (order == Moedl3)
        {
          model_var = 2;
        }
        else if (order == Moedl4)
        {
          model_var = 3;
        }
        //////////////////////////////
        // switch (RX_package[1])
        // {
        // case Stop:
        //     Serial.println("Stop");
        //     break;
        // case Forward:
        //     Serial.println("Forward");
        //     break;
        // case Backward:
        //     Serial.println("Backward");
        //     break;
        // case Turn_Left:
        //     Serial.println("Turn_Left");
        //     break;
        // case Turn_Right:
        //     Serial.println("Turn_Right");
        //     break;
        // case Top_Left:
        //     Serial.println("Top_Left");
        //     break;
        // case Bottom_Left:
        //     Serial.println("Bottom_Left");
        //     break;
        // case Top_Right:
        //     Serial.println("Top_Right");
        //     break;
        // case Bottom_Right:
        //     Serial.println("Bottom_Right");
        //     break;
        // case Clockwise:
        //     Serial.println("Clockwise");
        //     break;
        // case MotorLeft:
        //     Serial.println("MotorLeft");
        //     break;
        // case MotorRight:
        //     Serial.println("MotorRight");
        //     break;
        // case Moedl1:
        //     Serial.println("Moedl1");
        //     break;
        // case Moedl2:
        //     Serial.println("Moedl2");
        //     break;
        // case Moedl3:
        //     Serial.println("Moedl3");
        //     break;
        // case Moedl4:
        //     Serial.println("Moedl4");
        //     break;
        // default:
        //     break;
        // }
      }
    }
  }
}
