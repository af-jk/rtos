#include "cm4_periphs.h"

int sum(int a, int b) {
    return (a + b);
}

void usage_flt_handler(void) {
    while (1);
}

void bus_flt_handler(void) {
    while (1);
}

void Enable_Bus_Usage_Flts(void) {
    // Enable UsageFault and BusFault exceptions
    SCB->SHCSR |= (1 << 17) | (1 << 18);
}