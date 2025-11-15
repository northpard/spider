# Spider – Arduino Stepper Spider Reminder

출근/퇴근 시간을 잊지 않도록, 정해진 시간대에 줄에 매달린 거미 인형이
살아있는 것처럼 오르내리며 사람 눈에 띄게 만드는 아두이노 프로젝트입니다.
이 README 하나로 전체 설계, 배선, 패턴, 전체 스케치를 모두 볼 수 있습니다.

목차
-----
- [Quick Start](#quick-start)
- [Full System Concept](#full-system-concept)
- [Bill of Materials](#bill-of-materials)
- [Wiring](#wiring)
- [Operating Schedule](#operating-schedule)
- [Patterns and Helpers](#patterns-and-helpers)
- [Full Sketch](#full-sketch)
- [Troubleshooting & Safety](#troubleshooting--safety)
- [Future Ideas](#future-ideas)
- [License](#license)


## Quick Start

1. **부품 준비**
   - Arduino Uno 또는 Nano
   - 28BYJ-48 스텝모터 + ULN2003 드라이버
   - DS3231 RTC 모듈
   - 브레드보드, 점퍼 케이블, 5V 전원(USB 어댑터 등)
   - 얇은 실(낚싯줄·테그줄 등)과 가벼운 거미 인형
2. **배선 요약**
   - RTC: SDA → A4, SCL → A5, VCC → 5V, GND → GND
   - ULN2003: IN1 → D8, IN2 → D9, IN3 → D10, IN4 → D11, VCC → 5V, GND → GND
   - 모든 GND는 반드시 공통으로 묶습니다.
3. **코드 업로드**
   - 아래 “Full Sketch” 섹션의 코드를 `spider_controller/spider_controller.ino`에 넣고
     Arduino IDE 또는 PlatformIO로 보드/포트를 지정해 업로드합니다.
4. **첫 실행**
   - 시리얼 모니터(9600bps)로 로그를 확인합니다.
   - 설치 후 `maxDownSteps` 값을 조정해 실제 이동 거리가 적절한지 확인합니다.

## Full System Concept

1. 거미 인형을 얇은 실에 매답니다.
2. 실의 윗쪽을 릴(드럼)에 감고 릴을 28BYJ-48 스텝모터 축에 고정합니다.
3. 스텝모터가 정/역방향으로 회전하며 줄을 감고 풀어 거미를 위아래로 움직입니다.
4. DS3231 RTC 모듈로 현재 시간을 읽어
   - 오전 09:20~09:40
   - 오후 18:00~18:20
   동안에만 랜덤 패턴으로 “살아있는” 움직임을 만듭니다.

릴 반지름을 약 1.5~2cm로 맞추고 `maxDownSteps`(대략 2000~2200 스텝)을 튜닝하면
실제 거미가 약 10cm 이상 내려오도록 조절할 수 있습니다.

## Bill of Materials

- Arduino Uno 또는 Nano
- 28BYJ-48 스텝모터 + ULN2003 드라이버 보드
- DS3231 RTC 모듈
- 브레드보드, 점퍼 케이블
- 5V 전원 (USB 어댑터 또는 5V 1A 정도의 어댑터)
- 낚싯줄/얇은 실/테그줄
- 가벼운 거미 장난감 또는 피규어
- (옵션) 마지막 퇴근 시 거미를 완전 원위치시키는 버튼 1개

## Wiring

### DS3231 RTC ↔ Arduino

| DS3231 핀 | Arduino 핀 | 기능            |
|-----------|------------|-----------------|
| VCC       | 5V         | 전원 공급       |
| GND       | GND        | 공통 접지       |
| SDA       | A4         | I²C 데이터 라인 |
| SCL       | A5         | I²C 클럭 라인   |

### 28BYJ-48 + ULN2003 ↔ Arduino

1. **스텝모터 ↔ ULN2003**  
   모터의 흰색 커넥터를 ULN2003 드라이버 보드 소켓에 그대로 꽂습니다.

2. **ULN2003 ↔ Arduino**

| ULN2003 핀 | Arduino 핀 | 설명        |
|------------|------------|-------------|
| IN1        | D8         | 제어 신호 1 |
| IN2        | D9         | 제어 신호 2 |
| IN3        | D10        | 제어 신호 3 |
| IN4        | D11        | 제어 신호 4 |
| + (VCC)    | 5V         | 모터 전원   |
| – (GND)    | GND        | 공통 GND    |

3. **전원**
   - 가장 간단한 구성: Arduino 5V 핀 하나로 RTC와 ULN2003 모두 공급.
   - 더 안전하려면 외부 5V 어댑터를 사용하고 Arduino 5V에도 같은 전원을 넣되,
     **GND는 반드시 공통**으로 묶습니다.

### Optional Button

마지막 퇴근자가 눌러 거미를 완전히 위로 올릴 버튼을 추가하려면 아래처럼 연결합니다.

| 버튼 한쪽 | 버튼 다른쪽 |
|-----------|-------------|
| D2        | GND         |

```cpp
pinMode(2, INPUT_PULLUP);
int buttonState = digitalRead(2); // LOW면 버튼 눌림
```

## Operating Schedule

- **아침**: 09:20 ~ 09:40  
- **저녁**: 18:00 ~ 18:20  
  위 시간대에만 거미가 랜덤 패턴으로 움직이는 “살아있는 모드”로 동작합니다.

### 세션/패턴 개념

1. 활성 시간대 동안 일정 간격으로 새 “세션”을 시작합니다.
2. 세션이 시작되면 1~3개의 패턴을 랜덤으로 선택해 순서대로 실행합니다.
3. 각 패턴 사이에는 1~5초 랜덤 정지가 있어 진짜 거미처럼 멈춰 있습니다.
4. 세션이 끝나면 다음 세션까지 10~40초 랜덤 휴식을 줍니다.

### 비활성 시간대

- 시간대 밖에서는 거미가 거의 움직이지 않고,
- `currentSteps > 0`일 경우 천천히 위로 올라오며 상단 근처에서 대기합니다.

## Step Count vs Distance

- 28BYJ-48은 일반적으로 2048스텝이 한 바퀴입니다.
- 릴 반지름을 약 1.5cm로 잡으면 둘레 ≈ 9.4cm → 2048스텝 ≈ 9.4cm.
- 약 10cm를 내려오게 하려면 2000~2200 스텝 정도가 필요합니다.
- `maxDownSteps = 2200` 정도로 시작하고 설치 환경에 맞춰 조정합니다.

## Patterns and Helpers

거미 위치 관리를 위한 공통 변수:

```cpp
long currentSteps = 0;          // 0 = 가장 위
const long maxDownSteps = 2200; // 릴/실에 맞게 조정
```

### Step Movement Helper

```cpp
void moveSpiderSteps(long steps) {
  long target = currentSteps + steps;

  if (target < 0) {
    steps = -currentSteps;  // 최상단보다 위로 못 감
    target = 0;
  } else if (target > maxDownSteps) {
    steps = maxDownSteps - currentSteps; // 하강 한계 초과 방지
    target = maxDownSteps;
  }

  if (steps == 0) return;

  spiderStepper.step(steps);
  currentSteps = target;
}
```

### Example Patterns

1. **소심한 꿈틀 (작게 통통)**
   ```cpp
   void patternSmallWiggle() {
     Serial.println("Pattern: Small wiggle");
     int amp = 150;
     for (int i = 0; i < 3; i++) {
       moveSpiderSteps(amp);
       delay(200);
       moveSpiderSteps(-amp);
       delay(200);
     }
   }
   ```
2. **갑자기 휙 내려갔다 살짝 올라오기**
   ```cpp
   void patternDropAndSlightUp() {
     Serial.println("Pattern: Drop and slight up");
     int drop = random(400, 700);
     int up   = random(100, 250);
     moveSpiderSteps(drop);
     delay(300);
     moveSpiderSteps(-up);
     delay(300);
   }
   ```
3. **위로 슬쩍 올라갔다 원위치 복귀**
   ```cpp
   void patternUpAndBack() {
     Serial.println("Pattern: Up and back");
     int up = random(200, 400);
     moveSpiderSteps(-up);
     delay(400);
     moveSpiderSteps(up);
     delay(400);
   }
   ```
4. **긴 숨 고르기 (천천히 기어가기)**
   ```cpp
   void patternSlowMove() {
     Serial.println("Pattern: Slow move");
     int dir = (random(0, 2) == 0) ? 1 : -1;
     int dist = random(300, 800);
     int stepSize = (dir > 0) ? 10 : -10;
     int stepsMoved = 0;
     while (abs(stepsMoved) < dist) {
       moveSpiderSteps(stepSize);
       stepsMoved += stepSize;
       delay(80);
     }
   }
   ```

랜덤 패턴 선택:

```cpp
void runRandomPattern() {
  int p = random(0, 4);
  switch (p) {
    case 0: patternSmallWiggle();      break;
    case 1: patternDropAndSlightUp();  break;
    case 2: patternUpAndBack();        break;
    case 3: patternSlowMove();         break;
  }
}
```

## Full Sketch

RTC(DS3231) + 28BYJ-48 + ULN2003 + Arduino 기반 전체 예제 스케치입니다.

```cpp
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
const long maxDownSteps = 2200;    // 약 10cm 정도 내려오는 한계

// ===== 패턴 실행 타이밍 =====
unsigned long nextActionTime = 0;  // 다음 세션 시작 예정 시간 (millis)

// ======================== 기본 설정 ========================
void setup() {
  Serial.begin(9600);
  Wire.begin();
  rtc.begin();

  // 스텝모터 속도 (RPM). 너무 빠르면 토크 부족/미끄러짐.
  spiderStepper.setSpeed(10); // 10RPM 정도로 천천히

  // ★ RTC 시간 최초 1회 설정용 (사용 후 주석 처리)
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
```

## Troubleshooting & Safety

- **거미가 움직이지 않음**: 전원, ULN2003 연결, RTC 동작(시간 동기)을 확인합니다.
- **위치가 틀어짐**: `currentSteps`와 `maxDownSteps` 값을 조정합니다.
- **소음/전류 부족**: 별도의 5V 전원을 사용해 토크를 확보합니다.
- **안전**: 한계 스위치를 추가하면 과도한 하강/상승을 방지하고,
  설치 위치 주변 장애물을 제거해 줄 꼬임을 예방할 수 있습니다.

## Future Ideas

- **퇴근 버튼 연동**: 버튼을 눌러 `moveSpiderSteps(-currentSteps);`를 실행하면
  거미를 완전히 위로 올리고 그날은 더 이상 움직이지 않게 만들 수 있습니다.
- **LED 눈 추가**: 거미 눈에 LED를 달아 패턴 실행 중일 때만 켜지게 하면 효과가 큽니다.
- **ESP32 확장**: Wi-Fi로 NTP 시간 동기화 후, “오늘 몇 번 움직였는지”를 서버나 시트에 기록하는 등 확장이 가능합니다.

## License

프로젝트 라이선스는 `LICENSE` 파일을 확인하세요. 문의나 아이디어는 이슈/PR로 환영합니다.
