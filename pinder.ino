const int BUT1 = D0;
const int BUT2 = D1;
const int BUT3 = D2;

const int LED1 = D3;
const int LED2 = D4;
const int LED3 = D5;
const int LED4 = D6;

const int ENC_A = 10;
const int ENC_B = 9;
const int ENC_SW = 11;

enum GameState { IDLE, SHOW_SEQUENCE, WAIT_INPUT, GAME_OVER };
GameState state = IDLE;

const int maxSequence = 5;
int sequence[maxSequence];
int inputLog[maxSequence];
int currentLevel = 1;
int inputIndex = 0;

int selectedLED = 0;
volatile int lastEncoded = 0;

unsigned long lastActionTime = 0;
int showIndex = 0;
bool ledOn = false;

int speedMode = 1;

int getSpeedDelay() {
  switch (speedMode) {
    case 0: return 600;
    case 2: return 150;
    default: return 300;
  }
}

void turnOffLEDs() {
  digitalWrite(LED1, LOW);
  digitalWrite(LED2, LOW);
  digitalWrite(LED3, LOW);
  digitalWrite(LED4, LOW);
}

void showLED(int idx, bool on) {
  turnOffLEDs();
  if(!on) return;
  switch(idx) {
    case 0: digitalWrite(LED1, HIGH); break;
    case 1: digitalWrite(LED2, HIGH); break;
    case 2: digitalWrite(LED3, HIGH); break;
    case 3: digitalWrite(LED4, HIGH); break;
  }
}

void updateEncoder() {
  int MSB = digitalRead(ENC_A);
  int LSB = digitalRead(ENC_B);
  int encoded = (MSB << 1) | LSB;
  int sum = (lastEncoded << 2) | encoded;

  if (sum == 0b1101 || sum == 0b0100 || sum == 0b0001 || sum == 0b1011) {
    selectedLED = (selectedLED + 1) % 4;
  }
  if (sum == 0b1110 || sum == 0b0111 || sum == 0b0001 || sum == 0b1000) {
    selectedLED = (selectedLED + 3) % 4;
  }
  lastEncoded = encoded;
}

void setup() {
  Serial.begin(115200);
  randomSeed(analogRead(0));

  pinMode(ENC_A, INPUT_PULLUP);
  pinMode(ENC_B, INPUT_PULLUP);
  pinMode(ENC_SW, INPUT_PULLUP);

  pinMode(BUT1, INPUT_PULLUP);
  pinMode(BUT2, INPUT_PULLUP);
  pinMode(BUT3, INPUT_PULLUP);

  pinMode(LED1, OUTPUT);
  pinMode(LED2, OUTPUT);
  pinMode(LED3, OUTPUT);
  pinMode(LED4, OUTPUT);

  attachInterrupt(digitalPinToInterrupt(ENC_A), updateEncoder, CHANGE);
  attachInterrupt(digitalPinToInterrupt(ENC_B), updateEncoder, CHANGE);

  turnOffLEDs();
}

void loop() {
  handleButtons();
  switch (state) {
    case IDLE:
    if (digitalRead(ENC_SW) == LOW) {
      delay(200);
      startGame();
    } else {
      previewLED(selectedLED);
    }
    break;

    case SHOW_SEQUENCE:
      handleShowSequence();
      break;

    case WAIT_INPUT:
      handleEncoderInput();
      break;

    case GAME_OVER:
      handleGameOver();
      break;
  }
}

void startGame() {
  currentLevel = 1;
  inputIndex = 0;

  for (int i = 0; i < maxSequence; i++) {
    sequence[i] = random(0, 4);
  }

  state = SHOW_SEQUENCE;
  ledOn = false;
  showIndex = 0;
  lastActionTime = millis();
}

void turnOffLEDs() {
  digitalWrite(LED1, LOW);
  digitalWrite(LED2, LOW);
  digitalWrite(LED3, LOW);
  digitalWrite(LED4, LOW);
}

void showLED(int idx, bool on) {
  turnOffLEDs();
  if (!on) return;
  switch (idx) {
    case 0: digitalWrite(LED1, HIGH); break;
    case 1: digitalWrite(LED2, HIGH); break;
    case 2: digitalWrite(LED3, HIGH); break;
    case 3: digitalWrite(LED4, HIGH); break;
  }
}

void previewLED(int idx) {
  showLED(idx, true);
  delay(50);
  turnOffLEDs();
}

void handleShowSequence() {
  int delayTime = getSpeedDelay();
  if (millis() - lastActionTime >= delayTime) {
    if (ledOn) {
      turnOffLEDs();
      showIndex++;
    } else {
      if (showIndex < currentLevel) {
        showLED(sequence[showIndex], true);
      } else {
        state = WAIT_INPUT;
        inputIndex = 0;
        return;
      }
    }
    ledOn = !ledOn;
    lastActionTime = millis();
  }
}

void handleEncoderInput() {
  previewLED(selectedLED);

  if (digitalRead(ENC_SW) == LOW) {
    delay(200);
    showLED(selectedLED, true);
    delay(300);
    turnOffLEDs();

    inputLog[inputIndex] = selectedLED;

    if (selectedLED == sequence[inputIndex]) {
      inputIndex++;
      if (inputIndex >= currentLevel) {
        currentLevel++;
        if (curentLevel > maxSequence) currentLevel = maxSequence;
        state = SHOW_SEQUENCE;
        showIndex = 0;
        ledOn = false;
        delay(500);
      }
    } else {
      state = GAME_OVER;
      lastActionTime = millis();
    }

    while (digitalRead(ENC_SW) == LOW);
  }
}

void handleGameOver() {
  if ((millis() - lastActionTime) < 1500) {
    bool on = ((millis() / 300) % 2) == 0;
    digitalWrite(LED1, on);
    digitalWrite(LED2, on);
    digitalWrite(LED3, on);
    digitalWrite(LED4, on);
  } else {
    turnOffLEDs();
    state = IDLE;
  }
}

void handleButtons() {
  if (digitalRead(BUT1) == LOW) {
    delay(150);
    startGame();
    while (digitalRead(BUT1) == LOW);
  }

  if (digitalRead(BUT2) == LOW) {
    delay(150);
    speedMode = (speedMode + 1) % 3;
    while (digitalRead(BUT2) == LOW);
  }

  if (state == WAIT_INPUT && inputIndex > 0 && digitalRead(BUT3) == LOW) {
    delay(150);
    inputIndex--;
    selectedLED = inputLog[inputIndex];
    showLED(selectedLED, true);
    delay(200);
    turnOffLEDs();
    while (digitalRead(BUT3) == LOW);
  }
}
