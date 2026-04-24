// =========================================================
// NodeMCU Car Control

// This code is designed to control a NodeMCU-based car
// with steering and throttle control via a web interface.

// It uses an ESP32 with a web server to handle
// commands from a web page.

// The car can be steered left or right, and it can move
// forward or backward with throttle control.

// The code also includes a simple HTML interface
// for user interaction.
// =========================================================

// =========================================================
// Libraries
// =========================================================
#include <WiFi.h>
#include <WiFiClient.h>
#include <WebServer.h>
#include <EEPROM.h>

#include "conf.h"
#include "app.h"

// =========================================================
// Constants and Definitions
// =========================================================
int OMR_INV_DIR = 0;
int OMR_INV_SPEED = 0;
#define setSteerDir(dir) digitalWrite(PIN_STEER_DIR, (dir))
//#define setCarDir(dir) digitalWrite(PIN_CAR_DIR, (dir))

// =========================================================
// Global Variables
// =========================================================

int car_speed = DEFAULT_SPEED;            // Car speed in DAC value
int stepper_pulse_width = DEFAULT_STEER;  // Pulse width in micro sec
signed char current_position = 0;         // Track current step position

#if GO_HOME_MODE == GO_HOME_MODE_AUTO
bool auto_home_flag = 1;
#else
bool auto_home_flag = 0;  // Auto Go Home disabled by default
#endif

bool manual_home_flag = 0;

String dir = "S";  //String to store app command state.
WebServer server(80);

struct {
  bool forward;
  bool backward;
  bool left;
  bool right;
  bool stop;
} dir_flags;
bool stopped_flag = true;

// =========================================================
// Forward declarations for functions used before definition
// =========================================================
void goHome();
void goAhead();
void goBack();
void goLeft();
void goRight();
void stopRobot();
void stepMotor();
void setSpeed(int speed);
void setSteer(int speed);
void moveDirection(String dir);
void addToCurrentPosition(signed char val);

// =========================================================
// Setup Function
// =========================================================
void setup() {
  pinMode(PIN_THOTTLE, OUTPUT);
  pinMode(PIN_BRAKES, OUTPUT);
  pinMode(PIN_CAR_DIR, OUTPUT);
  pinMode(PIN_STEER_DIR, OUTPUT);
  pinMode(PIN_STEP, OUTPUT);

dacWrite(PIN_THOTTLE, DAC_MIN_VALUE);
  digitalWrite(PIN_BRAKES, BRAKES_ON);
  Serial.begin(9600);
  Serial2.begin(9600);
  // Initialize EEPROM
  //EEPROM.begin(EEPROM_SIZE);

  // Check if position was saved from previous run
 // if (EEPROM.read(EEPROM_INDICATOR_ADDR) == EEPROM_INDICATOR_VALUE) {
    // Restore position from EEPROM (8-bit signed value)
    //signed char saved_position = EEPROM.read(EEPROM_POSITION_ADDR);

    // Validate the restored position is within bounds
    /*if (saved_position >= -MAX_STEERING_STEPS && saved_position <= MAX_STEERING_STEPS) {
      current_position = saved_position;
      Serial.print("Restored position from EEPROM: ");
      Serial.println((int)current_position);
    } else {
      Serial.println("Invalid position in EEPROM, resetting to 0");
      current_position = 0;
      // Save the corrected position
      EEPROM.write(EEPROM_POSITION_ADDR, 0);
      EEPROM.commit();
    }
  } else {
    // First run or no saved position, set indicator and save current position
    EEPROM.write(EEPROM_INDICATOR_ADDR, EEPROM_INDICATOR_VALUE);
    EEPROM.write(EEPROM_POSITION_ADDR, (current_position & 0xFF));
    EEPROM.commit();
    Serial.println("First run - saved initial position to EEPROM");
  }
*/// ==========================================
  // EEPROM DISABLED
  // ==========================================


  // Setting default position to 0 on every boot to avoid errors
  current_position = 0; 
  Serial.println("EEPROM Disabled - Steering position defaults to 0");


  // Connecting WiFi
  WiFi.mode(WIFI_AP);
  WiFi.softAP(WIFI_SSID, WIFI_PASSWORD);

  IPAddress myIP = WiFi.softAPIP();
  Serial.print("AP IP address: ");
  Serial.println(myIP);

  // Serve the main page
  server.on("/", HTTP_GET, []() {
    Serial.println("Web request received for /");
    server.send(200, "text/html", htmlPage);
  });

  // Handle direction commands
  server.on("/cmd", []() {
    if (server.hasArg("dir")) {

      dir = server.arg("dir");

      Serial.print("Received command: ");
      Serial.println(dir);  // <-- Print the command sent

      server.send(200, "text/plain", "OK");

      moveDirection();

    } else if (server.hasArg("autoHome")) {

      String autoHome = server.arg("autoHome");

      if (autoHome == "1") {

        auto_home_flag = 1;

        Serial.println("Auto Go Home enabled");

      } else {

        auto_home_flag = 0;

        Serial.println("Auto Go Home disabled");
      }

      server.send(200, "text/plain", "OK");

    } else if (server.hasArg("manualHome")) {

      Serial.println("Manual Go Home triggered");

      //Check if Already at Home
      if (current_position == 0) {
        Serial.println("Already at Home");
      } else {
        manual_home_flag = 1;
      }

      server.send(200, "text/plain", "OK");

    } else if (server.hasArg("resetPosition")) {
      
      Serial.println("Reset Position triggered");
      
      addToCurrentPosition(-current_position);  // Reset to zero
      
      server.send(200, "text/plain", "OK");

    } else {

      server.send(400, "text/plain", "Missing dir");
    }
  });

  // Handle speed commands
  server.on("/speed", []() {
    if (server.hasArg("car")) {

      int speed = server.arg("car").toInt();

      Serial.print("Received command: ");
      Serial.println(speed);  // <-- Print the command sent

      setSpeed(speed);

      server.send(200, "text/plain", "OK");

    } else if (server.hasArg("steer")) {

      int speed = server.arg("steer").toInt();

      Serial.print("Received command: ");
      Serial.println(speed);  // <-- Print the command sent

      setSteer(speed);

      server.send(200, "text/plain", "OK");

    } else {

      server.send(400, "text/plain", "Missing val");
    }
  });

  server.begin();
}
#include <string.h>
// =========================================================
// Loop Function
// =========================================================
void loop() {
  server.handleClient();
  if (dir_flags.forward)
    goAhead();

  if (dir_flags.backward)
    goBack();

  if (dir_flags.stop)
    stopRobot();

  if (dir_flags.left)
    goLeft();

  if (dir_flags.right)
    goRight();

  if (!dir_flags.forward && !dir_flags.backward && !stopped_flag) {
    stopRobot();
    stopped_flag = true;
  }

  if (manual_home_flag || (!dir_flags.right && !dir_flags.left && current_position != 0 && auto_home_flag)) {
    // If no steering direction is pressed, go home
    goHome();
  }
}

// =========================================================
// Core Motor Functions
// =========================================================
void stepMotor() {
  //move one step
  digitalWrite(PIN_STEP, HIGH);
  delayMicroseconds(stepper_pulse_width);
  digitalWrite(PIN_STEP, LOW);
  delayMicroseconds(stepper_pulse_width);
}

void goAhead() {

// 1. Release brakes
  digitalWrite(PIN_BRAKES, BRAKES_OFF);
  // 2. Set direction forward (0)
  digitalWrite(PIN_CAR_DIR, DIR_FORWARD);

  // 3.apply throttle
  dacWrite(PIN_THOTTLE, car_speed);
  
  Serial.print("Car moving forward with speed: ");
  Serial.println(car_speed * 3.3 / 255.0); // Print speed in volts

  OMR_INV_DIR = 1;
  OMR_INV_SPEED = 50;
  int speed_cmd = OMR_INV_SPEED * OMR_INV_DIR;
  char cmd_str[64];
  sprintf(cmd_str, "$%i$", speed_cmd);
  Serial2.println(cmd_str);
}

void goBack() {
  // 1. Release brakes
  digitalWrite(PIN_BRAKES, BRAKES_OFF);
  
  // 2. Set direction backward (1)
  digitalWrite(PIN_CAR_DIR, DIR_BACKWARD);

  // 3. Apply throttle (Scaled down to 80% for safer reverse speed)
  int backward_speed = car_speed * 0.8;
  if (backward_speed < DAC_MIN_VALUE) {
  backward_speed = DAC_MIN_VALUE;
  } // Prevent dropping below 0.8V
  dacWrite(PIN_THOTTLE, backward_speed);
 

  Serial.print("Car moving backward with speed: ");
  Serial.println(car_speed * 3.3 / 255.0); // Print speed in volts

  OMR_INV_DIR = -1;
  OMR_INV_SPEED = 20;
  int speed_cmd = OMR_INV_SPEED * OMR_INV_DIR;
  char cmd_str[64];
  sprintf(cmd_str, "$%i$", speed_cmd);
  Serial2.println(cmd_str);
}

void goLeft() {
  if (current_position < MAX_STEERING_STEPS) {
    stepMotor();
    addToCurrentPosition(1);
  }
}

void goRight() {
  if (abs(current_position - 1) <= MAX_STEERING_STEPS) {
    stepMotor();
    addToCurrentPosition(-1);
  }
}

void goHome() {
  // Set direction
  setSteerDir(current_position > 0 ? st_right : st_left);

  stepMotor();
  
  addToCurrentPosition(current_position > 0 ? -1 : 1);

  if (current_position == 0){
    manual_home_flag = 0;
    Serial.println("Steering Reached Home");
  }
}

void stopRobot() {
  // Release throttle
  dacWrite(PIN_THOTTLE, DAC_MIN_VALUE);
  OMR_INV_DIR = 1;
  OMR_INV_SPEED = 0;
  //Activate brakes
  digitalWrite(PIN_BRAKES, brakes_on);

  Serial.println("Car stopped");
  int speed_cmd = OMR_INV_SPEED * OMR_INV_DIR;
  char cmd_str[64];
  sprintf(cmd_str, "$%i$", speed_cmd);
  Serial2.println(cmd_str);
}

void moveDirection() {
// Handle the 'G' command for stopping everything immediately
  if (dir == "G") {
    dir_flags.forward = false;
    dir_flags.backward = false;
    dir_flags.left = false;
    dir_flags.right = false;
    dir_flags.stop = true;
    
    stopRobot(); // Trigger the 0.8V and Brakes
    Serial.println("Command G Received: Throttle set to 0.8V (Inverter Stop)");
    return;
  }

  switch (dir[0]) {
    case 'S':
      {
        switch (dir[1]) {
          case 'L':
            dir_flags.left = false;
            // Save to EEPROM only when left steering stops
            //EEPROM.write(EEPROM_POSITION_ADDR, (current_position & 0xFF));
            //EEPROM.commit();
            break;
          case 'R':
            dir_flags.right = false;
            // Save to EEPROM only when right steering stops
            //EEPROM.write(EEPROM_POSITION_ADDR, (current_position & 0xFF));
            //EEPROM.commit();
            break;
          case 'F':
            dir_flags.forward = false;
            break;
          case 'B':
            dir_flags.backward = false;
            break;
          default:
            dir_flags.stop = false;
        }
        break;
      }
    case 'F':
      {
        if (!dir_flags.backward) {
          stopped_flag = false;
          //setCarDir(forward);
          dir_flags.forward = true;
        }
        break;
      }
    case 'B':
      {
        if (!dir_flags.forward) {
          stopped_flag = false;
          //setCarDir(backward);
          dir_flags.backward = true;
        }
        break;
      }
    case 'L':
      {
        if (!dir_flags.right) {
          manual_home_flag = 0;
          dir_flags.left = true;
          setSteerDir(st_left);
        }
        break;
      }
    case 'R':
      {
        if (!dir_flags.left) {
          manual_home_flag = 0;
          dir_flags.right = true;
          setSteerDir(st_right);
        }
        break;
      }
    default:
      {
        manual_home_flag = 0;
        dir_flags.stop = true;
        break;
      }
  }
}

void setSpeed(int speed) {
  // map speed to min-255
  car_speed = map(speed, 0, 100, DAC_MIN_VALUE, 255);
  Serial.print("Car speed Volt: ");
  Serial.println(speed * 3.3 / 100.0);
}

void setSteer(int speed) {
  // map speed to min-255
  stepper_pulse_width = map(speed, 1, 100, 10000, 1000);
  Serial.print("Steer speed delay us: ");
  Serial.println(stepper_pulse_width);
}

void addToCurrentPosition(signed char val) {
  current_position = current_position + val;


}
