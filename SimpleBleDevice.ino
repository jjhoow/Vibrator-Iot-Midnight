#include <BLEDevice.h>
#include <BLEUtils.h>
#include <BLEServer.h>
#include <vector>

// Motor A
int motor1Pin1 = 2; 
int motor1Pin2 = 3; 
int enable1Pin = 2; 

// Motor B
int motor2Pin1 = 4; 
int motor2Pin2 = 5; 
int enable2Pin = 4;

// Setting PWM properties
const int freq = 30000;
const int pwmChannel1 = 0;
const int pwmChannel2 = 1;
const int resolution = 8;

BLECharacteristic *characteristic;

// Estrutura para representar um padrão de controle de motor
struct MotorPattern {
  uint8_t motorId;
  uint8_t intensity;
  uint16_t duration;
};

// Vetor de padrões pré-definidos
std::vector<std::vector<MotorPattern>> patterns = {
  // Padrão 1
  {
    {1, 200, 300},
    {3, 200, 3000},
    {2, 100, 1000}
  },
  // Padrão 2
  {
    {2, 150, 500},
    {1, 100, 200},
    {3, 180, 1000}
  }
};

class MyServerCallbacks : public BLEServerCallbacks {
  void onConnect(BLEServer *server) {
    Serial.println("Conectado ao dispositivo BLE");
  }

  void onDisconnect(BLEServer *server) {
    Serial.println("Desconectado do dispositivo BLE");
  }
};

class MyCallbacks : public BLECharacteristicCallbacks {
  void onWrite(BLECharacteristic *characteristic) {
    std::string value = characteristic->getValue();
    if (value.length() > 0) {
      std::string code(value.begin(), value.end());
      Serial.print("Received code: ");
      Serial.println(code.c_str());

      if (code[0] == 'p') {
        // Código de padrão pré-definido
        int patternNumber = atoi(code.substr(1).c_str());

        if (patternNumber >= 0 && patternNumber < patterns.size()) {
          std::vector<MotorPattern> pattern = patterns[patternNumber];
          executeMotorPattern(pattern);
        }
      } else {
        // Código de controle individual de motor
        int motorId, intensity, duration;
        sscanf(code.c_str(), "%d,%d,%d", &motorId, &intensity, &duration);
        executeMotorControl(motorId, intensity, duration);
      }
    }
  }

  void executeMotorPattern(const std::vector<MotorPattern>& pattern) {
    for (const MotorPattern& motorPattern : pattern) {
      executeMotorControl(motorPattern.motorId, motorPattern.intensity, motorPattern.duration);

    }
  }

  void executeMotorControl(int motorId, int intensity, int duration) {
    // Set PWM value and control the motor
    if (motorId == 1) {
      Serial.println("Moving Forward - Motor A");
      digitalWrite(motor1Pin1, LOW);
      digitalWrite(motor1Pin2, HIGH); 
      ledcWrite(pwmChannel1, intensity);
      delay(duration);
      ledcWrite(pwmChannel1, 0);
      digitalWrite(motor1Pin1, LOW);
      digitalWrite(motor1Pin2, LOW); 
    }
    else if (motorId == 2) {
      Serial.println("Moving Forward - Motor B");
      digitalWrite(motor2Pin1, LOW);
      digitalWrite(motor2Pin2, HIGH);
      ledcWrite(pwmChannel2, intensity);
      delay(duration);
      ledcWrite(pwmChannel2, 0);
      digitalWrite(motor2Pin1, LOW);
      digitalWrite(motor2Pin2, LOW);
    }
    else if (motorId == 3) {
      Serial.println("Moving Forward - Both Motors");
      digitalWrite(motor1Pin1, LOW);
      digitalWrite(motor1Pin2, HIGH);
      digitalWrite(motor2Pin1, LOW);
      digitalWrite(motor2Pin2, HIGH);
      ledcWrite(pwmChannel1, intensity);
      ledcWrite(pwmChannel2, intensity);
      delay(duration);
      ledcWrite(pwmChannel1, 0);
      ledcWrite(pwmChannel2, 0);
      digitalWrite(motor1Pin1, LOW);
      digitalWrite(motor1Pin2, LOW);
      digitalWrite(motor2Pin1, LOW);
      digitalWrite(motor2Pin2, LOW);
    }
  }
};

void setup() {
  pinMode(motor1Pin1, OUTPUT);
  pinMode(motor1Pin2, OUTPUT);
  pinMode(enable1Pin, OUTPUT);

  pinMode(motor2Pin1, OUTPUT);
  pinMode(motor2Pin2, OUTPUT);
  pinMode(enable2Pin, OUTPUT);

  // configure LED PWM functionalities
  ledcSetup(pwmChannel1, freq, resolution);
  ledcSetup(pwmChannel2, freq, resolution);

  // attach the channels to the GPIOs to be controlled
  ledcAttachPin(enable1Pin, pwmChannel1);
  ledcAttachPin(enable2Pin, pwmChannel2);

  Serial.begin(115200);

  // BLE initialization
  BLEDevice::init("Arduino BLE Control");
  BLEServer *server = BLEDevice::createServer();
  server->setCallbacks(new MyServerCallbacks());

  BLEService *service = server->createService(BLEUUID("4fafc201-1fb5-459e-8fcc-c5c9c331914b"));

  characteristic = service->createCharacteristic(
    BLEUUID("beb5483e-36e1-4688-b7f5-ea07361b26a8"),
    BLECharacteristic::PROPERTY_WRITE
  );
  characteristic->setCallbacks(new MyCallbacks());

  service->start();

  BLEAdvertising *advertising = server->getAdvertising();
  advertising->start();
}

void loop() {
  // Empty loop, all control is handled in the BLE callback function
}
