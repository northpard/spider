#include <Wire.h>
#include "RTClib.h"
#include <Stepper.h>

RTC_DS3231 rtc;

// ===== 스텝모터 설정 =====
const int STEPS_PER_REV = 2048;

// ULN2003 보드 IN1~IN4를 Arduino 8~11번에 연결했다고 가정
const int IN1 = 8;
const int IN2 = 9;
const int IN3 = 10;
const int IN4 = 11;

// Stepper 라이브러리: (IN1, IN3, IN2, IN4) 순서 많이 사용됨
Stepper spiderStepper(STEPS_PER_REV, IN1, IN3, IN2, IN4);

// ===== 거미 위치 관리 =====
long currentSteps = 0;             // 0 = 가장 위
const long maxDownSteps = 2200;    // 약 10cm 정도 내려오는 한계 (릴/실에 따라 조정)

// ===== 패턴 실행 타이밍 =====
unsigned long nextActionTime = 0;           // 다음 “세션” 시작 예정 시간 (millis 기준)

// ======================== 기본 설정 ========================
void setup() {
  Serial.begin(9600);
  Wire.begin();
  rtc.begin();

  // 스텝모터 속도 (RPM). 너무 빠르면 토크 부족/미끄러짐.
  spiderStepper.setSpeed(10); // 10RPM 정도로 천천히

  // ★ RTC 시간 최초 1회 설정용 (사용 후에는 꼭 주석 처리!)
  // rtc.adjust(DateTime(2025, 11, 13, 9, 10, 0)); // 연,월,일,시,분,초

  // 랜덤 초기값
  randomSeed(analogRead(A0));

  // 처음엔 5초 뒤쯤 첫 행동 가능하도록
  nextActionTime = millis() + 5000;

  Serial.println("Spider random alive mode with time windows ready!");
}

// ======================== 유틸 함수들 ========================

// 거미 이동: steps > 0 아래로, steps < 0 위로
void moveSpiderSteps(long steps) {
  long target = currentSteps + steps;

  if (target < 0) {
    steps = -currentSteps;  // 0보다 위로 못 가게
    target = 0;
  } else if (target > maxDownSteps) {
    steps = maxDownSteps - currentSteps; // 아래 한계 넘지 않도록
    target = maxDownSteps;
  }

  if (steps == 0) return;

  spiderStepper.step(steps);
  currentSteps = target;
}

// 현재 시간이 활성 시간대인지 확인
bool isActiveTime(DateTime now) {
  int hour = now.hour();
  int minute = now.minute();

  bool morning = false;
  bool evening = false;

  // 아침: 09:20 ~ 09:40
  if (hour == 9 && minute >= 20 && minute <= 40) {
    morning = true;
  }

  // 저녁: 18:00 ~ 18:20
  if (hour == 18 && minute >= 0 && minute <= 20) {
    evening = true;
  }

  return (morning || evening);
}

// ======================== 패턴들 정의 ========================

// 1. 소심한 꿈틀 (작게 통통 뛰기)
void patternSmallWiggle() {
  Serial.println("Pattern: Small wiggle");
  int amp = 150; // 진동 크기 (스텝 수)

  for (int i = 0; i < 3; i++) {
    moveSpiderSteps(amp);   // 아래로
    delay(200);
    moveSpiderSteps(-amp);  // 위로
    delay(200);
  }
}

// 2. 갑자기 휙 내려갔다 살짝 올라오기
void patternDropAndSlightUp() {
  Serial.println("Pattern: Drop and slight up");
  int drop = random(400, 700);    // 크게 내려가기
  int up   = random(100, 250);    // 살짝 올라오기

  moveSpiderSteps(drop);   // 아래로
  delay(300);
  moveSpiderSteps(-up);    // 조금 위로
  delay(300);
}

// 3. 위로 슬쩍 올라갔다 제자리로
void patternUpAndBack() {
  Serial.println("Pattern: Up and back");
  int up = random(200, 400);

  moveSpiderSteps(-up);    // 위로
  delay(400);
  moveSpiderSteps(up);     // 다시 아래(원래 위치)
  delay(400);
}

// 4. 긴 숨 고르기 (천천히 크게 이동: 기어가는 느낌)
void patternSlowMove() {
  Serial.println("Pattern: Slow move");
  int dir = (random(0, 2) == 0) ? 1 : -1;   // 1: 아래, -1: 위
  int dist = random(300, 800);              // 전체 이동 거리

  int stepSize = (dir > 0) ? 10 : -10;      // 한 번에 조금씩
  int stepsMoved = 0;

  while (abs(stepsMoved) < dist) {
    moveSpiderSteps(stepSize);
    stepsMoved += stepSize;
    delay(80);  // 천천히 움직이는 느낌
  }
}

// 랜덤 패턴 하나 실행
void runRandomPattern() {
  int p = random(0, 4); // 0,1,2,3 중 하나

  switch (p) {
    case 0: patternSmallWiggle();      break;
    case 1: patternDropAndSlightUp();  break;
    case 2: patternUpAndBack();        break;
    case 3: patternSlowMove();         break;
  }
}

// ======================== 메인 루프 ========================
void loop() {
  DateTime now = rtc.now();

  bool active = isActiveTime(now);
  unsigned long nowMillis = millis();

  if (active) {
    // 활성 시간대: 아침(9:20~9:40) / 저녁(18:00~18:20)
    if ((long)(nowMillis - nextActionTime) >= 0) {
      // 이번 "세션"에서 실행할 패턴 개수 (1~3개)
      int numPatterns = random(1, 4);

      Serial.print("Session start, patterns: ");
      Serial.println(numPatterns);

      for (int i = 0; i < numPatterns; i++) {
        runRandomPattern();

        // 패턴과 패턴 사이에 정지 시간 (1~5초)
        unsigned long pause = random(1000, 5000);
        Serial.print("Pause between patterns (ms): ");
        Serial.println(pause);
        delay(pause);
      }

      // 세션 끝난 후, 다음 세션까지 쉬는 시간 (10~40초)
      unsigned long rest = random(10000, 40000);
      Serial.print("Rest until next session (ms): ");
      Serial.println(rest);

      nextActionTime = millis() + rest;
    }

  } else {
    // 비활성 시간대: 크게 움직이지 않음
    // 필요하다면, 천천히 위로 복귀시키는 로직 추가
    if (currentSteps > 0) {
      moveSpiderSteps(-10); // 조금씩 위로 올리기
      delay(100);
    }

    // 다음 활성 구간에서 바로 폭주하지 않도록, 여유 시간 설정
    if ((long)(nowMillis - nextActionTime) >= 0) {
      nextActionTime = nowMillis + 10000; // 10초 후부터 행동 가능
    }

    delay(200);
  }
}
