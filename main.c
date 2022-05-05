/*
 * File:   main.c
 * Author: yuki
 *
 * Created on May 3, 2022, 6:14 AM
 */

// PIC16F1823 Configuration Bit Settings

// 'C' source line config statements

// CONFIG1
#pragma config FOSC = INTOSC // Oscillator Selection (INTOSC oscillator: I/O
                             // function on CLKIN pin)
#pragma config WDTE = ON     // Watchdog Timer Enable (WDT disabled)
#pragma config PWRTE = ON    // Power-up Timer Enable (PWRT enabled)
#pragma config MCLRE =                                                         \
    OFF // MCLR Pin Function Select (MCLR/VPP pin function is digital input)
#pragma config CP = OFF  // Flash Program Memory Code Protection (Program memory
                         // code protection is disabled)
#pragma config CPD = OFF // Data Memory Code Protection (Data memory code
                         // protection is disabled)
#pragma config BOREN = OFF // Brown-out Reset Enable (Brown-out Reset disabled)
#pragma config CLKOUTEN = OFF // Clock Out Enable (CLKOUT function is disabled.
                              // I/O or oscillator function on the CLKOUT pin)
#pragma config IESO = OFF     // Internal/External Switchover (Internal/External
                              // Switchover mode is disabled)
#pragma config FCMEN = OFF    // Fail-Safe Clock Monitor Enable (Fail-Safe Clock
                              // Monitor is disabled)

// CONFIG2
#pragma config WRT =                                                           \
    OFF // Flash Memory Self-Write Protection (Write protection off)
#pragma config PLLEN = ON  // PLL Enable (4x PLL enabled)
#pragma config STVREN = ON // Stack Overflow/Underflow Reset Enable (Stack
                           // Overflow or Underflow will cause a Reset)
#pragma config BORV = LO   // Brown-out Reset Voltage Selection (Brown-out Reset
                           // Voltage (Vbor), low trip point selected.)
#pragma config LVP = OFF   // Low-Voltage Programming Enable (High-voltage on
                           // MCLR/VPP must be used for programming)

// #pragma config statements should precede project file includes.
// Use project enums instead of #define for ON and OFF.

#include <stdbool.h>
#include <xc.h>
#define _XTAL_FREQ 1000000

// #pragma config statements should precede project file includes.
// Use project enums instead of #define for ON and OFF.
/*
 * RA0: Serial
 * RA1: Serial
 *
 * RC1:  Boot Switch. HIGH(ON), LOW(OFF)
 * RC2: PS_HOLD
 * RC3 POWER_ON
 * RC4 POWER_OFF
 * RC5: VBUS_POWER
 *
 * POWER_ONをPS_HOLDがHIGHになるまでHIGHを保持
 * PS_HOLDがHIGHになればPOWER_ONをLOWにしてよい
 *
 * Software主導(UART経由てきな)
 * 1.POWER_ONをLOW
 * 2.パワーダウンコマンドを発行
 * 3. PS_HOLDがLOWになるのを待つ(最大30Secぐらい？)
 * 4. 電源を切ってよし
 *
 * ハードウェア主導
 * 1. POWER_ONをLOW
 * 2. POWER_OFF_NをLOW(PS_HOLDがLOWになるまで)
 * 3. PS_HOLDがLOWになるのを待つ(3Sec以上)
 * 4. Power OFF
 *
 *
 *              POWER_OFF_N
 *                  |
 *              ||--|
 *  POWER_OFF --||<-
 *              ||-|
 *                 |
 *                GND
 */
// input
#define VBUS_POWER LATC5
// POWER_OFF 1 is LOW . 0 is HIGH
#define POWER_OFF LATC4
#define POWER_ON LATC3
// input
#define PS_HOLD RC2
// input
#define BOOT_SWITCH RC1

void powerOn(bool *start_up);
void powerOffHard(bool *start_up);

void main(void) {

  // clock setting
  OSCCON = 0b01001000;
  /*
   * RC5: in
   * RC4: out
   * RC3: out
   * RC2: in
   * RC1: in
   */
  TRISC = 0b00100110;
  ANSELC = 0b00000000;

  __delay_ms(500);
  // 1のとき起動中
  volatile bool start_up = 0;
  /*
   * VBUS_POWER
   * に電圧が来ており、start_upがfalseかつBOOT_SWITCHがtrueなら起動プロセス
   */
  while (1) {

    if (!VBUS_POWER) {
      powerOffHard(&start_up);
    } else {
      if (BOOT_SWITCH && !start_up && !PS_HOLD) {
        powerOn(&start_up);
      }
    }
  }
}

void powerOn(bool *start_up) {
  /*
   * POWER_ONをPS_HOLDがHIGHになるまでHIGHを保持
   * PS_HOLDがHIGHになればPOWER_ONをLOWにしてよい
   * Power ON
   */
  POWER_ON = 1;
  POWER_OFF = 0;
  *start_up = 1;
  while (!PS_HOLD) {
    __delay_ms(50);
  }
  return;
}

void powerOffHard(bool *start_up) {
  /*
   * ハードウェア主導
   * 1. POWER_ONをLOW
   * 2. POWER_OFF_NをLOW(PS_HOLDがLOWになるまで)
   * 3. PS_HOLDがLOWになるのを待つ(3Sec以上)
   * 4. Power OFF
   */
  POWER_ON = 0;
  POWER_OFF = 1;
  *start_up = 0;
  while (PS_HOLD) {
    __delay_ms(5);
  }
  return;
}
