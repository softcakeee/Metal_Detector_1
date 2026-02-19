#include <avr/io.h> //avr 포트 사용 헤더파일
#include <avr/interrupt.h> //avr 인터럽트 헤더파일
#include <stdint.h> // 기본 int_t 사용하기 위한 std int
#include <SPI.h> // SPI 헤더파일
#include "nRF24L01.h" // nRF24L01 헤더파일
#include "RF24.h"// nRF24L01 헤더파일

#define capPin 5
#define pulsePin 4
#define HallPin 3


#define CE_PIN 8
#define CSN_PIN 7

RF24 radio(CE_PIN, CSN_PIN); //CE , CSN 핀 세팅
const uint64_t pipe = 0xE8E8F0F0E1LL; //SPI RF 주소 지정

void setup() //기본 uart , 포트 설정
{
    UBRR0H = 0;
    UBRR0L = 103; // 9600 Baudrate 지정
    UCSR0B = (1 << TXEN0); // TX 지정
    UCSR0C = (1 << UCSZ01) | (1 << UCSZ00); // 8bit 데이터 사용

    DDRC &= ~(1 << DDC5); // C포트 C5 INPUT 지정 (CAP 값 읽는 ADC 사용핀)
    DDRC |= (1 << DDC4);  // C포트 C4 OUTPUT 지정 (1kHZ 펄스 생성핀 output 지정)
    PORTC &= ~(1 << PORTC4); // C포트 C4 LOW 설정
    DDRC &= ~(1 << DDC3); // HallPin as INPUT
    DDRB |= (1 << DDB2);  // B포트 B2 출력 (LED)
    DDRB |= (1 << DDB1);  // B포트 B1 출력 (BUZZER)
    PORTB &= ~(1 << PORTB1); // 부저 LOW 설정

    radio.begin(); // nRF24L01 기본 세팅
    radio.setPALevel(RF24_PA_MIN); // 송신 전력 지정
    radio.openWritingPipe(pipe); // 송신 주소 열기
}

void applyPulses()//펄스 발생 함수
{
    for (int i = 0; i < 3; i++) 
    {
        PORTC |= (1 << PORTC4); // 펄스 HIGH
        _delay_us(3); //3uS 딜레이
        PORTC &= ~(1 << PORTC4); // 펄스 LOW
        _delay_us(3); //3uS 딜레이
    }
}

void DLBProc(uint16_t cap_val)//LED , BUZZER ON /OFF 함수
{
    if(cap_val < 101)
    {
        PORTB |= (1 << PORTB2);  // led 켜기
        OCR0A = 50; // 부저 SIG 핀으로 PWM 값 지정
        TCCR0A |= (1 << COM0A0) | (1 << WGM01); // compare match , ctc 모드 활용하여 timer 0 로 PWM 발생
        TCCR0B |= (1 << CS01); // 프리스케일러 8 지정
    }
    else 
    {
        PORTB &= ~(1 << PORTB2); // led 끄기
        TCCR0A &= ~(1 << COM0A0); // 부저 끄기
        TCCR0B &= ~(1 << CS01);
    }
}

void CAP_SET()//CAP 방전 시키는 함수
{
    DDRC |= (1 << DDC5);  // CAP 읽는 핀을 OUT 으로 바꾼후
    PORTC &= ~(1 << PORTC5); // LOW 해서 충전된 CAP 을 방전 시킨다.
    _delay_us(20);
    DDRC &= ~(1 << DDC5);  // CAP 방전후 다시 INPUT 으로 변환
}

uint16_t analogReadCustom(uint8_t pin)//ADC 값 읽는 함수
{
    ADMUX = (1 << REFS0) | (pin & 0x07); // ADC 1 ~ 7 번중 몇번 ADC 를 사용할것인지 지정
    ADCSRA |= (1 << ADEN); // ADC 키기
    ADCSRA |= (1 << ADSC); // 비트 설정후 ADC 변환 시작
    while (ADCSRA & (1 << ADSC)); // ADC 변환 끝날때까지 while 
    return ADC; // ADC 변환 값 return
}

void loop() 
{
    CAP_SET(); //cap 방전 밑 세팅
    applyPulses(); // 1kHz 펄스
    uint16_t cap_val = analogReadCustom(capPin); // cap 값 adc읽기
    uint16_t Hall_val = analogReadCustom(HallPin); // 홀센서 값 adc읽기
    DLBProc(cap_val); //Detect LED BUZZER 로 LED,BUZ 설정 함수
    
    if (Hall_val < 40 && Hall_val > 20) // 홀센서 값이 인식되면 실행되는 조건문
    {
        radio.write(&cap_val, sizeof(cap_val)); //cap 값 송신
        _delay_ms(400); // 딜레이
    }
}



