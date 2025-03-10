//siuncia tmp102 rodmenis
#include <Wire.h> 
#include "LoRaWan_APP.h"
// #include "Arduino.h"
#include "HT_SSD1306Wire.h"
#include <SparkFunTMP102.h> // Used to send and recieve specific information from our sensor
// Connections
// VCC = 3.3V
// GND = GND
// SDA = 41
// SCL = 42
const int ALERT_PIN = A3;

TMP102 sensor0;
// Sensor address
// ADD0 - Address
//  VCC - 0x49
//  SDA - 0x4A
//  SCL - 0x4B

#define RF_FREQUENCY                                870000000 // Hz

#define TX_OUTPUT_POWER                             5        // dBm

#define LORA_BANDWIDTH                              0         // [0: 125 kHz,
                                                              //  1: 250 kHz,
                                                              //  2: 500 kHz,
                                                              //  3: Reserved]
#define LORA_SPREADING_FACTOR                       7         // [SF7..SF12]
#define LORA_CODINGRATE                             1         // [1: 4/5,
                                                              //  2: 4/6,
                                                              //  3: 4/7,
                                                              //  4: 4/8]
#define LORA_PREAMBLE_LENGTH                        8         // Same for Tx and Rx
#define LORA_SYMBOL_TIMEOUT                         0         // Symbols
#define LORA_FIX_LENGTH_PAYLOAD_ON                  false
#define LORA_IQ_INVERSION_ON                        false


#define RX_TIMEOUT_VALUE                            1000
#define BUFFER_SIZE                                 30 // Define the payload size here

char txpacket[BUFFER_SIZE];
// char txpacket_2[BUFFER_SIZE];

char rxpacket[BUFFER_SIZE];

double txNumber;

bool lora_idle=true;

static RadioEvents_t RadioEvents;
void OnTxDone( void );
void OnTxTimeout( void );



void setup() {
    Serial.begin(115200);
       Mcu.begin(HELTEC_BOARD,SLOW_CLK_TPYE);
	
    txNumber=0;

    RadioEvents.TxDone = OnTxDone;
    RadioEvents.TxTimeout = OnTxTimeout;
    
    Radio.Init( &RadioEvents );
    Radio.SetChannel( RF_FREQUENCY );
    Radio.SetTxConfig( MODEM_LORA, TX_OUTPUT_POWER, 0, LORA_BANDWIDTH,
                                   LORA_SPREADING_FACTOR, LORA_CODINGRATE,
                                   LORA_PREAMBLE_LENGTH, LORA_FIX_LENGTH_PAYLOAD_ON,
                                   true, 0, 0, LORA_IQ_INVERSION_ON, 3000 ); 

//********************tmp for rhe next

   Wire.begin(); //Join I2C Bus
pinMode(ALERT_PIN,INPUT);  // Declare alertPin as an input
  
  /* The TMP102 uses the default settings with the address 0x48 using Wire.
  
     Optionally, if the address jumpers are modified, or using a different I2C bus,
	 these parameters can be changed here. E.g. sensor0.begin(0x49,Wire1)
	 
	 It will return true on success or false on failure to communicate. */
  if(!sensor0.begin())
  {
    Serial.println("Cannot connect to TMP102.");
    Serial.println("Is the board connected? Is the device ID correct?");
    while(1);
  }
  
  Serial.println("Connected to TMP102!");
  delay(100);

  // Initialize sensor0 settings
  // These settings are saved in the sensor, even if it loses power
  
  // set the number of consecutive faults before triggering alarm.
  // 0-3: 0:1 fault, 1:2 faults, 2:4 faults, 3:6 faults.
  sensor0.setFault(0);  // Trigger alarm immediately
  
  // set the polarity of the Alarm. (0:Active LOW, 1:Active HIGH).
  sensor0.setAlertPolarity(1); // Active HIGH
  
  // set the sensor in Comparator Mode (0) or Interrupt Mode (1).
  sensor0.setAlertMode(0); // Comparator Mode.
  
  // set the Conversion Rate (how quickly the sensor gets a new reading)
  //0-3: 0:0.25Hz, 1:1Hz, 2:4Hz, 3:8Hz
  sensor0.setConversionRate(3);
  
  //set Extended Mode.
  //0:12-bit Temperature(-55C to +128C) 1:13-bit Temperature(-55C to +150C)
  sensor0.setExtendedMode(0);

  //set T_HIGH, the upper limit to trigger the alert on
  // sensor0.setHighTempF(85.0);  // set T_HIGH in F
  sensor0.setHighTempC(3.5); // set T_HIGH in C
  
  //set T_LOW, the lower limit to shut turn off the alert
  // sensor0.setLowTempF(84.0);  // set T_LOW in F
  sensor0.setLowTempC(0.5); // set T_LOW in C
   }


void loop()
{
  //************** tmp
 float temperature;
  boolean alertPinState, alertRegisterState;
  
  // Turn sensor on to start temperature measurement.
  // Current consumtion typically ~10uA.
  sensor0.wakeup();

  // read temperature data
  // temperature = sensor0.readTempF();
  temperature = sensor0.readTempC();

  // Check for Alert
  alertPinState = digitalRead(ALERT_PIN); // read the Alert from pin
  alertRegisterState = sensor0.alert();   // read the Alert from register
  
  // Place sensor in sleep mode to save power.
  // Current consumtion typically <0.5uA.
  sensor0.sleep();

  // Print temperature and alarm state
  Serial.print("Temperature: ");
  Serial.print(temperature);
  
  Serial.print("\tAlert Pin: ");
  Serial.print(alertPinState);
  
  Serial.print("\tAlert Register: ");
  Serial.println(alertRegisterState);
  
  delay(1000);  // Wait 1000ms
// display.clear();
   

	if(lora_idle == true)
	{
    delay(1000);
	txNumber += 0.01;
		sprintf(txpacket," %0.2f, Temp %0.2f ",txNumber,temperature );  //start a package
    // sprintf(txpacket,"Temperatura %0.2f",temperature);  //start a package
   
		Serial.printf("\r\nsending packet \"%s\" , length %d\r\n",txpacket, strlen(txpacket));
    // Serial.printf("\r\nsending packet \"%s\" , length %d\r\n",txpacket_2, strlen(txpacket_2));

		Radio.Send( (uint8_t *)txpacket, strlen(txpacket) ); //send the package out	
    // Radio.Send( (uint8_t *)txpacket_2, strlen(txpacket_2) ); //send the package 2 out	
    lora_idle = false;
	}
  Radio.IrqProcess( );
 }

void OnTxDone( void )
{
	Serial.println("TX done......");
	lora_idle = true;
}

void OnTxTimeout( void )
{
    Radio.Sleep( );
    Serial.println("TX Timeout......");
    lora_idle = true;
}