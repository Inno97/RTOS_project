#include "RTE_Components.h"
#include  CMSIS_device_header
#ifndef MOTOR_H_   /* Include guard */
#define MOTOR_H_

void initMotor(void);
void stop(void);
void forward(void);
void reverse(void);
void turnLeft(void);
void turnRight(void);
void swingLeft(void);
void swingRight(void);
void pivotLeft(void);
void pivotRight(void);


#endif
