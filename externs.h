#ifndef EXTERNS_H
#define EXTERNS_H
extern I2C i2c;
#if MBED_MAJOR_VERSION == 6
extern UnbufferedSerial pc;
#elif MBED_MAJOR_VERSION == 5
extern Serial pc;
#endif
#endif
