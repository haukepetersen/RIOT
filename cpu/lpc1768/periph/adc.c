









int adc_init(adc_t dev, adc_precision_t precision)
{
    if (adc != ADC_0) {
        return -1;
    }

    /* enable and select clock */
    ADC_0_CLKSEL();
    ADC_0_CLKEN();
    /* pin configuration */
    ADC_0_CH0_PINSEL &= ~(0x3 << ADC_0_CH0_POS);
    ADC_0_CH0_PINSEL |= (0x1 << ADC_0_CH0_POS);
    ADC_0_CH0_PINMODE &= ~(0x3 << ADC_0_CH0_POS);
    ADC_0_CH0_PINMODE |= (0x2 << ADC_0_CH0_POS);
    ADC_0_CH1_PINSEL &= ~(0x3 << ADC_0_CH1_POS);
    ADC_0_CH1_PINSEL |= (0x1 << ADC_0_CH1_POS);
    ADC_0_CH1_PINMODE &= ~(0x3 << ADC_0_CH1_POS);
    ADC_0_CH1_PINMODE |= (0x2 << ADC_0_CH1_POS);
    ADC_0_CH2_PINSEL &= ~(0x3 << ADC_0_CH2_POS);
    ADC_0_CH2_PINSEL |= (0x1 << ADC_0_CH2_POS);
    ADC_0_CH2_PINMODE &= ~(0x3 << ADC_0_CH2_POS);
    ADC_0_CH2_PINMODE |= (0x2 << ADC_0_CH2_POS);
    /* set CLKDIV for ADC-CLK of 12MHz*/
    ADD_0_DEV->CR = (1 << 8);
}
