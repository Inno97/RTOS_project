/*----------------------------------------------------------------------------
 * CMSIS-RTOS 'main' function template
 *---------------------------------------------------------------------------*/
 
#include "RTE_Components.h"
#include  CMSIS_device_header
#include "cmsis_os2.h"
#include "gpio.h"  	      	/** Contains code to control LED's */
#include "motor.h" 			/** Contains code to control motors */
#include "buzzer.h" 		/** Contains code to control buzzer */
#include "communication.h"  /** Contains code to initialize UART */

#define BAUD_RATE 9600      /** Baud Rate used for bluetooth communication */

#define FLAG_UART_MSK 0x00000001U
#define FLAG_RUNNING_MSK 0x00000002U
#define FLAG_MOTOR_MSK 0x00000004U
#define FLAG_CONNECT_MSK 0x00000008U
#define FLAG_END_MSK 0x00000010U
#define FLAG_STATIC_MSK 0x00000020U
#define FLAG_ALL 0xFFFFFFFFU


/* Global Variables */
volatile uint8_t rx_data = 0x01;

osEventFlagsId_t evt_id;

osMutexId_t led_mutex;

/* UART2 Interrupt Handler */
void UART2_IRQHandler(void) {
		NVIC_ClearPendingIRQ(UART2_IRQn);
		if (UART2->S1 & UART_S1_RDRF_MASK) {
			rx_data = UART2->D;
			osEventFlagsSet(evt_id, FLAG_UART_MSK);
		}
		if (UART2->S1 & (UART_S1_OR_MASK |
			UART_S1_NF_MASK |
			UART_S1_FE_MASK |
			UART_S1_PF_MASK)) {
			// handle the error
			UART2->S1 |= (UART_S1_OR_MASK | UART_S1_NF_MASK | UART_S1_FE_MASK | UART_S1_PF_MASK); // clear the flag
			rx_data = UART2->D; //discards corrupted data
		}
}

/*----------------------------------------------------------------------------
 * Application threads
 *---------------------------------------------------------------------------*/

/** Thread used to play music on the buzzer */
void tBuzzer(void *argument) {
	uint8_t note[] = {0x09, 0x11, 0x1b, 0x0a, 0xa0, 0x03, 0x02, 0x01, 0x91, 0x11,
		0x21, 0x22, 0x33, 0x00, 0x91, 0x11, 0x90, 0xaa, 0x00, 0x32, 0x19, 0x11, 0x12, 0x12, 0x33, 0x23};
	uint8_t duration[] = {0x11, 0x21, 0x11, 0x11, 0x21, 0x21, 0x11, 0x12,
				0x12, 0x11, 0x11, 0x12, 0x12, 0x11, 0x12, 0x11,
				0x11, 0x12, 0x12, 0x11, 0x11, 0x12, 0x11, 0x12,
				0x11, 0x44};
	uint8_t note2[] = {0x45, 0x61, 0x46, 0x65, 0x45, 0x88, 0x88, 0x43, 
			0x44, 0x44, 0x43, 0x43, 0x43, 0x21, 0x11, 0x22,
			0x22, 0x21, 0x01, 0x01, 0x54, 0x16, 0x66, 0x78,
			0x44, 0x65};
	uint8_t duration2[] = {0x22, 0x22, 0x22, 0x42, 0x22, 0x22, 0x24, 0x22,
				0x22, 0x22, 0x42, 0x22, 0x22, 0x24, 0x22, 0x22,
				0x22, 0x42, 0x22, 0x22, 0x26, 0x22, 0x22, 0x22,
				0x22, 0x2f};
	while (1) {
		play_main_music(note, duration);
		play_main_music(note2, duration2);
	}
}

/** Thread to execute instructions when the chalenge is finished */
void tEndChallenge(void * argument) {
	uint8_t noteArr[] = {0x1f, 0x3f, 0x1f, 0x15, 0x1b, 0x1f,
	0x7f, 0x1f, 0x18, 0x73, 0x17, 0xa1, 0xbf, 0xbc, 0x21};
	uint8_t durationArr[] = {0x44, 0x42, 0x22, 0x24, 0x44, 0x44, 
	0x42, 0x22, 0x24, 0x44, 0x44, 0x42, 0x22, 0x24, 0x48};
	while(1) {
		osEventFlagsWait(evt_id, FLAG_END_MSK, osFlagsWaitAny, osWaitForever);
		beep(noteArr, durationArr, 15);
	}
}

/** Thread to execute actions required being connection via bluetooth */
void tConnect(void *argument) {
	uint8_t noteArr[] = {0x98, 0x77, 0x76, 0x54, 0x44, 0x32,
	0x12, 0x10, 0x12, 0x10};
	uint8_t durationArr[] = {0x66, 0x44, 0x46, 0x66, 0x33, 0x66, 
	0x22, 0x23, 0x36, 0x66};
	while(1) {
		osEventFlagsWait(evt_id, FLAG_CONNECT_MSK, osFlagsWaitAny, osWaitForever);
		osMutexAcquire(led_mutex, osWaitForever);
		blink_twice();
		osMutexRelease(led_mutex);
		beep(noteArr, durationArr, 10);
	}
}

/** Thread to control LED's when the robot is static */
void tStaticLed(void *argument) {
	while(1) {
		osEventFlagsWait(evt_id, FLAG_STATIC_MSK, osFlagsNoClear, osWaitForever);
		osMutexAcquire(led_mutex, osWaitForever);
		static_led_display();
		osMutexRelease(led_mutex);
	}
}

/** Thread to light up green LED's sequentially when the robot is moving */
void tRunningLed(void *argument) { //tRunning priority > tStatic
	while(1) {
		osEventFlagsWait(evt_id, FLAG_RUNNING_MSK, osFlagsNoClear, osWaitForever);
		osMutexAcquire(led_mutex, osWaitForever);
		moving_led_display();
		osMutexRelease(led_mutex);
	}
}

/** Thread to control motor based on user's input */
void tMotor(void *argument) {
	while(1) {
		osEventFlagsWait(evt_id, FLAG_MOTOR_MSK, osFlagsWaitAny, osWaitForever);
		switch(rx_data) {
			case 0x00: stop(); //update flag for led blinking instructions
				osEventFlagsClear(evt_id, FLAG_RUNNING_MSK);
				osEventFlagsSet(evt_id, FLAG_STATIC_MSK);
				break;
			case 0x01: forward();
				osEventFlagsClear(evt_id, FLAG_STATIC_MSK);
				osEventFlagsSet(evt_id, FLAG_RUNNING_MSK);
				break;
			case 0x02: reverse();
				osEventFlagsClear(evt_id, FLAG_STATIC_MSK);
				osEventFlagsSet(evt_id, FLAG_RUNNING_MSK);
				break;
			case 0x03: pivotLeft();
				osEventFlagsClear(evt_id, FLAG_STATIC_MSK);
				osEventFlagsSet(evt_id, FLAG_RUNNING_MSK);
				break;
			case 0x04: pivotRight();
				osEventFlagsClear(evt_id, FLAG_STATIC_MSK);
				osEventFlagsSet(evt_id, FLAG_RUNNING_MSK);
				break;	
			case 0x05: turnLeft();
				osEventFlagsClear(evt_id, FLAG_STATIC_MSK);
				osEventFlagsSet(evt_id, FLAG_RUNNING_MSK);
				break;
			case 0x06: swingLeft();
				osEventFlagsClear(evt_id, FLAG_STATIC_MSK);
				osEventFlagsSet(evt_id, FLAG_RUNNING_MSK);
				break;
			case 0x07: turnRight();
				osEventFlagsClear(evt_id, FLAG_STATIC_MSK);
				osEventFlagsSet(evt_id, FLAG_RUNNING_MSK);
				break;
			case 0x08: swingRight();
				osEventFlagsClear(evt_id, FLAG_STATIC_MSK);
				osEventFlagsSet(evt_id, FLAG_RUNNING_MSK);
				break;
		}
	}
}

/** Main thread which conrols the other threads by settting event flags */
void tBrain(void *argument) {
	while(1) {
		osEventFlagsWait(evt_id, FLAG_UART_MSK, osFlagsWaitAny, osWaitForever);
		switch(rx_data) {
			case 0x00: 
			case 0x01: 
			case 0x02:
			case 0x03:
			case 0x04:
			case 0x05:
			case 0x06:
			case 0x07:
			case 0x08:
				osEventFlagsSet(evt_id, FLAG_MOTOR_MSK);
				break;
			case 0x09: 
				osEventFlagsSet(evt_id, FLAG_END_MSK);
				break;
			case 0x0A:
				osEventFlagsSet(evt_id, FLAG_CONNECT_MSK);
				break;
			}
	}
}

int main (void) {
		// System Initialization
		SystemCoreClockUpdate();
		osKernelInitialize();               // Initialize CMSIS-RTOS
	
		initGPIO();
		initBuzzer();
		initMotor();
		initUART2(BAUD_RATE);
		
		led_mutex = osMutexNew(NULL);
		evt_id = osEventFlagsNew(NULL);
		osEventFlagsClear(evt_id, FLAG_ALL);
		osEventFlagsSet(evt_id, FLAG_STATIC_MSK);
		
		/** Create new threads */
		osThreadNew(tBrain,	NULL, NULL);    
		osThreadNew(tConnect, NULL, NULL);
		osThreadNew(tStaticLed, NULL, NULL);
		osThreadNew(tRunningLed, NULL, NULL);
		osThreadNew(tEndChallenge, NULL, NULL);
		osThreadNew(tBuzzer, NULL, NULL);
		osThreadNew(tMotor, NULL, NULL);
	
		/** Start thread execution */
		osKernelStart();                     
		for (;;) {}
}
