/*
 * File:   Main.c
 * Author: Yuri e Richard
 *
 * Created on 21 de Julho de 2022, 12:38
 */
#include <xc.h>                                 // Include Biblioteca Padrao
#include <stdio.h>                              // Include Biblioteca padrao C
#include <pic16f877a.h>                         // Include Biblioteca do Chip

#pragma config WDTE = ON   
#pragma config FOSC = XT    //define uso do clock externo
#pragma config PWRTE = ON   //habilita reset ao ligar
#pragma config BOREN = ON   //Habilita o reset por Brown-out 

#define _XTAL_FREQ 2000000

//#define SensorPro PORTBbits.RB0
//#define BFrente PORTBbits.RB1
//#define BEsquerda PORTBbits.RB2
//#define BDireita PORTBbits.RB3
//#define BTras PORTBbits.RB4
//#define BBuzz PORTBbits.RB5

//#define MotorFrente PORTCbits.RC0
//#define MotorEsquerda PORTCbits.RC1
//#define MotorDireita PORTCbits.RC2
//#define MotorTras PORTCbits.RC3
//#define LedAlert PORTCbits.RC4
//#define Buzzer PORTCbits.RC5

//*** define pinos referentes a interface com LCD
#define RS RD2
#define EN RD3
#define D4 RD4
#define D5 RD5
#define D6 RD6
#define D7 RD7

//**** inclui a biblioteca do LCD escolhido
#include "lcd.h"
//int flag = 0;
float valor = 0 ;
char buffer[3];
int porcentagem=0;
int sec=0 ,min=0, hora=0;

void __interrupt() colisao(void) {

    if (INTF) {
        INTCONbits.INTF = 0;

        while (PORTBbits.RB0 == 0) // Enquanto o sensor de aproximacao estiver ativado
        {
            CLRWDT();
            PORTCbits.RC0 = 0;
            PORTCbits.RC3 = 0;
            PORTCbits.RC2 = 0;
            PORTCbits.RC1 = 0;
            PORTCbits.RC4 = 1;
            PORTCbits.RC5 = 1;
            __delay_ms(500);
            CLRWDT();
            PORTCbits.RC4 = 0;
            PORTCbits.RC5 = 0;
            __delay_ms(500);

        }

        //flag = 1; // Atribui a flag que houve interrupcao
    }

     if(TMR1IF){
           
           //reconfigurar timer
           PIR1bits.TMR1IF = 0;
           TMR1H = 0x0B;
           TMR1L = 0xDC;
           
            sec++;
           //tratar interrup��o
            if(sec == 59){ 
               min++; 
               sec=0;
           }
        }
    
    return;
}

void main(void) {

    //OPTION_REGbits.nRBPU = 0; //PULL UP
    TRISB = 0b11111111;
    OPTION_REG = 0b00111111; //preescaler WDT bit3_ (1:4)
    //Watchdog
//    OPTION_REGbits.PSA = 1;
//    OPTION_REGbits.PS0 = 1; //1:2
//    OPTION_REGbits.PS1 = 0;
//    OPTION_REGbits.PS2 = 0;

    //OPTION_REGbits.INTEDG = 0;
    INTCONbits.GIE = 1; //GLOBAL INTERRUPCAO
    INTCONbits.INTE = 1; //INTERRUPCAO EXTERNA
    INTCONbits.PEIE = 1; //INTERRUPCAO PERIFERICO
    //PIE1bits.ADIE = 1; //CONVERSOR AD
    PIE1bits.TMR1IE = 1;    //Habilita interrupcao timer 1
    
    T1CONbits.TMR1CS = 0;   //timer 1 como temporizador (Fosc/4)
    T1CONbits.T1CKPS0 = 1;  //bit configura pre-escaler(1:8)
    T1CONbits.T1CKPS1 = 1;  //**  
    TMR1H = 0x0B;
    TMR1L = 0xDC;    //carregar valor inicial no contador      
    T1CONbits.TMR1ON = 1;   //Liga o timer
    
    TRISC = 0x00;
    TRISD = 0x00;
    TRISA = 0b11111111;
    //Conversor AD
    ADCON1bits.PCFG0 = 0;
    ADCON1bits.PCFG1 = 1;
    ADCON1bits.PCFG2 = 1;
    ADCON1bits.PCFG3 = 1;

    ADCON0bits.ADCS0 = 0;
    ADCON0bits.ADCS1 = 0;

    ADCON1bits.ADFM = 0;

    ADRESL = 0x00;
    ADRESH = 0x00;
    ADCON0bits.ADON = 1;

    Lcd_Init();
    CLRWDT();
    
    while (1) {
        //Conversor AD

        ADCON0bits.CHS0 = 0;
        ADCON0bits.CHS1 = 0;
        ADCON0bits.CHS2 = 0;

        ADCON0bits.GO = 1;
        __delay_us(10);
        valor = ADRESH;

        //conveter valor em porcentagem

        porcentagem = (valor / 255)*100;
        
        PORTCbits.RC1 = 0;
        PORTCbits.RC0 = 0;
        PORTCbits.RC2 = 0;
        PORTCbits.RC3 = 0;
        PORTCbits.RC4 = 0;
        PORTCbits.RC5 = 0;
        
    while (PORTBbits.RB5 == 0) {
        PORTCbits.RC5 = 1;
        __delay_ms(200);
        //CLRWDT();
    }
    PORTCbits.RC5 = 0;
    
        Lcd_Clear();
        Lcd_Set_Cursor(1,1);
        Lcd_Write_String("BATERIA: ");
        
        sprintf(buffer, "%i", porcentagem);
        Lcd_Set_Cursor(1, 10);
        Lcd_Write_String(buffer);
        
        CLRWDT();
        
        if(min == 59){
            hora++;
            min = 0;
        }
        if(hora == 23){
            min = 0;
            sec = 0;
            hora = 0;
        }
        sprintf(buffer, "%i", hora);
        Lcd_Set_Cursor(2,1);
        Lcd_Write_String(buffer);
        
        Lcd_Set_Cursor(2,3);
        Lcd_Write_String(":");
        
        sprintf(buffer, "%i", min);
        Lcd_Set_Cursor(2,4);
        Lcd_Write_String(buffer);
        
        sprintf(buffer, "%i", sec);
        Lcd_Set_Cursor(2,7);
        Lcd_Write_String(buffer);

        if (PORTBbits.RB1 == 0) {
            PORTCbits.RC0 = 1;
            __delay_ms(200);
            if (PORTBbits.RB2 == 0) {
                PORTCbits.RC1 = 1;
                __delay_ms(200);
                PORTCbits.RC1 = 0;
            } else if (PORTBbits.RB3 == 0) {
                PORTCbits.RC2 = 1;
                __delay_ms(200);
                PORTCbits.RC2 = 0;
            }

        }
        PORTCbits.RC0 = 0;
        PORTCbits.RC3 = 0;

        if (PORTBbits.RB4 == 0) {
            PORTCbits.RC3 = 1;
            __delay_ms(200);
            if (PORTBbits.RB2 == 0) {
                PORTCbits.RC1 = 1;
                __delay_ms(200);
                PORTCbits.RC1 = 0;
            } else if (PORTBbits.RB3 == 0) {
                PORTCbits.RC2 = 1;
                __delay_ms(200);
                PORTCbits.RC2 = 0;
            }

        }
        PORTCbits.RC0 = 0;
        PORTCbits.RC3 = 0;
    }

    return;
}
