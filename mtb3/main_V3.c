//  MTB3 firmware 
//  Author M.Maggiali
//  Rev. 2.0.4 del 15/06/2010
//
//  Tested on MTB3 boards (DSPIC30F4011)
//  pic30-gcc V4.03
//  MPLAB IDE ver 8.46.00.00
// 
//
//  Rev. 2.0.5 del 18/06/2010
//
//  I2C at 100KHz
//
//  Rev. 2.0.6 del 23/06/2010
//
//  I2C at 300KHz
//  TIMER_VALUE=0x3700;
//
//  Rev. 2.0.8 del 25/06/2010
//
//  There is a check that if a value is out of the range the value will be discarged 
//  In the status message there is the count of the errors in the I2C reading
//
//  Rev. 2.0.9 del 16/07/2010
//
//  Added the CAN filters 
//  
//
//  Rev 2.0.10 del 28/07/2010
//
//  solved few bugs
//
//  Rev 2.0.11 del 29/07/2010
//  ConfigBuffer[STAGE_CAL_EN]=0x0FFF;
//  MPLAB 8.46
//
//  Rev 2.0.12 del 12/08/2010
//  Check if there is an error in the I2C communication and if the pad is broken.
//	MPLAB 8.46

//  Rev 2.0.13 del 31/08/2010
//  Removed the message with CANID=100 for debug. It can be enabled by uncomment a line below
//  MPLAB 8.56
//

//  Rev 2.0.154 del 31/06/2011
//  NOLOAD=235;
//  MPLAB 8.63
//

//  Rev 2.0.15 del 31/08/2011
//  Removed the bug in the overflow of the CDC value sent trought CAN 
//  the NOLOAD pressure is set to 235 (default). There is a new message for sending parameters like:
//		data[0] message type CAN_TACT_SETUP2 (0x4E)
//		data[1] right SHIFT factor for the sensor readings (12 indipendent measurements)
//		data[2] right SHIFT_THREE  factor for the sensor readings (3 macro areas)     
//		data[3] right SHIFT_ALL  factor for the sensor readings (1 macro area) 
//		data[4] NO_LOAD value (it is set to 235 for default)
//		data[5] ACC_ON ACC_OFF (1 or 0)
//		data[6] NU
//		data[7] NU
 //  MPLAB 8.63
//

//  Rev 2.0.16 del 31/08/2011
//  Added the accelerometer reading  
//  MPLAB 8.76
//
#include<p30f4011.h>
#include"can_interface.h"
#include "AD7147RegMap.h"
#include "ADC.h"
#include "I2C.h"
#include "timers.h"
#include "LED.h"
#include "eeprom.h"
#include "options.h"
#include <timer.h>
#include <libpic30.h>
#include<adc10.h>


//#define DEBUG

unsigned int AN2 = 0; //Accelerometer X axes
unsigned int AN3 = 0; //Accelerometer Y axes
unsigned int AN4 = 0; //Accelerometer Z axes
// inizializzazione bit di configurazione (p30f4013.h)
_FOSC(CSW_FSCM_OFF & EC_PLL8); 
  // Clock switching disabled Fail safe Clock Monitor disabled
  // External clock with PLL x8 (10MHz*8->Fcycle=80/4=20MIPS)
_FWDT(WDT_OFF);      // WD disabled
//
_FBORPOR(MCLR_EN & PWRT_64 & PBOR_ON & BORV27);  // BOR 2.7V POR 64msec
_FGS(CODE_PROT_OFF); // Code protection disabled



// static unsigned int value=0;
//------------------------------------------------------------------------
//								Function prototypes
//------------------------------------------------------------------------
static void ServiceAD7147Isr(unsigned char Channel);
void ServiceAD7147Isr_three(unsigned char Channel);
void ServiceAD7147Isr_all(unsigned char Channel);
void Wait(unsigned int value);
static void FillCanMessages8bit(unsigned char Channel,unsigned char triangleN);
void FillCanMessages8bit_three(unsigned char Channel,unsigned char triangleN);
void FillCanMessages8bit_all(unsigned char Channel,unsigned char triangleN);
void FillCanMessages16bit(unsigned char Channel,unsigned char triangleN);
void FillCanMessages16bit_all(unsigned char Channel,unsigned char triangleN);
void FillCanMessagesTest(unsigned char Channel,unsigned int i); 	
void TrianglesInit(unsigned char Channel);
void TrianglesInit_all(unsigned char Channel);
       
void __attribute__((interrupt, no_auto_psv)) _C1Interrupt(void);  



//------------------------------------------------------------------------
//								Global variables
//------------------------------------------------------------------------
struct s_eeprom _EEDATA(1) ee_data = 
{
  0x0,           // EE_B_EEErased             :1
  0x0,           // EE_B_EnableWD             :1
  0x1,           // EE_B_EnableBOR            :1
  0x05,          // EE_CAN_BoardAddress;      :8
  0x01,          // EE_CAN_MessageDataRate    :8
  0x04,          // EE_CAN_Speed;             :8
  {'T','a','c','t','i','l','e',' ','B','o','a','r','d',' ',' '},
  0x0000 // Checksum
};

// Board Configuration image from EEPROM
struct s_eeprom BoardConfig = {0}; 


volatile char flag;
volatile char flag2;
unsigned int AD7147Registers[16][12];  //Element[23] = 0x17 = ID register @ 0x17
const	unsigned char AD7147_ADD[4]={0x2C,0x2D,0x2E,0x2F};
typedef unsigned const int __prog__ * FlashAddress; //flsh address 
unsigned int __attribute__ ((space(prog), aligned(_FLASH_PAGE*2) ))   CapOffset[16][12]={0,0,0,0,0,0,0,0,0,0,0,
																						 0,0,0,0,0,0,0,0,0,0,0,
																					     0,0,0,0,0,0,0,0,0,0,0,
																					     0,0,0,0,0,0,0,0,0,0,0,
																					     0,0,0,0,0,0,0,0,0,0,0,
																					     0,0,0,0,0,0,0,0,0,0,0,
																					     0,0,0,0,0,0,0,0,0,0,0,
																						 0,0,0,0,0,0,0,0,0,0,0,
																					     0,0,0,0,0,0,0,0,0,0,0,
																					     0,0,0,0,0,0,0,0,0,0,0,
																						 0,0,0,0,0,0,0,0,0,0,0,
																					     0,0,0,0,0,0,0,0,0,0,0,
																					     0,0,0,0,0,0,0,0,0,0,0,
																						 0,0,0,0,0,0,0,0,0,0,0,
																					     0,0,0,0,0,0,0,0,0,0,0,
																					     0,0,0,0,0,0,0,0,0,0,0
																					    };     //Offset of the capacitance 
const FlashAddress _pCapOffset[16]={&CapOffset[0],&CapOffset[1],&CapOffset[2],&CapOffset[3],&CapOffset[4],&CapOffset[5],&CapOffset[6],&CapOffset[7],
							     &CapOffset[8],&CapOffset[9],&CapOffset[10],&CapOffset[11],&CapOffset[12],&CapOffset[13],&CapOffset[14],&CapOffset[15]
							      }; 
unsigned int __attribute__ ((space(prog), aligned(_FLASH_PAGE*2) ))   CapOffset_all[16][3]={      
												                                         0,0,0,
																				         0,0,0,
																					     0,0,0,
																					     0,0,0,
																					     0,0,0,
																					     0,0,0,
																					     0,0,0,
																					     0,0,0,
																					     0,0,0,
																					     0,0,0,
																				         0,0,0,
																					     0,0,0,
																					     0,0,0,
																					     0,0,0,
																					     0,0,0,
																					     0,0,0
																					    };     //Offset of the capacitance 
const FlashAddress _pCapOffset_all[]={&CapOffset_all[0],&CapOffset_all[1],&CapOffset_all[2],&CapOffset_all[3],&CapOffset_all[4],&CapOffset_all[5],&CapOffset_all[6],&CapOffset_all[7],
							     &CapOffset_all[8],&CapOffset_all[9],&CapOffset_all[10],&CapOffset_all[11],&CapOffset_all[12],&CapOffset_all[13],&CapOffset_all[14],&CapOffset_all[15]								
							     }; 	
unsigned int BitToSend;    // number of bit to be send
//unsigned int stagecomplete;  

unsigned int _board_ID=2;
unsigned char board_MODE=EIGHT_BITS;
unsigned char new_board_MODE=EIGHT_BITS;
char _additional_info [32]={'T','a','c','t','i','l','e',' ','S','e','n','s','o','r'};
unsigned int PW_CONTROL= 0x0B0; // 0x1B0 for 128 decim  
unsigned int TIMER_VALUE=TIMER_SINGLE_256dec; // Timer duration 0x3000=> 40ms
unsigned int TIMER_VALUE2=0x133;//0xC00; // Timer duration 0xC00=> 10ms
unsigned int SHIFT=2; //shift of the CDC value for removing the noise
unsigned int SHIFT_THREE=3;// shift of the CDC value for removing the noise
unsigned int SHIFT_ALL=4; //shift of the CDC value for removing the noise
unsigned int NOLOAD=235;
unsigned int ACCELEROMETER=0;
unsigned int CONFIG_TYPE=CONFIG_SINGLE;
unsigned int ERROR_COUNTER=0; //it counts the errors in reading the triangles.
unsigned int ConValue[2]={0x2200, 0x2200}; //offset of the CDC reading 
volatile unsigned int PMsgID; //pressure measurement ID 
unsigned char acc[]={0,0,0,0,0,0,0,0}; //value of the three accelerometers
    unsigned int stagecomplete0[1][1];
    unsigned int stagecomplete1[1][1];
    unsigned int stagecomplete2[1][1];
    unsigned int stagecomplete3[1][1];


//
//------------------------------------------------------------------------------ 
//								External functions
//------------------------------------------------------------------------------

extern void ConfigAD7147(unsigned char Channel, unsigned int i,unsigned int pw_control_val, unsigned int * convalue);
extern void ConfigAD7147_THREE(unsigned char Channel, unsigned int i,unsigned int pw_control_val, unsigned int * convalue);
extern void ConfigAD7147_ALL(unsigned char Channel, unsigned int i,unsigned int pw_control_val, unsigned int * convalue);

//------------------------------------------------------------------------------
// 								Interrrupt routines
//------------------------------------------------------------------------------
void __attribute__((interrupt, no_auto_psv)) _T1Interrupt(void)
{
	unsigned int m=0;
    flag=1;
 	//		led0=~led0;
     ServiceAD7147Isr(CH0);
   	 for (m=0;m<16;m++)
	 {
		FillCanMessages8bit(CH0,m); 	
     }
     _T1IF = 0;

}
void __attribute__((interrupt, no_auto_psv)) _T2Interrupt(void)
{
	    acc[1]=((AN2 &0xFF00) >>0x8); // axis X
        acc[0]=(AN2 & 0xFF);
        acc[3]=((AN3 &0xFF00) >>0x8); // axis Y
        acc[2]=(AN3 & 0xFF);
        acc[5]=((AN4 &0xFF00) >>0x8); // axis Z
        acc[4]=(AN4 & 0xFF);
        while (!CAN1IsTXReady(1));    
        CAN1SendMessage( (CAN_TX_SID(0x550)) & CAN_TX_EID_DIS & CAN_SUB_NOR_TX_REQ,
                        (CAN_TX_EID(0)) & CAN_NOR_TX_REQ, acc, 6,1);
    //executing the sampling of the 
        ADCON1bits.SAMP = 1;        
    _T2IF = 0;

}
void __attribute__((interrupt, no_auto_psv)) _C1Interrupt(void)
{
	//ready to send a CAN message
    if (C1INTFbits.TX0IF || C1INTFbits.TX1IF || C1INTFbits.TX2IF)
    {
        CAN1_interruptTx();
    }
	
	//receive a CAN message
    if (C1INTFbits.RX0IF || C1INTFbits.RX1IF )
    {
        CAN1_interruptRx();   
    }
	IFS1bits.C1IF =0;
}
//------------------------------------------------------------------------
//									MAIN
//------------------------------------------------------------------------
int main(void)
{
    //------------------------------------------------------------------------
    //									Variables   
    //------------------------------------------------------------------------
    char init;
    unsigned char i,l;
    unsigned int counter;
    unsigned int led_counter;
    unsigned int result;
    unsigned int mean; 
   	//
    // EEPROM Data Recovery
    // 
    // Initialize BoardConfig variable in RAM with the Data EEPROM stored values 
    RecoverConfigurationFromEEprom();
    _board_ID=BoardConfig.EE_CAN_BoardAddress;
    //------------------------------------------------------------------------
    //								Peripheral init
    //------------------------------------------------------------------------
    T1_Init(TIMER_VALUE);
   
if (ACCELEROMETER)
{
    T2_Init(TIMER_VALUE2);
    ADC_Init();             //Initialize the A/D converter
}  
    CAN_Init();
    I2C_Init(CH0); 
    LED_Init();
    //Read Silicon versions to check communication, It should read 0xE622

    for (i=0;i<4;i++)
    {
        ReadFromAD7147ViaI2C(CH0,AD7147_ADD[i],DEVID, 1, AD7147Registers[0],AD7147Registers[4],AD7147Registers[8],AD7147Registers[12], DEVID);  
    } 
    
       //............................Configure AD7147
  
  switch (CONFIG_TYPE)
	{
		case CONFIG_SINGLE:
		{
			  for (i=0;i<4;i++)
    			 {
        	    	ConfigAD7147(CH0,i,PW_CONTROL,ConValue); //0 is the number of the device	
    		   	 }
  		       	flag=0;
    			init=0;
  			    WriteTimer1(0);
    			counter=0;
		        while (flag==0);
		        // Calibration
		        ServiceAD7147Isr(CH0);
		        flag=0;
		       	WriteTimer1(0);
                while (flag==0);
		        TrianglesInit(CH0);
		}
		break;	
		case CONFIG_THREE:
		{
			  for (i=0;i<4;i++)
    		    {
        	 		ConfigAD7147_THREE(CH0,i,PW_CONTROL,ConValue); 
  		       	}
  		     	flag=0;
    			init=0;
  		     	WriteTimer1(0);
    			counter=0;
		        while (flag==0); 
		        // Calibration
		        ServiceAD7147Isr_three(CH0);
		        flag=0;
		       	WriteTimer1(0);
                while (flag==0);
		        TrianglesInit_all(CH0);
		}
		break;
		case CONFIG_ALL:
		{
		    for (i=0;i<4;i++)
   		    {
       			ConfigAD7147_ALL(CH0,i,PW_CONTROL,ConValue); 
 		       	}
 		     	flag=0;
   			init=0;
 		    	WriteTimer1(0);
   			counter=0;
	        while (flag==0);
	        // Calibration
	        ServiceAD7147Isr_all(CH0);   
	        flag=0;
	       	WriteTimer1(0);
               while (flag==0);
	        TrianglesInit_all(CH0);
		}
		break;
	}
    l=0;
    i=0;
    counter=0; 
//______________________________________  MAIN LOOP ____________________________________________
//
//
//
		led_counter=0;
	led0=0;
    for (;;)
    {
        if (flag==1)
        {
        	flag=0;
           	i = 0;
			result=0;
			mean=0;
		//	if (led_counter==20)
			{
	//			led0=~led0;
				led_counter=0;
			}	
			led_counter++;
			
#ifdef DEBUG 			
/// ADC conversion
		while(i<15)
        {
			ConvertADC10();		
			i=i+1;
	        while(BusyADC10());
	        result =ReadADC10(0);
            ADCON1bits.SAMP = 1;
			mean=result+mean;
	        for(l=0;l<10;l++);	
		}
 			status[1]=(((mean>>4) &0xFF00) >>0x8); //the value of the analog input
	        status[0]=(mean>>4) & 0xFF;
	        status[3]=((counter &0xFF00) >>0x8); // cycle number
	        status[2]=(counter & 0xFF);
	        status[5]=((ERROR_COUNTER &0xFF00) >>0x8); // ERRORS in reading the I2C
	        status[4]=(ERROR_COUNTER & 0xFF);
	        counter++;
 	        
 		    		
#endif  

 ////////////////////////////////////////////////////////				
            //debug
				// Filling the can messages with the CapSensorData
				switch (board_MODE)
				{
					case  (EIGHT_BITS):
				 	{
					 	switch(CONFIG_TYPE)
					 	{
						 	case CONFIG_SINGLE :
						 	{
							 	// Service routine for the triangles 
	            	/*			 ServiceAD7147Isr(CH0);
							 	 for (i=0;i<16;i++)
	            				 {
		 							FillCanMessages8bit(CH0,i); 	
		      					 }
		      		*/
		      				}
		      				break;
		      			    case CONFIG_THREE:
		      				{
			      				ServiceAD7147Isr_three(CH0);
			      				for (i=0;i<16;i++)					
		            			{
		 						    FillCanMessages8bit_three(CH0,i); 	
			      				}
		      				}
		      				break;
		      			    case CONFIG_ALL:
		      				{
						     	ServiceAD7147Isr_all(CH0);
		      					for (i=0;i<16;i++)					
	            				{
	 								FillCanMessages8bit_all(CH0,i); 	
		      					}
		      				}
		      				break;	
	      					}										
					}
					break;
					case (SIXTEEN_BITS):
					{
						switch (CONFIG_TYPE)
						{
							case CONFIG_SINGLE:
							{
						        ServiceAD7147Isr(CH0);	
							    for (i=0;i<16;i++)					
		            			{
									FillCanMessages16bit(CH0,i);
								}
							}
							break;
					        case CONFIG_THREE:
		      				{
			      				ServiceAD7147Isr_three(CH0);
			      				for (i=0;i<16;i++)					
		            			{
		 						    FillCanMessages16bit_all(CH0,i); 	
			      				}
		      				}
		      				break;
							case CONFIG_ALL:
							{
								ServiceAD7147Isr_all(CH0);
								for (i=0;i<16;i++)
	            				{
									FillCanMessages16bit_all(CH0,i);
								}	
							}
							break;	
							default: break;
						}
					}
					break;		
					case  (CALIB):
					{
						board_MODE=new_board_MODE;
						switch (CONFIG_TYPE)
						{
							case CONFIG_SINGLE:
							{
								  for (i=0;i<4;i++)
					    			 {
					        	    	ConfigAD7147(CH0,i,PW_CONTROL,ConValue); //0 is the number of the device	
					    		   	 }
					  		       	flag=0;
					    			init=0;
					  			    WriteTimer1(0);
					    			counter=0;
							        while (flag==0);
							        // Calibration
							        ServiceAD7147Isr(CH0);
							        flag=0;
							       	WriteTimer1(0);
					                while (flag==0);
							        TrianglesInit(CH0);

							}
							break;	
							case CONFIG_THREE:
							{
								  for (i=0;i<4;i++)
					    		    {
					        	 		ConfigAD7147_THREE(CH0,i,PW_CONTROL,ConValue); 
					  		       	}
					  		     	flag=0;
					    			init=0;
					  		     	WriteTimer1(0);
					    			counter=0;
							        while (flag==0); 
							        // Calibration
							        ServiceAD7147Isr_three(CH0);
							        flag=0;
							       	WriteTimer1(0);
					                while (flag==0);
							        TrianglesInit_all(CH0);
							}
							break;
							case CONFIG_ALL:
							{
							    for (i=0;i<4;i++)
					   		    {
					       			ConfigAD7147_ALL(CH0,i,PW_CONTROL,ConValue); 
					 		       	}
					 		     	flag=0;
					   			init=0;
					 		    	WriteTimer1(0);
					   			counter=0;
						        while (flag==0);
						        // Calibration
						        ServiceAD7147Isr_all(CH0);   
						        flag=0;
						       	WriteTimer1(0);
					               while (flag==0);
						        TrianglesInit_all(CH0);
							}
							break;
						}
					}
					break;	
					default: break;
				} //switch
        }//if (flag==1)
        CAN1_handleRx(_board_ID);
    	  
    }//for(;;)
}//main



static void ServiceAD7147Isr(unsigned char Channel)
{
    unsigned int i=0;
   unsigned int ConfigBuffer[0];
   unsigned int nets=4;
    
    		//Calibration configuration
	ConfigBuffer[0]=0x8000;//0x8000;//0x220;
/*		for (i=0;i<nets;i++)
	    {
		   ReadFromAD7147ViaI2C(CH0,AD7147_ADD[i],STAGE_COMPLETE_LIMIT_INT, 1, stagecomplete0[0],stagecomplete1[0],stagecomplete2[0],stagecomplete3[0], 0);		
		 //   WriteToAD7147ViaI2C(Channel,AD7147_ADD[i],AMB_COMP_CTRL0,1, ConfigBuffer, AMB_COMP_CTRL0);
	    }
*/	 	
	    for (i=0;i<nets;i++)
	    {		
	    // Added 0x0B because of register remapping
	       ReadFromAD7147ViaI2C(CH0,AD7147_ADD[i],(ADCRESULT_S0+0x0B), 2, AD7147Registers[i],AD7147Registers[i+4],AD7147Registers[i+8],AD7147Registers[i+12], ADCRESULT_S0);
	    }	    
	    for (i=0;i<nets;i++)
	    {		
	    // Added 0x0B because of register remapping
	       ReadFromAD7147ViaI2C(CH0,AD7147_ADD[i],(ADCRESULT_S2+0x0B), 10, AD7147Registers[i],AD7147Registers[i+4],AD7147Registers[i+8],AD7147Registers[i+12], ADCRESULT_S2);
	    }
   		
	    for (i=0;i<nets;i++)
	    {
		 //  ReadFromAD7147ViaI2C(CH0,AD7147_ADD[i],STAGE_COMPLETE_LIMIT_INT, 1, stagecomplete0[0],stagecomplete1[0],stagecomplete2[0],stagecomplete3[0], 0);		
		     WriteToAD7147ViaI2C(CH0,AD7147_ADD[i],AMB_COMP_CTRL0,1, ConfigBuffer, 0);
	///DEBUG	      
	  //        ConfigAD7147(CH0,i,PW_CONTROL,ConValue); //0 is the number of the device		 
	    } 
}
void ServiceAD7147Isr_all(unsigned char Channel)
{
    int i=0;
    unsigned int stagecomplete0[1];
    unsigned int stagecomplete1[1];
    unsigned int stagecomplete2[1];
    unsigned int stagecomplete3[1];
    unsigned int ConfigBuffer[12];
    unsigned int ntriangles=4;
    
    //Calibration configuration
	ConfigBuffer[AMB_COMP_CTRL0]=0x8000;//0x220;

	    stagecomplete0[0]=0;
		stagecomplete1[0]=0;
		stagecomplete2[0]=0;
		stagecomplete3[0]=0;	
		//Read ADC Values
//	    for (i=0;i<4;i++)
//	    {
//		    ReadFromAD7147ViaI2C(CH0,AD7147_ADD[i],STAGE_COMPLETE_LIMIT_INT, 1, stagecomplete0[0],stagecomplete1[0],stagecomplete2[0],stagecomplete3[0], 0);		
//	//	    WriteToAD7147ViaI2C(Channel,AD7147_ADD[i],AMB_COMP_CTRL0,1, ConfigBuffer, AMB_COMP_CTRL0);
//	    }
	   	for (i=0;i<ntriangles;i++)
	    {		
	    // Added 0x0B because of register remapping
	       ReadFromAD7147ViaI2C(CH0,AD7147_ADD[i],(ADCRESULT_S0+0x0B), 1, AD7147Registers[i],AD7147Registers[i+4],AD7147Registers[i+8],AD7147Registers[i+12], ADCRESULT_S0);
	    }

}
void ServiceAD7147Isr_three(unsigned char Channel)
{
    int i=0;
    unsigned int ConfigBuffer[12];
     unsigned int ntriangles=4;
    		//Calibration configuration
	ConfigBuffer[AMB_COMP_CTRL0]=0x8000;//0x220;
		//Read ADC Values
	    for (i=0;i<ntriangles;i++)
	    {		
	    // Added 0x0B because of register remapping
	       ReadFromAD7147ViaI2C(CH0,AD7147_ADD[i],(ADCRESULT_S0+0x0B), 3, AD7147Registers[i],AD7147Registers[i+4],AD7147Registers[i+8],AD7147Registers[i+12], ADCRESULT_S0);
	    }
}
//------------------------------------------------------------------------
//					This is just a simple delay routine.
//------------------------------------------------------------------------
void Wait(unsigned int value)    
{
   while (value>0)
        value--;
}//Wait();

void TrianglesInit(unsigned char Channel)
{
    int i,j,k;
	_prog_addressT p;
    int  source[_FLASH_ROW];

		_init_prog_address(p, CapOffset);  /* get address in program space */	
		// delete all the space for the _pCapOffset[32][12]  the ROW is 32 then 12 is the number of ROW to be deleted 		
		for(i=0;i<6;i++)
		{
			_erase_flash(p);
			p += (_FLASH_ROW * 2);	
		} 
		j=0; 
		_init_prog_address(p, CapOffset);  /* get address in program space */		
    	for (i=0;i<16;i++)
    	{
        	for (k=0;k<12;k++)
        	{
				source[j]=AD7147Registers[i][ADCRESULT_S0+k];
				j++;
				if (j==_FLASH_ROW)
				{
					_write_flash16(p,source);
					p += (_FLASH_ROW * 2);	
					j=0;	
				}		
        	
			}
    	}
}
void TrianglesInit_all(unsigned char Channel)
{
    int i,j,k;
	_prog_addressT p;
	 int  source[_FLASH_ROW];

		_init_prog_address(p, CapOffset_all);  /* get address in program space */
		
		// delete all the space for the _pCapOffset[32][12]  the ROW is 32 then 12 is the number of ROW to be deleted 		
		for(i=0;i<3;i++)
		{
			_erase_flash(p);
			p += (_FLASH_ROW * 2);	
		} 
		j=0; 
		_init_prog_address(p, CapOffset_all);  /* get address in program space */
		
    	for (i=0;i<16;i++)
    	{
        	for (k=0;k<3;k++)
        	{
				source[j]=AD7147Registers[i][ADCRESULT_S0+k];
				j++;
				if (j==_FLASH_ROW)
				{
					_write_flash16(p,source);
					p += (_FLASH_ROW * 2);	
					j=0;	
				}		
        	
		}
    	}
    	_write_flash16(p,source);
	p += (_FLASH_ROW * 2);	
	
}

static void FillCanMessages8bit(unsigned char Channel,unsigned char triangleN)
{
    unsigned char data[8];
    unsigned int i,j,error;
    int value; //difference of the current measurement and the initial value (_pCapOffset)
    unsigned int txdata[12];
	int UP_LIMIT, BOT_LIMIT;	
		error=0;
		UP_LIMIT=((MAXVAL-NOLOAD)<<SHIFT);
		BOT_LIMIT=(NOLOAD)<<SHIFT;
	    for (i=0;i<12;i++)
	    {
		    if (((_pCapOffset[triangleN][i]!=0) && ((AD7147Registers[triangleN][ADCRESULT_S0+i]==0))))
		    {
			    error=1;
			    ERROR_COUNTER++;
			}
			value=(AD7147Registers[triangleN][ADCRESULT_S0+i]-_pCapOffset[triangleN][i]);    
			
		    if (value<=-UP_LIMIT) 
		    {
		    	txdata[i]=MAXVAL; // out of range, pressure too low
		    }
		    if (value>=BOT_LIMIT) 
		    {
		    	txdata[i]=MINVAL; // out of range, pressure too high    	
		    }
		    if ((value>-UP_LIMIT) && (value<BOT_LIMIT))
		    {   
		            txdata[i]=NOLOAD-(value>>SHIFT);
		    } 
	    }
	    
	    if (error==1)
	    {
		      for (j=0;j<4;j++)
			  {
			  	ConfigAD7147(CH0,j,PW_CONTROL,ConValue); //0 is the number of the device
			  }	
			  return;
		}
		else 
		{
		    PMsgID=0x300;   
		    PMsgID |= ((triangleN) | BoardConfig.EE_CAN_BoardAddress<<4);
		    //First message	
		    data[0]=0x40;       
			for (i=1;i<8;i++)
			{
			    data[i]    = (unsigned char)   (txdata[i-1] & 0xFF); //the last 6 bits	
		 	}  	
		
		    CAN1_send(PMsgID,1,8,data); 
		    //Second message	
		    data[0]=0xC0;       
		   	for (i=1;i<6;i++)
			{
			    data[i]    = (unsigned char)   (txdata[i+6] & 0xFF); //the last 6 bits	
		 	}
	//	 	#warning "debug"
		 	
	//	 	data[6]=(unsigned char) (ERROR_COUNTER &0xFF);//stagecomplete0[0][0];
		 //	data[7]=stagecomplete3[0][0];
		 //	data[7]= (unsigned char) (ERROR_COUNTER &0xFF);
		
		    CAN1_send(PMsgID,1,6,data);
		}
}
void FillCanMessages8bit_all(unsigned char Channel,unsigned char triangleN)
{
    unsigned char data[8];
    unsigned int i,val;
    unsigned int txdata[12];

			i=0;
	        if (_pCapOffset_all[triangleN][i]>=AD7147Registers[triangleN][ADCRESULT_S0+i])
	        {
	            val=(_pCapOffset_all[triangleN][i]-AD7147Registers[triangleN][ADCRESULT_S0+i])>>SHIFT_ALL;
	            if (val>=10) txdata[i]=255;
	            else
	                txdata[i]=val+244;
	        } else
	        {
	            val=(AD7147Registers[triangleN][ADCRESULT_S0+i]-_pCapOffset_all[triangleN][i])>>SHIFT_ALL;
	            if (val>=243)   txdata[i]=1;
	            else
	                txdata[i]=244-val;
	        }
	    
	    PMsgID=0x300;   
	    PMsgID |= ((triangleN) | BoardConfig.EE_CAN_BoardAddress<<4);
	    //First message	
	    data[0]=0x40;       
        data[1]= (unsigned char)   (txdata[0] & 0xFF); 
	    CAN1_send(PMsgID,1,2,data); 
}
void FillCanMessages8bit_three(unsigned char Channel,unsigned char triangleN)
{
    unsigned char data[8];
    unsigned int i,val;
    unsigned int txdata[12];
 
	    for (i=0;i<3;i++)
	    {
	        if (_pCapOffset_all[triangleN][i]>=AD7147Registers[triangleN][ADCRESULT_S0+i])
	        {
	            val=(_pCapOffset_all[triangleN][i]-AD7147Registers[triangleN][ADCRESULT_S0+i])>>SHIFT_THREE;
	            if (val>=10) txdata[i]=255;
	            else
	                txdata[i]=val+244;
	        } else
	        {
	            val=(AD7147Registers[triangleN][ADCRESULT_S0+i]-_pCapOffset_all[triangleN][i])>>SHIFT_THREE;
	            if (val>=243)   txdata[i]=1;
	            else
	                txdata[i]=244-val;
	        }
	    }
	    PMsgID=0x300;   
	    PMsgID |= ((triangleN) | BoardConfig.EE_CAN_BoardAddress<<4);
	    //First message	
	    data[0]=0x40;       
		for (i=1;i<4;i++)
		{
		    data[i]    = (unsigned char)   (txdata[i-1] & 0xFF); 
	 	}  	
	    CAN1_send(PMsgID,1,4,data); 
}

void FillCanMessages16bit(unsigned char Channel,unsigned char triangleN)

{
	unsigned char data[8];
	unsigned int i,val,l;
	PMsgID=0x300;
    PMsgID |= (triangleN*3+TRIANGLE_OFFSET);
	l=0;
	 for (i=0;i<8;i=i+2)
  	 {
        	val=AD7147Registers[triangleN][ADCRESULT_S0+l];
        	data[i]= val & 0x00FF;  ; //16bit
        	data[i+1]=(val>>8) & 0x00FF; //16bit
        	l=l+1;
    	 } 
	 CAN1_send(PMsgID,1,8,data);	
	 PMsgID=0x300;
	 PMsgID |= (triangleN*3+TRIANGLE_OFFSET)+1;
	 for (i=0;i<8;i=i+2)
  	 {
        	val=AD7147Registers[triangleN][ADCRESULT_S0+l];
        	data[i]= val & 0x00FF;  ; //16bit
        	data[i+1]=(val>>8) & 0x00FF; //16bit
        	l=l+1;
    	 } 
	 CAN1_send(PMsgID,1,8,data);
	  PMsgID=0x300;
	  PMsgID |= (triangleN*3+TRIANGLE_OFFSET)+2;
	  for (i=0;i<8;i=i+2)
  	 {
        	val=AD7147Registers[triangleN][ADCRESULT_S0+l];
        	data[i]= val & 0x00FF;  ; //16bit
        	data[i+1]=(val>>8) & 0x00FF; //16bit
        	l=l+1;
    	 } 
	 CAN1_send(PMsgID,1,8,data);	
	  	
}
void FillCanMessages16bit_all(unsigned char Channel,unsigned char triangleN)
{
	unsigned char data[8];
	unsigned int i,val,l;
	PMsgID=0x300+triangleN; 
	l=0;
	 for (i=0;i<8;i=i+2)
  	 {
        	val=AD7147Registers[triangleN][ADCRESULT_S0+l];
        	data[i]= val & 0x00FF;  ; //16bit
        	data[i+1]=(val>>8) & 0x00FF; //16bit
        	l=l+1;
    	 } 
	 CAN1_send(PMsgID,1,6,data);		  	
}
