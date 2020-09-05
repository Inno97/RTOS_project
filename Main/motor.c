/*----------------------------------------------------------------------------
 * CMSIS-RTOS 'main' function template
 *---------------------------------------------------------------------------*/
 
#include "RTE_Components.h"
#include  CMSIS_device_header

#define PTB0_Pin 0
#define PTB1_Pin 1
#define PTB2_Pin 2
#define PTB3_Pin 3
#define UP 0x00
#define DOWN 0xFF
#define LEFT 0xF0
#define RIGHT 0x0F
#define SPEED 200

volatile uint8_t dir = 0x00;

/** Sets up the pins used for pwm to control the motors */ 
void initMotor(void) {
		SIM_SCGC5 |= SIM_SCGC5_PORTB_MASK;
		
		PORTB->PCR[PTB0_Pin] &= ~PORT_PCR_MUX_MASK;
		PORTB->PCR[PTB0_Pin] |= PORT_PCR_MUX(3);
		
		PORTB->PCR[PTB1_Pin] &= ~PORT_PCR_MUX_MASK;
		PORTB->PCR[PTB1_Pin] |= PORT_PCR_MUX(3);	
		
		PORTB->PCR[PTB2_Pin] &= ~PORT_PCR_MUX_MASK;
		PORTB->PCR[PTB2_Pin] |= PORT_PCR_MUX(3);
		
		PORTB->PCR[PTB3_Pin] &= ~PORT_PCR_MUX_MASK;
		PORTB->PCR[PTB3_Pin] |= PORT_PCR_MUX(3);
	
		SIM_SCGC6 |= SIM_SCGC6_TPM1_MASK | SIM_SCGC6_TPM2_MASK;

		SIM->SOPT2 &= ~SIM_SOPT2_TPMSRC_MASK;
		SIM->SOPT2 |= SIM_SOPT2_TPMSRC(1);

		TPM1->MOD = 7499;

		TPM1->SC &= ~((TPM_SC_CMOD_MASK) | (TPM_SC_PS_MASK));
		TPM1->SC |= (TPM_SC_CMOD(1) | TPM_SC_PS(7));
		TPM1->SC &= ~(TPM_SC_CPWMS_MASK);

		TPM1_C0SC &= ~((TPM_CnSC_ELSB_MASK) | (TPM_CnSC_ELSA_MASK) | (TPM_CnSC_MSB_MASK) | (TPM_CnSC_MSB_MASK));
		TPM1_C0SC |= (TPM_CnSC_ELSB(1) | TPM_CnSC_MSB(1));
		TPM1_C1SC &= ~((TPM_CnSC_ELSB_MASK) | (TPM_CnSC_ELSA_MASK) | (TPM_CnSC_MSB_MASK) | (TPM_CnSC_MSB_MASK));
		TPM1_C1SC |= (TPM_CnSC_ELSB(1) | TPM_CnSC_MSB(1));
		
		TPM2->MOD = 7499;
		
		TPM2->SC &= ~((TPM_SC_CMOD_MASK) | (TPM_SC_PS_MASK));
		TPM2->SC |= (TPM_SC_CMOD(1) | TPM_SC_PS(7));
		TPM2->SC &= ~(TPM_SC_CPWMS_MASK);

		TPM2_C0SC &= ~((TPM_CnSC_ELSB_MASK) | (TPM_CnSC_ELSA_MASK) | (TPM_CnSC_MSB_MASK) | (TPM_CnSC_MSB_MASK));
		TPM2_C0SC |= (TPM_CnSC_ELSB(1) | TPM_CnSC_MSB(1));
		TPM2_C1SC &= ~((TPM_CnSC_ELSB_MASK) | (TPM_CnSC_ELSA_MASK) | (TPM_CnSC_MSB_MASK) | (TPM_CnSC_MSB_MASK));
		TPM2_C1SC |= (TPM_CnSC_ELSB(1) | TPM_CnSC_MSB(1));
}

/* Motor Base Functions */
/** Changes the speed of the motor used to rotate the left wheels */
void setLeftSpeed(uint8_t percentage) {
		if (dir & 0xF0) {
				TPM2_C0V = 0;
				TPM2_C1V = (int) (percentage * 7499.0 / 255);
		} else {
				TPM2_C0V = (int) (percentage * 7499.0 / 255);
				TPM2_C1V = 0;
		}
}

/** Changes the speed of the motor used to rotate the right wheels */
void setRightSpeed(uint8_t percentage) {
		if (dir & 0x0F) {
				TPM1_C0V = 0;
				TPM1_C1V = (int) (percentage * 7499.0 / 255);
		} else {
				TPM1_C0V = (int) (percentage * 7499.0 / 255);
				TPM1_C1V = 0;
		}
}

// stop all PWM value
/** All pwm pins are set to output a logic low signal */
void stop(void) {	
		TPM1_C0V = 0;
		TPM1_C1V = 0;
		TPM2_C0V = 0;
		TPM2_C1V = 0;
}

/* Advanced Motor Functions */
// basic forward / backward
/** Moves the robot forward */
void forward(void) {
		dir = UP;
		setLeftSpeed(SPEED);
		setRightSpeed(SPEED);
}

/** Moves the robot backward */
void reverse(void) {
		dir = DOWN;
		setLeftSpeed(SPEED);
		setRightSpeed(SPEED);
}

// keep both wheels spinning, the other at 25% power
void turnRight(void) {
		dir = DOWN;
		setLeftSpeed(55);
		setRightSpeed(250);
}

void turnLeft(void) {
		dir = DOWN;
		setLeftSpeed(250);
		setRightSpeed(55);
}

// keep the other wheel stationary
void swingRight(void) {
		dir = DOWN;
		setLeftSpeed(SPEED >> 3);
		setRightSpeed(SPEED);
}

void swingLeft(void) {
		dir = DOWN;
		setLeftSpeed(SPEED);
		setRightSpeed(SPEED >> 3);
}

// pivot on the spot, both wheels in opposite directions
/** Turns the robot towards it left */
void pivotLeft(void) {
		dir = LEFT;
		setLeftSpeed(125);
		setRightSpeed(125);
}

/** Turns the robot towards it right */
void pivotRight(void) {
		dir = RIGHT;
		setLeftSpeed(125);
		setRightSpeed(125);	
}
