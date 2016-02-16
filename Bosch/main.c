#include "msp430g2553.h"
//version 1.11 (2015/11/24) fix reboot bug

long sample;
volatile    int flag  =   0;
// Function prototypes
void ConfigureAdc(void);
void ConfigureLED(void);

void main(void)
{
  WDTCTL = WDTPW + WDTHOLD; // Stop WDT
  ConfigureAdc();
  ConfigureLED();

	ADC10CTL0 |= ENC + ADC10SC; // Sampling and conversion start
	__bis_SR_register(CPUOFF + GIE); // LPM0, ADC10_ISR will force exit
	sample = ADC10MEM; // get ADC in value
	__delay_cycles(1150000); 	// 1 second delay

   for(;;)
  {
		__delay_cycles(1150000); 	// 1 second delay
	ADC10CTL0 |= ENC + ADC10SC; // Sampling and conversion start
	__bis_SR_register(CPUOFF + GIE); // LPM0, ADC10_ISR will force exit
	sample = ADC10MEM; // get ADC in value



		if (sample > 892 ) {
					P1OUT &= ~BIT0; 			//Power off
					P1OUT |= BIT6;   //Enable heater
					P2OUT |= (BIT1); //Enable RED Led
					P1OUT &= ~BIT1;  //Disable BLUE Led

		}




		if (sample < 892 ) {
			P1OUT |= BIT0; 			//Power on
    		__delay_cycles(3450000); 	// 3 second delay
     		__delay_cycles(3450000); 	// 3 second delay
     		__delay_cycles(3450000); 	// 3 second delay
     		__delay_cycles(3450000); 	// 3 second delay
     		__delay_cycles(1150000); 	// 1 second delay

			if((P1IN & BIT2) == 0  ) {
					P1OUT &= ~BIT6;   		//disable heater
					P2OUT &= ~(BIT1);     	//Disable RED Led
					P1OUT &= ~(BIT1);	  	//Disable Blue Led
					flag = 0;
				}else {

     		while(1)
			{
    			if( flag == 0 && (P1IN & BIT2) == BIT2) {
    				// Reboot
    				P1OUT &= ~BIT0;			//poweroff
    				P1OUT &= ~BIT6;   		//disable heater
    				P2OUT &= ~(BIT1);     	//Disable RED Led
    				P1OUT &= ~(BIT1);	  	//Disable Blue Led
    				__delay_cycles(3450000); 	// 3 second delay
    				P1OUT |= BIT0; 			//Power on

    				break;

    			}else{

    				while(1)
    				{

					//system off
					if((P1IN & BIT2) == 0 ) {
						P1OUT &= ~BIT6;   		//disable heater
						P2OUT &= ~(BIT1);     	//Disable RED Led
						P1OUT &= ~(BIT1);	  	//Disable Blue Led
						flag = 0;
						//break;
						ADC10CTL0 |= ENC + ADC10SC; // Sampling and conversion start
						__bis_SR_register(CPUOFF + GIE); // LPM0, ADC10_ISR will force exit
						sample = ADC10MEM; // get ADC in value
						__delay_cycles(1150000); 	// 1 second delay
						ADC10CTL0 |= ENC + ADC10SC; // Sampling and conversion start
						__bis_SR_register(CPUOFF + GIE); // LPM0, ADC10_ISR will force exit
						sample = ADC10MEM; // get ADC in value
					}

					//system on & too cold reboot
					if((P1IN & BIT2) == BIT2 && sample > 892)
						break;

					//keyboard reset
					if( (P1IN & BIT3) == 0 && (P1IN & BIT2) == BIT2 )
						break;

					//for Power sequence test
					if((P1OUT & BIT0) == 0)
						break;

    				}
				break;
    			}

			}
			 }
		}



  }
}

// ADC10 interrupt service routine
#pragma vector=ADC10_VECTOR
__interrupt void ADC10_ISR(void)
{
  __bic_SR_register_on_exit(CPUOFF); // Clear CPUOFF bit from 0(SR)
}

void ConfigureAdc(void)
{
	//Configure P1.4 to ADC
	ADC10CTL0 = ADC10SHT_2 + ADC10ON + ADC10IE; // ADC10ON, interrupt enabled
	ADC10CTL1 = INCH_5; // input A5
	ADC10AE0 |= 0x20; // PA.5 ADC option select
}

void ConfigureLED(void)
{
	P1DIR |= (BIT0+BIT1+BIT4+BIT6); // P1.0 P1.1 P1.5 P1.6 to GPIO output
	P2DIR |= (BIT1);        // P2.1 to GPIO output
	P1OUT &= ~BIT0; 		// Set P1.0 to low (don't power on)
	P1DIR &= ~(BIT3+BIT2);         // Port 1 P1.3 (push button) as input, 0 is input
	P1REN |=  (BIT3+BIT2);         // Enable Port 1 P1.3 (push button) pull-up resistor


	//set the interrupt registers
	P1IES &= ~BIT3; 		//select low -> high transition
	P1IFG &= ~BIT3;			//clear the flag for P1.3 before enabling the interrupt,
	// to prevent an immediate interrupt
	P1IE |= BIT3;			//enable interrupt for P1.3

	P2DIR &= ~BIT0;
	P1REN |= BIT0;
	//set the interrupt registers
	P2IES |= BIT0; 		//select high -> low transition
	P2IFG &= ~BIT0;			//clear the flag for P2.0 before enabling the interrupt,
	// to prevent an immediate interrupt
	P2IE |= BIT0;			//enable interrupt for P2.0


	P1OUT |= (BIT4);   		// Set the P1.4 to high
	P1OUT &= ~BIT0; 		// Set P1.0 to low (don't power on)

    P1OUT &= ~(BIT6);   	//Disable heater
	P2OUT &= ~(BIT1);     	//Disable RED Led
	P1OUT &= ~(BIT1);	  	//Diable Blue Led


}

//  Port    1   interrupt   service routine
#pragma vector=PORT1_VECTOR
__interrupt void    Port_1(void)
{


	 if((P1IFG & BIT3) == BIT3 )
	 {
	         	P1IFG &= ~BIT3; //clear the interrupt flag
	         	P1OUT &= ~(BIT6);   	//Disable heater
				P2OUT &= ~(BIT1);     	//Disable RED Led
				P1OUT |= (BIT1);	  	//Enable Blue Led
				flag = 1;

	 }



}


//  Port    2   interrupt   service routine
#pragma vector=PORT2_VECTOR
__interrupt void    Port_2(void)
{


	 if((P2IFG & BIT0) == BIT0 )
	 {
      	    P2IFG &= ~BIT0; //clear the interrupt flag
			P1OUT &= ~BIT0; 			//Power off
			P1OUT &= ~BIT6;   		//disable heater
			P2OUT &= ~(BIT1);     	//Disable RED Led
			P1OUT &= ~(BIT1);	  	//Disable Blue Led

	 }



}
