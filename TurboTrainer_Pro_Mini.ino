/*
  TurboTrainer Generator
  28/7/2022
  
  Description:
    - A 149kv outrunner is spun by a stationary bike flywheel via a flat belt. The drive chain is super efficient.
    - A 6 position rotary switch connected to some resistors generates 6 different analog voltages which selects the power
    - We operate at 50,100,150,200,250,300 Watts (Mechanical) and use a PI controller to adjust the resitive torque to realise these powers

    - Drive chain losses + Motor Rotation losses are approximately 25 Watts. 
        # I calculated this by running the outrunner as a motor to spin the pedals at a sensible cadence. Doing this used about 25W. 
        # At a very slow pedal (ERPM 8000) I measured 13W and at a very fast pedal (ERPM 18000) I measured 31W
        # This assumes the losses are equal in positive and negative torque directions - I don't see why they wouldn't be       
*/

#include      <VescUart.h>
#include      <PID_v1.h>
#include      <MovingAverage.h>

//------------------------------------------------------------------------------------------------
#define       LED_PIN               10
#define       ROTARY_SWITCH_PIN     A0  

#define       LED_SLOW_BLINK        0
#define       LED_FAST_BLINK        1
#define       LED_ON                2
#define       AS_BEFORE             9

#define       ON                    1
#define       OFF                   0

//------------------------------------------------------------------------------------------------
VescUart      VESC;                                                                               // VescUart class
double        Setpoint, Power, BrakeCurrent, Target_Setpoint;
MovingAverage <uint16_t, 8> ADC_Filter;

double        Kp=0.0, Ki=0.5, Kd=0.0;                                                             // PID Tuning parameters
PID           myPID(&Power, &BrakeCurrent, &Setpoint, Kp, Ki, Kd, DIRECT);

//------------------------------------------------------------------------------------------------
void Flash_LED(byte Flash_Mode);

// -------------------------------------------------------------------------------------------------------------------------------------------------
// #################################################################################################################################################
// -------------------------------------------------------------------------------------------------------------------------------------------------
void setup() 
{
  Serial.begin(115200);  
  while (!Serial) {;}
  
  Serial.setTimeout(40);
  
  VESC.setSerialPort(&Serial); 

  myPID.SetSampleTime(50);
  myPID.SetOutputLimits(0.0,32.0);                                                                // Max current we can use to slow the motor - more than this and the belt might slip  

  pinMode(LED_PIN, OUTPUT);
}

// -------------------------------------------------------------------------------------------------------------------------------------------------
// #################################################################################################################################################
// -------------------------------------------------------------------------------------------------------------------------------------------------
void loop() { 
  static unsigned long TimeStamp = millis();
  static int MOTOR_ENABLED = false;

  TimeStamp += 50;
  while(TimeStamp > millis()) {
    Flash_LED(AS_BEFORE);
  }  
  
  // Acquire data from VESC
  if (VESC.getVescValues()) {
    Power = -VESC.data.avgInputCurrent * VESC.data.inpVoltage;
       
    if(VESC.data.rpm > 6000.0)                        
    { 
      myPID.SetMode(AUTOMATIC);
      Setpoint += 10.0;                                                                           // This ramps up the power setpoint by 200 Watts/Second 
      Setpoint = min(Setpoint, Target_Setpoint); 
      Flash_LED(LED_ON);                                                            
    }   
    else if (VESC.data.rpm < 4000.0)  
    {  
      myPID.SetMode(MANUAL);
      Setpoint = 0.0;     
      BrakeCurrent = 0.0; 

      if (VESC.data.rpm < 100.0)
        Flash_LED(LED_SLOW_BLINK);   
      else
        Flash_LED(LED_FAST_BLINK);                                       
    } 

    myPID.Compute();
    VESC.setBrakeCurrent(BrakeCurrent); 
  }

  // Read the rotary switch (It produces different anolog voltages) to select the power
  ADC_Filter.add(analogRead(ROTARY_SWITCH_PIN));
  int Rotary_ADC = ADC_Filter.get();

  if(Rotary_ADC > 922)                                                                            // The VESC is only somewhat accurate. These are the required values based on my calibrations
    Target_Setpoint = 38.0;                                                                       // This results in 50W mechanical pedal power
  else if(Rotary_ADC > 718)
    Target_Setpoint = 94.0;                                                                       // This results in 100W mechanical pedal power
  else if(Rotary_ADC > 514)
    Target_Setpoint = 151.0;                                                                      // This results in 150W mechanical pedal power
  else if(Rotary_ADC > 308)
    Target_Setpoint = 207.0;                                                                      // This results in 200W mechanical pedal power
  else if(Rotary_ADC > 104)
    Target_Setpoint = 264.0;                                                                      // This results in 250W mechanical pedal power
  else
    Target_Setpoint = 320.0;                                                                      // This results in 300W mechanical pedal power
}

void Flash_LED(byte Flash_Mode)
{
  static unsigned long on_time, off_time, timestamp = 0;
  static byte LED_Status = OFF;
  
  switch(Flash_Mode)  {
    case LED_SLOW_BLINK:
      on_time = 200;
      off_time = 800;
      break;
    case LED_FAST_BLINK:
      on_time = 150;
      off_time = 150;
      break;      
    case LED_ON:
      on_time = 1000;
      off_time = 0;
      break;
  }

  if(millis() > timestamp) {
    if(LED_Status == OFF) {
      timestamp = millis() + on_time;
      digitalWrite(LED_PIN, HIGH);
      LED_Status = ON;
    }
    else {
      timestamp = millis() + off_time;
      digitalWrite(LED_PIN, LOW);
      LED_Status = OFF;      
    }    
  }
}
