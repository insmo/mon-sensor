#include <stdio.h>
#include <stdint.h>
#include <math.h>

#define VERBOSE 0

static uint8_t btov(uint16_t adc) {
    uint8_t v, vres;
    float vbit;

    /* 0.00322mVx10,000 = 32 */
    vbit = 0.00322266;

    /* volt x 16 */
    v = vbit * (adc * 64);
    
    return v;
    /* volt x 1000 f%1.3 */
    vres = v/10; 

    return vres;
}

static uint16_t volt(uint16_t adc) {
    uint8_t vref, mult, scale;
    vref = 52;
    mult = 16;
    scale = 4;
    return ((vref * adc) / mult) * 4;
}

static float error(float expected, float actual) {
    float tmp;
    tmp = expected - actual;
    if (tmp < 0)
        tmp = -tmp;
    return (tmp/actual) * 100.0 ;
}

static float current(float m, float c, uint16_t adc) {
    return m * adc + c;
}

int main(const int argc, char **argv) {
    int i;
    float err;

    int v[3] = {106, 321, 418};
    float v_exp[3] = {1.39, 4.13, 5.36};
    float v_act[3];

    for (i = 0, err=0; i < 3; i++) {
        v_act[i] = (float)volt(v[i])/1000.0;
        printf("%.2f err %.2f \% \n", v_act[i], error(v_exp[i], v_act[i]));
        err += error(v_exp[i], v_act[i]);
    }
    printf("avg err %.2f\% \n", err/3);

    float y1, y2, x1, x2, m, b, a;
    y1 = 7.6;
    y2 = 60.0;
    x1 = 117.0;
    x2 = 220;
    
    m = (y2-y1)/(x2-x1); 
    b = (y1 - m * x1);
    a = 1.39166;

    printf("m: %.10f, b: %.10f\n", m, b);

    int c[3] = {120, 134, 190};
    float c_exp[3] = {7.6, 15.1, 40.6};
    float c_act[3];

    for (i = 0, err=0; i < 3; i++) {
        c_act[i] = current(m, b, c[i]);
        printf("%.2fmAh err %.2f \% \n", c_act[i], error(c_exp[i], c_act[i]));
        err += error(c_exp[i], c_act[i]);
    }
    printf("avg err %.2f\% \n", err/3);
    return 0;
}
