

#include "periph/adc.h"

#include "xenbat.h"

#define BAT_LINE            ADC_LINE(3)
#define BAT_MUL             (95898)     /* (V_max * (R15 + R16) * 1000) / 100 */
#define BAT_DIV             (21504)     /* (R16 * ADC_max) / 100  */


void xenbat_init(void)
{
    adc_init(BAT_LINE);
}

uint16_t xenbat_sample(void)
{
    int raw = adc_sample(BAT_LINE, ADC_RES_10BIT);
    return (uint16_t)((raw * BAT_MUL) / BAT_DIV);
}

