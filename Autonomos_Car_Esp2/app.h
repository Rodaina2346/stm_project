#ifndef APP_H
#define APP_H

// =========================================================
// Include necessary libraries
// =========================================================

#include "conf.h"

// =========================================================
// Constants and Definitions
// =========================================================

// Throttle stop/min value
#define DAC_MIN_VALUE           (255 * 0.8 / 3.3)

#define DEFAULT_SPEED           (255 * 1.8 / 3.3)
#define DEFAULT_STEER           3000

#define MAX_STEERING_STEPS      100


// WiFi credentials and settings
#define WIFI_SSID               "Wifi Car"
#define WIFI_PASSWORD           "12345678"
#define WIFI_AP_IP              "192.168.4.1"
#define WIFI_AP_PORT            80

// control pins
#define PIN_THOTTLE             25
#define PIN_BRAKES              27
#define PIN_CAR_DIR             26

// steering pins
#define PIN_STEER_DIR           33    
#define PIN_STEP                32    

// EEPROM addresses
/*#define EEPROM_SIZE             512
#define EEPROM_INDICATOR_ADDR   0
#define EEPROM_POSITION_ADDR    1
#define EEPROM_INDICATOR_VALUE  0xA0
*/
// =========================================================
// Direction and Brakes Values 
// =========================================================
#define DIR_FORWARD             0
#define DIR_BACKWARD            1

#define BRAKES_ON               0  
#define BRAKES_OFF              1  
// =========================================================
// Enums for car direction and brakes
// =========================================================

// forward normally opened
enum {
  backward = 0,
  forward
};

// brakes off normally opened
enum {
  brakes_on = 0,
  brakes_off
};

// steering
enum {
  st_right = 0,
  st_left
};

// =========================================================
// HTML Page for the web interface
// =========================================================

const char* htmlPage = R"r(
<!DOCTYPE html>
<html>
<head>
  <title>NodeMCU_Car</title>
  <meta name="viewport" content="width=device-width, initial-scale=1.0">
  <style>
    * {
      margin: 0;
      padding: 0;
      box-sizing: border-box;
    }

    body { 
      font-family: 'Segoe UI', Tahoma, Geneva, Verdana, sans-serif; 
      background: linear-gradient(135deg, #667eea 0%, #764ba2 100%);
      min-height: 100vh;
      color: #333;
    }

    .container {
      max-width: 500px;
      margin: 0 auto;
      padding: 20px;
    }

    .header { 
      background: rgba(255, 255, 255, 0.95);
      backdrop-filter: blur(10px);
      color: #333;
      padding: 20px;
      font-size: 24px;
      font-weight: 600;
      text-align: center;
      border-radius: 20px;
      margin-bottom: 30px;
      box-shadow: 0 8px 32px rgba(0, 0, 0, 0.1);
      border: 1px solid rgba(255, 255, 255, 0.2);
    }

    .controls { 
      display: grid; 
      grid-template-columns: repeat(3, 1fr); 
      gap: 15px; 
      max-width: 300px;
      margin: 0 auto 30px;
    }

    .btn {
      background: rgba(255, 255, 255, 0.9);
      border: none;
      outline: none;
      cursor: pointer;
      border-radius: 20px;
      padding: 15px;
      transition: all 0.3s ease;
      box-shadow: 0 4px 15px rgba(0, 0, 0, 0.1);
      border: 1px solid rgba(255, 255, 255, 0.2);

      /* ENABLE MULTI-TOUCH */
      touch-action: none;
      pointer-events: auto;
    }

    .btn:hover {
      transform: translateY(-2px);
      box-shadow: 0 8px 25px rgba(0, 0, 0, 0.15);
      background: rgba(255, 255, 255, 1);
    }

    .btn svg { 
      width: 40px; 
      height: 40px;
      filter: drop-shadow(0 2px 4px rgba(0, 0, 0, 0.1));
    }

    .slider-container { 
      background: rgba(255, 255, 255, 0.95);
      backdrop-filter: blur(10px);
      border-radius: 25px;
      padding: 30px;
      box-shadow: 0 8px 32px rgba(0, 0, 0, 0.1);
      border: 1px solid rgba(255, 255, 255, 0.2);
    }

    .slider-group {
      margin-bottom: 25px;
    }

    .slider-label { 
      font-size: 16px; 
      font-weight: 600; 
      margin-bottom: 12px; 
      color: #333;
      display: block;
    }

    .slider-value { 
      font-size: 14px; 
      color: #666; 
      margin-left: 10px;
      font-weight: 500;
    }

    .slider { 
      width: 100%;
      height: 8px;
      border-radius: 5px;
      background: #e0e0e0;
      outline: none;
      -webkit-appearance: none;
      margin: 10px 0;
    }

    .slider::-webkit-slider-thumb {
      -webkit-appearance: none;
      width: 24px;
      height: 24px;
      border-radius: 50%;
      background: linear-gradient(135deg, #667eea 0%, #764ba2 100%);
      cursor: pointer;
    }

    .toggle-row { 
      display: flex; 
      flex-direction: column;
      gap: 15px;
      margin-top: 20px;
    }

    .toggle-btn, .home-btn {
      background: linear-gradient(135deg, #667eea 0%, #764ba2 100%);
      color: white;
      border: none;
      border-radius: 15px;
      padding: 15px 25px;
      font-size: 16px;
      font-weight: 600;
      cursor: pointer;
      transition: all 0.3s ease;
    }

    .toggle-btn.active {
      background: linear-gradient(135deg, #ff6b6b 0%, #ee5a24 100%);
    }

    .home-btn:disabled {
      background: #ccc;
      cursor: not-allowed;
    }
  </style>
</head>
<body>
  <div class="container">
    <div class="header">NodeMCU Car Control</div>
    <div class="controls">
      <!-- Empty corner position -->
      <div></div>
      <button class="btn" id="btnF" onpointerdown="handleButtonPress('F', 'btnF')" onpointerup="handleButtonRelease('SF', 'btnF')">
        <svg viewBox="0 0 60 60"><polygon points="30,10 50,30 10,30" fill="#667eea"/></svg>
      </button>
      <!-- Empty corner position -->
      <div></div>
      <button class="btn" id="btnL" onpointerdown="handleButtonPress('L', 'btnL')" onpointerup="handleButtonRelease('SL', 'btnL')">
        <svg viewBox="0 0 60 60"><polygon points="10,30 30,50 30,10" fill="#667eea"/></svg>
      </button>
      <button class="btn" id="btnS" onpointerdown="handleButtonPress('S', 'btnS')" onpointerup="handleButtonRelease('SS', 'btnS')">
        <svg viewBox="0 0 60 60"><circle cx="30" cy="30" r="25" stroke="#667eea" stroke-width="5" fill="none"/></svg>
      </button>
      <button class="btn" id="btnR" onpointerdown="handleButtonPress('R', 'btnR')" onpointerup="handleButtonRelease('SR', 'btnR')">
        <svg viewBox="0 0 60 60"><polygon points="50,30 30,50 30,10" fill="#667eea"/></svg>
      </button>
      <!-- Empty corner position -->
      <div></div>
      <button class="btn" id="btnB" onpointerdown="handleButtonPress('B', 'btnB')" onpointerup="handleButtonRelease('SB', 'btnB')">
        <svg viewBox="0 0 60 60"><polygon points="30,50 50,30 10,30" fill="#667eea"/></svg>
      </button>
      <!-- Empty corner position -->
      <div></div>
    </div>
)r"

#if ENABLE_CAR_SPEED || ENABLE_STEERING_SPEED || GO_HOME_MODE != GO_HOME_MODE_AUTO
R"r(
    <div class="slider-container">
)r"
#endif

#if ENABLE_CAR_SPEED
R"r(
      <div class="slider-group">
        <label class="slider-label">Car Speed</label>
        <input type="range" min="0" max="100" value="0" class="slider" id="speedSlider" oninput="updateSpeedValue(this.value); sendSpeed(this.value)">
        <span class="slider-value" id="speedValue">0%</span>
      </div>
)r"
#endif

#if ENABLE_STEERING_SPEED
R"r(
      <div class="slider-group">
        <label class="slider-label">Steering Speed</label>
        <input type="range" min="1" max="100" value="75" class="slider" id="steerSlider" oninput="updateSteerValue(this.value); sendSteerSpeed(this.value)">
        <span class="slider-value" id="steerValue">75%</span>
      </div>
)r"
#endif

#if GO_HOME_MODE != GO_HOME_MODE_AUTO
R"r(
      <div class="toggle-row">
)r"
#endif

#if GO_HOME_MODE == GO_HOME_MODE_USER
R"r(
        <button id="autoHomeBtn" class="toggle-btn" onclick="toggleAutoHome()">Enable Auto Go Home</button>
)r"
#endif

#if GO_HOME_MODE != GO_HOME_MODE_AUTO
R"r(
        <button id="manualHomeBtn" class="home-btn" onclick="sendGoHome()">Go Home</button>
)r"
#endif
#if ENABLE_RESET_POSITION && GO_HOME_MODE != GO_HOME_MODE_AUTO
R"r(
        <button id="resetPosition" class="home-btn" onclick="resetPosition()">Reset Steering to zero</button>
)r"
#endif

#if GO_HOME_MODE != GO_HOME_MODE_AUTO
R"r(
      </div>
)r"
#endif

#if ENABLE_CAR_SPEED || ENABLE_STEERING_SPEED || GO_HOME_MODE == GO_HOME_MODE_USER
R"r(
    </div>
)r"
#endif

R"r(
  </div>
  <script>
    const activeButtons = new Set();

    function sendCmd(cmd) {
      fetch('/cmd?dir=' + cmd);
    }

    function handleButtonPress(cmd, btnId) {
      if (activeButtons.has(btnId)) return;
      activeButtons.add(btnId);
      sendCmd(cmd);
    }

    function handleButtonRelease(cmd, btnId) {
      if (!activeButtons.has(btnId)) return;
      sendCmd(cmd);
      activeButtons.delete(btnId);
    }

    function sendSpeed(val) {
      fetch('/speed?car=' + val);
    }

    function sendSteerSpeed(val) {
      fetch('/speed?steer=' + val);
    }

    function updateSpeedValue(val) {
      document.getElementById('speedValue').textContent = val + '%';
    }

    function updateSteerValue(val) {
      document.getElementById('steerValue').textContent = val + '%';
    }

    function toggleAutoHome() {
      const btn = document.getElementById('autoHomeBtn');
      const manualBtn = document.getElementById('manualHomeBtn');
      const enabled = btn.classList.toggle('active');
      btn.textContent = enabled ? 'Disable Auto Go Home' : 'Enable Auto Go Home';
      manualBtn.disabled = enabled;
      if (enabled) {
        manualBtn.classList.remove('active');
      } else {
        manualBtn.classList.add('active');
      }
      fetch('/cmd?autoHome=' + (enabled ? '1' : '0'));
    }

    function sendGoHome() {
      fetch('/cmd?manualHome=1');
    }

    function resetPosition() {
      fetch('/cmd?resetPosition=1');
    }

    window.onload = function () {
      const btn = document.getElementById('autoHomeBtn');
      const manualBtn = document.getElementById('manualHomeBtn');
      if (!btn.classList.contains('active')) {
        manualBtn.disabled = false;
        manualBtn.classList.add('active');
      }
    }
  </script>
</body>
</html>
)r";

#endif
