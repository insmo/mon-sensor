#ifndef __SLEEP_H
#define __SLEEP_H

#include "arduino/sleep.h"

#define sleep(mode)\
    cli();\
    sleep_mode_##mode();\
    sleep_enable();\
    sleep_bod_disable();\
    sei();\
    sleep_cpu();\
    sleep_disable();
#endif
