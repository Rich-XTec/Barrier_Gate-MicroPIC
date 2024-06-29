/*
* UFSC- Universidade Federal de Santa Catarina
* Projeto: Cancela, Passagem de nivel
* Autor: Richard Stolk - 21106342
*/

#include <xc.h>                                 // Include Biblioteca Padr�o
#include <stdio.h>                              // Include Biblioteca padr�o C
#include <pic16f877a.h>                         // Include Biblioteca do Chip

#define _XTAL_FREQ 4000000                      

#define ST PORTBbits.RB0
#define SA PORTBbits.RB1
#define SCF PORTBbits.RB2
#define SCA PORTBbits.RB3

#define BUZZ PORTCbits.RC0
#define abrindo PORTCbits.RC4
#define fechando PORTCbits.RC6

#define MCA PORTDbits.RD1
#define MCF PORTDbits.RD3

int flag;                                       // Vari�vel de verifica��o da interrup��o
int aux;                                        // Vari�vel para verifica��o de processo

void __interrupt() TremChegando(void)
{
    if(INTF)
    {
        INTCONbits.INTF = 0;

        if(SA == 0 && SCA == 0)                // Se o sensor de autom�vel estiver ativado e a cancela aberta
        {
            for(int i = 0; i < 5;i++)         // Ativa a sirene por 20 segundos 
            {
                BUZZ = 1;                      
                __delay_ms(4000);
                BUZZ = 0;                       
                __delay_ms(4000);
            }
            
            while(ST != 1)                     // Enquanto o sensor do trem estiver ativado
            {
                BUZZ = 1;                      
                __delay_ms(500);
                BUZZ = 0;                      
                __delay_ms(500);
                
                if(SCF != 0)       		// Se a cancela n�o estiver fechada
                {
                    MCF = 1;                   
                    fechando = 1;               
                }
                else                           
                {
                    MCF = 0;                    
                    fechando = 0;              
                }
            }
            
            flag = 1;                           // Atribui a flag que houve interrup��o
        }
        else                                   
        {
            while(ST != 1)                     // Enquanto o sensor do trem estiver ativado
            {
                BUZZ = 1;                       
                __delay_ms(500);
                BUZZ = 0;                      
                __delay_ms(500);
                
                MCF = 0;                       
                MCA = 0;                       
                abrindo = 0;                   
                fechando = 0;                   
            }
            
            flag = 1;                           // Atribui a flag que houve interrup��o
        }    
    }
    return;
}

void desligaMotores(){
    // Desliga todos os componentes da cancela
    abrindo = 0;
    fechando = 0;
    MCA = 0;
    MCF = 0;
}

void main(void){
   // INICIALIZA��ES
    OPTION_REG = 0b00111111;                    // Resistores Pull-up ativados e INTEDG sens�vel a descida em 0 "bit 6"
    
    //INTCON = 0b10010000;                        // Ativa os bits GIE e INTE
    INTCONbits.GIE = 1;                       
    INTCONbits.INTE = 1;                      
    
    /* Inicializa��o das portas */
    TRISB = 255;                                // Port B como entrada
    TRISC = 0;                                  // Port C como sa�da
    TRISD = 0;                                  // Port D como sa�da
    
    PORTB = 0;                                  // Inicializa Port B com 0V
    PORTC = 0;                                  // Inicializa Port C com 0V
    PORTD = 0;                                  // Inicializa Port D com 0V

    
    while(1){
        flag = 0;                                   // Seta o flag da interrup��o para iniciar
        aux = 0;
        
        desligaMotores();
        BUZZ = 0;
        
        while(aux != 3 && flag == 0){               // Enquanto n�o tiver entrado numa interrup��o e n�o tiver chegado no �ltimo processo
            if(aux == 0 && SA == 0){                // Se est� no primeiro processo e o sensor de autom�vel � ativado               
                MCF = 0;                                    
    		fechando = 0;                               
    		MCA = 1;                                   
    		abrindo = 1;                      
                aux = 1;                            // Ativa o pr�ximo processo
            }
            if(aux == 1 && SCA == 0 & SCF == 1)    // Se a cancela abriu e se o sensor foi ativado
            {
                desligaMotores();                   // Desliga os motores e sinalizadores
                __delay_ms(20000);                  // Aguarda 20 segundos 
                MCF = 1;                            
    		fechando = 1;                       
    		MCA = 0;                            
    		abrindo = 0;                        
                aux = 2;                            
            }
            if(aux == 2 && SCF == 0 && SCA == 1){
                desligaMotores();                   // Desliga tudo
                __delay_ms(1000);                   // Aguarda 1 segundo por seguran�a
                aux = 3;                            // Retorna para o in�cio
            }
        }
    }
}
