/*
Simple Deep Sleep with Timer Wake Up
=====================================
ESP32 offers a deep sleep mode for effective power
saving as power is an important factor for IoT
applications. In this mode CPUs, most of the RAM,
and all the digital peripherals which are clocked
from APB_CLK are powered off. The only parts of
the chip which can still be powered on are:
RTC controller, RTC peripherals ,and RTC memories

This code displays the most basic deep sleep with
a timer to wake it up and how to store data in
RTC memory to use it over reboots

This code is under Public Domain License.

Author:
Pranav Cherukupalli <cherukupallip@gmail.com>
*/


#define uS_TO_S_FACTOR 1000000  /* Conversion factor for micro seconds to seconds */
#define TIME_TO_SLEEP  5        /* Time ESP32 will go to sleep (in seconds) */

RTC_DATA_ATTR int bootCount = 0;

/*
Method to print the reason by which ESP32
has been awaken from sleep
*/
byte TimerWakeUp_print_wakeup_reason(){
  esp_sleep_wakeup_cause_t wakeup_reason;

  wakeup_reason = esp_sleep_get_wakeup_cause();

  switch(wakeup_reason)
  {
    case 1  : Serial.print("Wakeup caused by external signal using RTC_IO "); break;
    case 2  : Serial.print("Wakeup caused by external signal using RTC_CNTL "); break;
    case 3  : Serial.print("Wakeup caused by timer "); break;
    case 4  : Serial.print("Wakeup caused by touchpad "); break;
    case 5  : Serial.print("Wakeup caused by ULP program "); break;
    default : Serial.print("Wakeup was not caused by deep sleep "); break;
  }
  Serial.println( "(" + String(wakeup_reason) + ")");
  return wakeup_reason;
}

void TimerWakeUp_setSleepTime(int time_sec){
  esp_sleep_enable_timer_wakeup(time_sec * uS_TO_S_FACTOR);
  Serial.println("Setup ESP32 to sleep for every " + String(time_sec) +
  " Seconds");
}

void TimerWakeUp_setExternalInput(gpio_num_t gpio, int level){
  /* @param mask  bit mask of GPIO numbers which will cause wakeup. Only GPIOs
   *              which are have RTC functionality can be used in this bit map:
   *              0,2,4,12-15,25-27,32-39.
  */
  esp_sleep_enable_ext0_wakeup(gpio,level);
  Serial.println("Wakeup ESP32 when IO" + String(gpio) + " = " + String(level));
}

byte TimerWakeUp_init(){
  ++bootCount;	//Increment boot number and print it every reboot
  Serial.println("Boot number: " + String(bootCount));
  return TimerWakeUp_print_wakeup_reason();  //Print the wakeup reason for ESP32
}

void TimerWakeUp_sleep(){
  Serial.println("Going to sleep now");
  delay(100);
  Serial.flush(); 
  // タッチパッド起動を有効にしていなくても動作してしまう不具合対策
  esp_sleep_disable_wakeup_source(ESP_SLEEP_WAKEUP_TOUCHPAD);
  Serial.println("Disabled wakeup touchpad, please ignore above \"E () sleep: Incorrect\"");
  esp_deep_sleep_start();
}

void TimerWakeUp_setup(){
  Serial.begin(115200);
  delay(1000); //Take some time to open up the Serial Monitor

  //Increment boot number and print it every reboot
  ++bootCount;
  Serial.println("Boot number: " + String(bootCount));

  //Print the wakeup reason for ESP32
  TimerWakeUp_print_wakeup_reason();

  /*
  First we configure the wake up source
  We set our ESP32 to wake up every 5 seconds
  */
  esp_sleep_enable_timer_wakeup(TIME_TO_SLEEP * uS_TO_S_FACTOR);
  Serial.println("Setup ESP32 to sleep for every " + String(TIME_TO_SLEEP) +
  " Seconds");

  /*
  Next we decide what all peripherals to shut down/keep on
  By default, ESP32 will automatically power down the peripherals
  not needed by the wakeup source, but if you want to be a poweruser
  this is for you. Read in detail at the API docs
  http://esp-idf.readthedocs.io/en/latest/api-reference/system/deep_sleep.html
  Left the line commented as an example of how to configure peripherals.
  The line below turns off all RTC peripherals in deep sleep.
  */
  //esp_deep_sleep_pd_config(ESP_PD_DOMAIN_RTC_PERIPH, ESP_PD_OPTION_OFF);
  //Serial.println("Configured all RTC Peripherals to be powered down in sleep");

  /*
  Now that we have setup a wake cause and if needed setup the
  peripherals state in deep sleep, we can now start going to
  deep sleep.
  In the case that no wake up sources were provided but deep
  sleep was started, it will sleep forever unless hardware
  reset occurs.
  */
  Serial.println("Going to sleep now");
  Serial.flush(); 
  esp_deep_sleep_start();
  Serial.println("This will never be printed");
}

void TimerWakeUp_loop(){
  //This is not going to be called
}
