#include "stm32f4xx.h"
#include "adc.h"
#include <math.h>
#include <mq2_ppm.h>
#include <stdio.h>

#define VREF     3.3f
#define ADC_RES  4095.0f   // 12-bit ADC
#define RL       10000.0f  // 10 kΩ
#define RO       1000.0f  // 24 kΩ (calibrated in clean air)
#define A        565.0f
#define B        -1.48f

// LPG(A=565,B=-1.48), CH4(A=393,B=-1.18), CO(A=472,B=-1.28), H2(A=575,B=-1.26)



float MQ2_PPM_OP(void) {
	start_conversion_adc();
	uint32_t adc_val = adc_read();
	float voltage = ((float)adc_val / ADC_RES) * VREF;
	 if (voltage < 0.01f) {
	        // Avoid division by zero or invalid reading
	        return -1;
	    }
	 float Rs = RL * (VREF - voltage) / voltage;
	 float ratio = Rs / RO;
	 float ppm = powf(10, (log10f(ratio) - B) / A);
   return ppm;
}



