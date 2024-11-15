unsigned long newtimePWM, oldtimePWM;
uint8_t oldtime = 0;
char nWidgets = 9;
const char nWidgetsArray = 9;
short int stat[nWidgetsArray];
//char widget[nWidgetsArray]; // inputWidjet[0] = 'unknown';1 = 'toggle';2 = 'simple-btn';4 = 'range';4 = 'small-badge';5 = 'chart';
char descr[nWidgetsArray][10];
char id[nWidgetsArray];
unsigned char pin[nWidgetsArray];
short int defaultVal[nWidgetsArray];
char IrButtonID[nWidgetsArray];
float analogDivider = 1.0F;
short int analogSubtracter = 0;
unsigned int low_pwm[nWidgetsArray];
bool low_pwm_off = false;             // low_pwm
unsigned char pinmode[nWidgetsArray]; // inputPinmode[0] = "no pin";inputPinmode[1] = "in";inputPinmode[2] = "out";inputPinmode[3] = "pwm";inputPinmode[4] = "adc";inputPinmode[5] = "low_pwm";inputPinmode[6] = "IR";inputPinmode[7] = "датчик газа MQ7";

unsigned char subscribe_loop = 0;
uint8_t subscr_sec = 5;
uint8_t mqttspacing_oldtime;
// WiFiClient wclient;

char *delimeter(char *str, char *separator, char num)
{
  // printf ("%s\n","ok");
  char *pch;
  pch = strtok(str, separator);
  uint8_t i;
  while (pch != NULL)
  {

    // printf ("%s\n",pch);
    if (i == num)
    {
      break;
    }
    pch = strtok(NULL, separator);
    i++;
  }
  return pch;
}
