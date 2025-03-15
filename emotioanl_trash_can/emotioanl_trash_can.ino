#include <Servo.h>

// 초음파 센서 핀 설정
const int trigPin = 6;
const int echoPin = 7;

// 서보 모터 핀 설정
const int leftEyebrowPin = 2;
const int rightEyebrowPin = 3;
const int headShakePin = 8;
const int trashEjectPin = 9;

// 서보 모터 객체 생성
Servo leftEyebrow;
Servo rightEyebrow;
Servo headShake;
Servo trashEject;

// 거리 변수
long duration;
int distance;

// 오류 값 처리 및 평균 계산 변수
const int MAX_DISTANCE = 400;
const int MIN_DISTANCE = 2;
const int NUM_READINGS = 5;
int readings[NUM_READINGS];
int readIndex = 0;
int total = 0;
int averageDistance = 0;

// 쓰레기 감지 거리 (cm)
const int TRASH_DETECT_DISTANCE = 16;

// 화난 상태 시간 변수
unsigned long angryStartTime = 0;
bool isAngry = false;

// 마지막 감지 시간 변수
unsigned long lastDetectedTime = 0;

void setup() {
  // 핀 모드 설정
  pinMode(trigPin, OUTPUT);
  pinMode(echoPin, INPUT);

  // 서보 모터 연결
  leftEyebrow.attach(leftEyebrowPin);
  rightEyebrow.attach(rightEyebrowPin);
  headShake.attach(headShakePin);
  trashEject.attach(trashEjectPin);

  // 시리얼 통신 시작
  Serial.begin(9600);

  // 초기 측정값 배열 초기화
  for (int i = 0; i < NUM_READINGS; i++) {
    readings[i] = 0;
  }

  // 초기 서보 모터 위치 설정
  leftEyebrow.write(60);
  rightEyebrow.write(60);
  headShake.write(135);
  trashEject.write(105);
}

void loop() {
  // Trig 핀에 짧은 펄스 발생 (10 마이크로초)
  digitalWrite(trigPin, LOW);
  delayMicroseconds(2);
  digitalWrite(trigPin, HIGH);
  delayMicroseconds(10);
  digitalWrite(trigPin, LOW);

  // Echo 핀에서 반사된 신호를 받아오는 시간 측정
  duration = pulseIn(echoPin, HIGH, 30000);

  // 초음파의 이동 시간을 기반으로 거리 계산
  distance = duration * 0.0344 / 2;

  // 오류 값 처리
  if (distance > MAX_DISTANCE || distance < MIN_DISTANCE || duration == 0) {
    Serial.println("Error: Out of range or no signal.");
    delay(500);
    return;
  }

  // 측정값 배열에 저장 및 평균 계산
  total = total - readings[readIndex];
  readings[readIndex] = distance;
  total = total + readings[readIndex];
  readIndex = (readIndex + 1) % NUM_READINGS;

  // 평균 거리 계산
  averageDistance = total / NUM_READINGS;

  // 시리얼 모니터에 거리 출력
  Serial.print("Distance: ");
  Serial.print(averageDistance);
  Serial.println(" cm");

  // 쓰레기 감지
  if (averageDistance < TRASH_DETECT_DISTANCE) {
    // 마지막 감지 시간 업데이트
    lastDetectedTime = millis();

    if (!isAngry) {
      // 화난 상태 시작
      angryStartTime = millis();
      isAngry = true;

      // 화난 표정
      leftEyebrow.write(240);
      rightEyebrow.write(110);

      // 고개 흔들기
      shakeHead();

      // 쓰레기 배출
      trashEject.write(150);
    }
  }

  // 화난 상태 유지 및 종료
  if (isAngry) {
    if (millis() - angryStartTime >= 3000) {
      // 화난 상태 종료
      isAngry = false;
    }
  }

  // 2초 이상 감지되지 않으면 초기 상태로 복귀
  if (millis() - lastDetectedTime >= 2000 && isAngry) {
    isAngry = false;
    resetServos();
  }

  // 100ms 대기 후 다시 측정
  delay(100);
}

// 고개 흔들기 함수
void shakeHead() {
  for (int i = 0; i < 8; i++) {
    headShake.write(210);
    delay(250);
    headShake.write(50);
    delay(250);
  }
  headShake.write(130);
}

// 서보 모터 초기화 함수
void resetServos() {
  leftEyebrow.write(60);
  rightEyebrow.write(60);
  trashEject.write(105);
  headShake.write(135);
}
