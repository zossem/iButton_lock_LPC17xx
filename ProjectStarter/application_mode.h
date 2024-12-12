#ifndef _MODE_
#define _MODE_

#define CONFIG_MODE 0
#define WORK_MODE 1

#define MODE_PIN 13 //P2_13

void mode_Initialize(void);
int get_mode(void);

#endif