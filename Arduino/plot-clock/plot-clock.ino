/*
 * plot-clock (c) by drspangle
 * plot-clock is licensed under a Creative Commons Attribution-ShareAlike 3.0 Unported License.
 * You should have received a copy of the license along with this work. If not, see <http://creativecommons.org/licenses/by-sa/3.0/>.
 * 
 * plot-clock has been modified from it's original version.
 * 
 * Attributions:
 * Plotclock by joo
 * Published on February 12, 2014
 * www.thingiverse.com/thing:248009
 * 
 * Glow-In-Dark Plot Clock by TuckerPi
 * Published on March 21, 2018
 * www.thingiverse.com/thing:2833916
 * 
 */
// 

// units: mm; microseconds; radians
// origin: bottom left of drawing surface

#include <TimeLib.h>
#include <DS1307RTC.h>
#include <Time.h>
#include <Servo.h>
#include <Wire.h>


//#define CALIBRATION      // Uncoenable calibration mode
#define REALTIMECLOCK    // enable real time clock

// When in calibration mode, adjust the following factors until the servos move exactly 90 degrees
#define SERVOFAKTORLEFT 600
#define SERVOFAKTORRIGHT 650

// Zero-position of left and right servo
// When in calibration mode, adjust the NULL-values so that the servo arms are at all times parallel
// either to the X or Y axis
#define SERVOLEFTNULL 2000
#define SERVORIGHTNULL 600

#define SERVOPINLEFT  9
#define SERVOPINRIGHT 10

// length of arms
#define L1 35
#define L2 55.1
#define L3 14
#define L4 45

//comment the next line out for military 24 hour time setting.
//#define NONMILITARY


// origin points of left and right servo
#define O1X 24      //point 1
#define O1Y -23     //point 1
#define O2X 49    //point 2
#define O2Y -25     //point 2

#define LASER 12

#define COLON 11

Servo servo2;  //Left servo
Servo servo3;  //Right servo

//Used to store the last known position of the plotter
volatile double lastX = 35;
volatile double lastY = 30;

//Random number for which side to rest the plotter on
long randNumber;
int last_min = 0;

// interval at which to write the time independently of button activation (default 10 minutes)
const long interval = 600000;    
unsigned long previousMillis = 0;       

//For printing to the serial console...
void print2digits(int number) {
  if (number >= 0 && number < 10) {
    Serial.write('0');
  }
  Serial.print(number);
}

void setup()
{
  Serial.begin(9600);
  randomSeed(analogRead(0));
  digitalWrite(LASER, LOW);
  pinMode(12, OUTPUT);
  pinMode(7, INPUT_PULLUP);
}

void loop()
{
  tmElements_t tm;
#ifdef CALIBRATION
  if (!servo2.attached()) servo2.attach(SERVOPINLEFT);
  if (!servo3.attached()) servo3.attach(SERVOPINRIGHT);

  while (true) {
    drawTo(0, 0);
    delay(5000);
    drawTo(75, 40);
    delay(5000);

    drawTo(5, 15);
    delay(2000);
    drawTo(5, 50);
    delay(500);
    drawTo(70, 50);
    delay(500);
    drawTo(70, 15);
    delay(500);
  }

#endif

  unsigned long currentMillis = millis();
  int xoffset = 3;
  int i = 0;
  
  Serial.println("Ready.");

  while (digitalRead(7) != LOW) {
    currentMillis = millis();
    if ((currentMillis - previousMillis) >= interval) {
      Serial.println("Time interval elapsed.");
      break;
    }
  }
  
  previousMillis = currentMillis;
  
  Serial.println("Activating.");
  if (!servo2.attached()) servo2.attach(SERVOPINLEFT);
  if (!servo3.attached()) servo3.attach(SERVOPINRIGHT);
  if (RTC.read(tm))
  {
    Serial.print("RTC OK, Time = ");
    print2digits(tm.Hour);
    Serial.write(':');
    print2digits(tm.Minute);
    Serial.write(':');
    print2digits(tm.Second);
    Serial.print(", Date (D/M/Y) = ");
    Serial.print(tm.Day);
    Serial.write('/');
    Serial.print(tm.Month);
    Serial.write('/');
    Serial.print(tmYearToCalendar(tm.Year));
    Serial.println();
    
    setTime(tm.Hour, tm.Minute, tm.Second, tm.Day, tm.Month, tm.Year);
    
    #ifdef NONMILITARY
    Serial.println("Output is 12h clock.");
    if (tm.Hour < 1) {
      setTime(12, tm.Minute, tm.Second, tm.Day, tm.Month, tm.Year);
    }
    if (tm.Hour > 12) {
      setTime(tm.Hour - 12, tm.Minute, tm.Second, tm.Day, tm.Month, tm.Year);
    }
    #endif
    
  } else {
    if (RTC.chipPresent()) {
      Serial.println("The DS1307 is stopped.  Please run the SetTime");
      Serial.println("example to initialize the time and begin running.");
      Serial.println();
    } else {
      Serial.println("DS1307 read error!  Please check the circuitry.");
      Serial.println();
    }
  }
  hour();
  while ((i + 1) * 10 <= hour())
  {
    i++;
  }

  
  if (i != 0) {
    Serial.println("Drawing first digit...");
    number(3 + xoffset, 23, i, 1.15);
    delay(100);
    Serial.println("Drawing second digit...");
    number(18 + xoffset, 23, (hour() - i * 10), 1.15);
    delay(100);
    Serial.println("Drawing colon...");
    number(33 + xoffset, 22, 11, 0.9);
    delay(100);
  
    i = 0;
    while ((i + 1) * 10 <= minute())
    {
      i++;
    }
    Serial.println("Drawing third digit...");
    number(42 + xoffset, 21, i, 1.1);
    delay(100);
    Serial.println("Drawing fourth digit...");
    number(62 + xoffset, 19, (minute() - i * 10), 1.0);
    delay(100);
  }else {
    Serial.println("Drawing first digit...");
    number(5 + xoffset, 23, (hour() - i * 10), 1.15);
    delay(100);
    Serial.println("Drawing colon...");
    number(20 + xoffset, 24, 11, 0.9);
    delay(100);
  
    i = 0;
    while ((i + 1) * 10 <= minute())
    {
      i++;
    }
    Serial.println("Drawing second digit...");
    number(35 + xoffset, 21, i, 1.1);
    delay(100);
    Serial.println("Drawing third digit...");
    number(55 + xoffset, 19, (minute() - i * 10), 1.0);
    delay(100);
  }
  
  Serial.println("Drawing complete. Resetting arm position.");
  
  randNumber = random(100);
  if (randNumber >= 50) {
    Serial.println("Resetting arm position on the RIGHT side.");
    drawTo(100, 0);
    delay(100);
  } else {
    Serial.println("Resetting arm position on the LEFT side.");
    drawTo(-5, 20);
    delay(100);
  }
  
  last_min = minute();
  Serial.println("Detaching servos.");
  delay(1000);
  servo2.detach();
  servo3.detach();
  delay(1000);
}


// Writing numeral with bx by being the bottom left originpoint. Scale 1 equals a 20 mm high font.
// The structure follows this principle: move to first startpoint of the numeral, lift down, draw numeral, lift up
void number(float bx, float by, int num, float scale) {

  switch (num) {
    case 0:
      scale = scale * 0.8;
      by = by + 3;
      drawTo(bx + 12 * scale, by + 6 * scale);
      digitalWrite(LASER, HIGH);
      bogenGZS(bx + 7 * scale, by + 10 * scale, 10 * scale, -0.8, 6.7, 0.5);
      digitalWrite(LASER, LOW);
      break;
    case 1:
      drawTo(bx + 5 * scale, by + 18 * scale);
      digitalWrite(LASER, HIGH);
      drawTo(bx + 8 * scale, by + 20 * scale);
      drawTo(bx + 8 * scale, by + 0 * scale);
      digitalWrite(LASER, LOW);
      break;
    case 2:
      scale = scale * 0.9;
      by = by + 3;
      drawTo(bx + 2 * scale, by + 15 * scale);
      digitalWrite(LASER, HIGH);
      bogenUZS(bx + 8 * scale, by + 14 * scale, 5 * scale, 3, -0.8, 1);
      drawTo(bx + 3 * scale, by + 0 * scale);
      drawTo(bx + 14 * scale, by + 0 * scale);
      digitalWrite(LASER, LOW);
      break;
    case 3:
      scale = scale * 0.95;
      drawTo(bx + 3 * scale, by + 17 * scale);
      digitalWrite(LASER, HIGH);
      bogenUZS(bx + 6 * scale, by + 15 * scale, 5 * scale, 3, -2, 0.8);
      bogenUZS(bx + 6 * scale, by + 5 * scale, 5 * scale, 1.57, -3, 0.8);
      digitalWrite(LASER, LOW);
      break;
    case 4:
      drawTo(bx + 10 * scale, by + 20 * scale);
      digitalWrite(LASER, HIGH);
      drawTo(bx + 10 * scale, by + 0 * scale);
      digitalWrite(LASER, LOW);
      drawTo(bx + 2 * scale, by + 20 * scale);
      digitalWrite(LASER, HIGH);
      drawTo(bx + 2 * scale, by + 10 * scale);
      drawTo(bx + 14 * scale, by + 10 * scale);
      digitalWrite(LASER, LOW);
      break;
    case 5:
      drawTo(bx + 2 * scale, by + 5 * scale);
      digitalWrite(LASER, HIGH);
      bogenGZS(bx + 5 * scale, by + 8 * scale, 5 * scale, -2.5, 2, 1);
      drawTo(bx + 3 * scale, by + 20 * scale);
      drawTo(bx + 13 * scale, by + 20 * scale);
      digitalWrite(LASER, LOW);
      break;
    case 6:
      drawTo(bx + 4 * scale, by + 10 * scale);
      digitalWrite(LASER, HIGH);
      bogenUZS(bx + 7 * scale, by + 6 * scale, 5 * scale, 2, -4.4, 1);
      drawTo(bx + 11 * scale, by + 20 * scale);
      digitalWrite(LASER, LOW);
      break;
    case 7:
      drawTo(bx + 2 * scale, by + 19 * scale);
      digitalWrite(LASER, HIGH);
      drawTo(bx + 12 * scale, by + 19 * scale);
      drawTo(bx + 2 * scale, by + 1);
      digitalWrite(LASER, LOW);
      break;
    case 8:
      drawTo(bx + 5 * scale, by + 10 * scale);
      digitalWrite(LASER, HIGH);
      bogenUZS(bx + 6 * scale, by + 16 * scale, 5 * scale, 4.7, -1.6, 1);
      bogenGZS(bx + 6 * scale, by + 6 * scale, 5 * scale, -4.7, 2, 1);
      digitalWrite(LASER, LOW);
      break;
    case 9:
      drawTo(bx + 9 * scale, by + 11 * scale);
      digitalWrite(LASER, HIGH);
      bogenUZS(bx + 7 * scale, by + 15 * scale, 5 * scale, 4, -0.5, 1);
      drawTo(bx + 5 * scale, by + 0);
      digitalWrite(LASER, LOW);
      break;
    case COLON:
      drawTo(bx + 5 * scale, by + 16 * scale);
      delay(100);
      digitalWrite(LASER, HIGH);
      delay(100);
      digitalWrite(LASER, LOW);
      drawTo(bx + 5 * scale, by + 4 * scale);
      delay(100);
      digitalWrite(LASER, HIGH);
      delay(100);
      digitalWrite(LASER, LOW);
      break;
  }
}



void bogenUZS(float bx, float by, float radius, int start, int ende, float sqee) {
  float inkr = -0.05;
  float count = 0;

  do {
    drawTo(sqee * radius * cos(start + count) + bx,
           radius * sin(start + count) + by);
    count += inkr;
  }
  while ((start + count) > ende);

}

void bogenGZS(float bx, float by, float radius, int start, int ende, float sqee) {
  float inkr = 0.05;
  float count = 0;

  do {
    drawTo(sqee * radius * cos(start + count) + bx,
           radius * sin(start + count) + by);
    count += inkr;
  }
  while ((start + count) <= ende);
}


void drawTo(double pX, double pY) {
  double dx, dy, c;
  int i;

  // dx dy of new point
  dx = pX - lastX;
  dy = pY - lastY;
  //path lenght in mm, times 4 equals 4 steps per mm
  c = floor(7 * sqrt(dx * dx + dy * dy));

  if (c < 1) c = 1;

  for (i = 0; i <= c; i++) {
    // draw line point by point
    set_XY(lastX + (i * dx / c), lastY + (i * dy / c));

  }

  lastX = pX;
  lastY = pY;
}

double return_angle(double a, double b, double c) {
  // cosine rule for angle between c and a
  return acos((a * a + c * c - b * b) / (2 * a * c));
}

void set_XY(double Tx, double Ty)
{
  if (digitalRead(LASER)) {
    delay(3);
  }
  else {
    delay(0.1);
  }
  double dx, dy, c, a1, a2, Hx, Hy;

  // calculate triangle between pen, servoLeft and arm joint
  // cartesian dx/dy
  dx = Tx - O1X;
  dy = Ty - O1Y;

  // polar lemgth (c) and angle (a1)
  c = sqrt(dx * dx + dy * dy); //
  a1 = atan2(dy, dx); //
  a2 = return_angle(L1, L2, c);

  servo2.writeMicroseconds(floor(((a2 + a1 - M_PI) * SERVOFAKTORLEFT) + SERVOLEFTNULL));

  // calculate joinr arm point for triangle of the right servo arm
  a2 = return_angle(L2, L1, c);
  Hx = Tx + L3 * cos((a1 - a2 + 0.621) + M_PI); //36,5°
  Hy = Ty + L3 * sin((a1 - a2 + 0.621) + M_PI);

  // calculate triangle between pen joint, servoRight and arm joint
  dx = Hx - O2X;
  dy = Hy - O2Y;

  c = sqrt(dx * dx + dy * dy);
  a1 = atan2(dy, dx);
  a2 = return_angle(L1, L4, c);

  servo3.writeMicroseconds(floor(((a1 - a2) * SERVOFAKTORRIGHT) + SERVORIGHTNULL));
}






