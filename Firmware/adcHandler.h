#define ADC1 15
#define ADC2 14
#define ADC3 13
#define ADC4 12
#define BATT_ADC 35

float getADC(uint8_t adcNumber, String multiplierV = "1.0")
{
    float v, mul;
    mul = multiplierV.toFloat();
    if (adcNumber == 1)
    {
        v = analogRead(ADC1);
        v = v * mul;
        return v;
    }
    else if (adcNumber == 2)
    {
        v = analogRead(ADC2);
        v = v * mul;
        return v;
    }

    else if (adcNumber == 3)
    {
        v = analogRead(ADC3);
        v = v * mul;
        return v;
    }
    else if (adcNumber == 4)
    {
        v = analogRead(ADC4);
        v = v * mul;
        return v;
    }
    else if (adcNumber == 5)
    { //battery
        v = analogRead(ADC5);
        v = v * mul;
        return v;
    }
}