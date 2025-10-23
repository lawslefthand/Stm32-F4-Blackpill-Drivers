// This is a sample code for testing SH1106 driver functions
//Ouput of this code is attached below main.c


#include "OLED.h"
#include "bitmap.h"


int _write(int file, char *ptr, int len)
{
  (void)file;
  int DataIdx;

  for (DataIdx = 0; DataIdx < len; DataIdx++)
  {
    ITM_SendChar(*ptr++);
  }
  return len;
}


int main(void)
{
   I2C1_init();
   delay_ms(100);

   printf("Starting I2C Scanner...\n");
   i2c_scanner();

   printf("Initializing SH1106...\n");
   SH1106_init();
   printf("Display Initialized\n");

   clear();
   write(0,1,"YASH_yash 12345");
   write(0,3,"One frame man");
   delay_ms(1000);
   
   clear();
   rect(0,0,120,60);
   delay_ms(1000);
   
   clear();
   invert_rect(0,0,120,60);
   delay_ms(1000);
   
   
   clear();
   drawline_H(0,10,128);
   drawline_H(0,20,128);
   drawline_H(0,30,128);
   drawline_H(0,40,128);
   drawline_H(0,50,128);
   drawline_H(0,60,128);
   delay_ms(1000);
   
   clear();
   drawline_V(10,0,64);
   drawline_V(20,0,64);
   drawline_V(30,0,64);
   drawline_V(40,0,64);
   drawline_V(50,0,64);
   drawline_V(60,0,64);
   drawline_V(70,0,64);
   drawline_V(80,0,64);
   drawline_V(90,0,64);
   drawline_V(100,0,64);
   drawline_V(110,0,64);
   delay_ms(1000);
   
   clear();
   drawBitmap(0,0,120,60,panther);
   delay_ms(1000);
   
   clear();
   drawBitmap(0,0,120,62,spider);
   delay_ms(1000);
   
   clear();
   drawBitmap(0,0,120,60,batman);

}

