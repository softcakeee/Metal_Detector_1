#include <SPI.h>
#include "nRF24L01.h"
#include "RF24.h"
 
#define CE_PIN 8
#define CSN_PIN 10
 
int Cap_val[2];
float dis = 0;
RF24 radio(CE_PIN, CSN_PIN);
const uint64_t pipe = 0xE8E8F0F0E1LL;
 
void setup(void)
{
  Serial.begin(9600);
  Serial.println("CLEARDATA");
  Serial.println("LABEL,dis,Cap_Val");
  
  radio.begin();
  radio.setPALevel(RF24_PA_MIN); // LOW, MAX
  radio.openReadingPipe(1,pipe);
  radio.startListening();
}
void loop(void)
{
  if ( radio.available() )
  {
      radio.read( Cap_val, sizeof(Cap_val) );
      Serial.print("DATA,"); //데이터 행에 데이터를 받겠다는 말
      Serial.print(dis);
      Serial.print(",");
      Serial.println(Cap_val[0]);
      dis = dis+4.7;
    
    
  }
}