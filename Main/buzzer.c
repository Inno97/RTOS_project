/*----------------------------------------------------------------------------
 * CMSIS-RTOS 'main' function template
 *---------------------------------------------------------------------------*/
 
#include "RTE_Components.h"
#include  CMSIS_device_header
#include "cmsis_os2.h"
#define PTE31_Pin 31 //Buzzer pin
#define NOTE_E6 1319
#define NOTE_F6 1397
#define NOTE_G6 1568
#define NOTE_GS6 1661
#define NOTE_A6 1760
#define NOTE_AS6 1865
#define NOTE_B6 1976
#define NOTE_C7 2093
#define NOTE_CS7 2217
#define NOTE_D7 2349
#define NOTE_F7 2794
#define NOTE_C6 1047
#define NOTE_DS6 1245

osMutexId_t buzzer_mutex;

/** Sets the frequency of the pwm signal used to control buzzer */
void setFrequency(int frequency){
		if (frequency != 0) {
				TPM0->MOD = (375000 / frequency) -1;
				TPM0_C4V = 375000 / (1.25*frequency);
		} else {
				TPM0->MOD = 0;
				TPM0_C4V = 0;
		}
		
}

/** Stores the music to be played in a hash table */
//These notes works with 2 resistors in parallel
int music_hash_table(uint8_t note) {
		switch(note) {
				case 0: return 831;//466;
				case 1: return 988;//554;
				case 2: return 1109;//622;
				case 3: return 1245;//698;
				case 4: return 1319;//740;
				case 5: return 1480;//831;
				case 6: return 1661;//932;
				case 7: return 1760;//988;
				case 8: return 1976;//1109;
				case 9: return 932;
				case 10: return 740;
				case 11: return 466;
		}
		return 0; //error
}

/** Stores the music to be played in a hash table 2, note is actually 4 bits only */
int music_hash_table2(uint8_t note) {
		switch(note) {
				case 0: return NOTE_E6;//466;
				case 1: return NOTE_F6;//554;
				case 2: return NOTE_G6;//622;
				case 3: return NOTE_GS6;//698;
				case 4: return NOTE_A6;//740;
				case 5: return NOTE_AS6;//831;
				case 6: return NOTE_B6;//932;
				case 7: return NOTE_C7;//988;
				case 8: return NOTE_CS7;//1109;
				case 9: return NOTE_D7;
				case 10: return NOTE_F7;
				case 11: return NOTE_DS6;
				case 12: return NOTE_C6;			
		}
		return 0; //error
}

/** Returns the first 4 bits of the 8 bit value */
uint8_t front_mask(uint8_t note) {
		return note >> 4;
}

/** Returns the final 4 bits of the 8 bit value */
uint8_t back_mask(uint8_t note) {
		return note & 0x0f;
}

/** Plays the specified note for the specified duration */
void play_note(int note, int duration) {
	setFrequency(note);
	osDelay(duration << 5);
	setFrequency(0);
    	osDelay(duration << 3);
}

/** Plays the main music, hardcoded to optimise memory space, music can be pre-empted between notes */
void play_main_music(uint8_t noteArr[], uint8_t durationArr[]) {
	for (uint8_t i = 0; i < 26; i++) {
		osMutexAcquire(buzzer_mutex, osWaitForever);
		int note = music_hash_table(front_mask(noteArr[i]));
		int duration = front_mask(durationArr[i]) << 2;
		play_note(note, duration);
		
		note = music_hash_table(back_mask(noteArr[i]));
		duration = back_mask(durationArr[i]) << 2;
		play_note(note, duration);
		osMutexRelease(buzzer_mutex);
	}
}

/** Beeps the buzzer, uses 2nd hashtable, hogs the buzzer resource for the entire duration */
void beep(uint8_t noteArr[], uint8_t durationArr[], uint8_t size) {
	osMutexAcquire(buzzer_mutex, osWaitForever);
	for (uint8_t i = 0; i < size; i++) {
		int note = music_hash_table2(front_mask(noteArr[i]));
		int duration = front_mask(durationArr[i]) << 1;
		play_note(note, duration);
		
		note = music_hash_table2(back_mask(noteArr[i]));
		duration = back_mask(durationArr[i]) << 1;
		play_note(note, duration);
	}
	osMutexRelease(buzzer_mutex);
}
 
/** Configures the pin used for PWM to play music on the buzzer */
void initBuzzer(void) {
		SIM_SCGC5 |= SIM_SCGC5_PORTE_MASK;

		PORTE->PCR[PTE31_Pin] &= ~PORT_PCR_MUX_MASK;
		PORTE->PCR[PTE31_Pin] |= PORT_PCR_MUX(3);

		SIM_SCGC6 |= SIM_SCGC6_TPM0_MASK;

		SIM->SOPT2 &= ~SIM_SOPT2_TPMSRC_MASK;
		SIM->SOPT2 |= SIM_SOPT2_TPMSRC(1);

		TPM0->SC &= ~((TPM_SC_CMOD_MASK) | (TPM_SC_PS_MASK));
		TPM0->SC |= (TPM_SC_CMOD(1) | TPM_SC_PS(7));
		TPM0->SC &= ~(TPM_SC_CPWMS_MASK);

		TPM0_C4SC &= ~((TPM_CnSC_ELSB_MASK) | (TPM_CnSC_ELSA_MASK) | (TPM_CnSC_MSB_MASK) | (TPM_CnSC_MSB_MASK));
		TPM0_C4SC |= (TPM_CnSC_ELSB(1) | TPM_CnSC_MSB(1));
		buzzer_mutex = osMutexNew(NULL);
}
