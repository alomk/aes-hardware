/* ========================================
 *
 * Copyright YOUR COMPANY, THE YEAR
 * All Rights Reserved
 * UNPUBLISHED, LICENSED SOFTWARE.
 *
 * CONFIDENTIAL AND PROPRIETARY INFORMATION
 * WHICH IS THE PROPERTY OF your company.
 *
 * ========================================
*/
#include <project.h>
#include <string.h>


CY_ISR(RX_INT) {
    uint8 char_buf = UART_ReadRxData();
}

// This function generates true random values using the temperatures recorded by the LED
void GetRandom(uint8* buf, int n ) {
    uint8 ret = 0;
    uint16 prev,curr = 0;
    int inner_counter,outer_counter = 0;
    
    while(outer_counter < n) {
        if (ADC_DelSig_1_IsEndConversion(ADC_DelSig_1_WAIT_FOR_RESULT)) {
            curr = ADC_DelSig_1_GetResult16(); // read the adc
            if (inner_counter != 8) { // we compare the latest value with the previous as our random source
                ret = ret + ((curr > prev)<<inner_counter); // ret holds the running sum for our byte
            } else {
                buf[outer_counter] = ret; // after every 8 iterations, we fill the next byte in our output
                inner_counter = 0;
                ret = 0;
                outer_counter++;
            }
            
            prev = curr;
            inner_counter++;
        }
    }
}

// This function takes a hex string and fills a byte array with the parsed values
void HexToByte(char* hex_string, uint8* buf, int len) {
    int i = 0;
    while ( i < len) {
        sscanf(hex_string+(2*i), "%2hh", &buf[i]);
        i++;
    }
    // assuming all necessary sanitization+padding happens before this function is called
}

// This function takes an array of len bytes and prints out a hex string over UART
void HexPrint(uint8* buf, int len) {
    char ret[2*len]; //each number in buffer will have two hex digits
    int i = 0;
    while (i < len) {
        sprintf(ret+(i*2), "%02X", buf[i]);
        i++;
    }
    UART_PutString(ret);
}


int main()
{
    CyGlobalIntEnable;
    rx_int_StartEx(RX_INT);             // start RX interrupt (look for CY_ISR with RX_INT address)
                                        // for code that writes received bytes to LCD.
    UART_Start();                       // initialize UART
    UART_ClearRxBuffer();
    
    LCD_Char_1_Start();     // initialize lcd  
    LCD_Char_1_ClearDisplay();
    
    ADC_DelSig_1_Start();   // start the ADC_DelSig_1  
    ADC_DelSig_1_StartConvert();   // 

    IDAC8_1_Start();
    
    int delay = 100; // milliseconds
    
    int mode = 0; //0 for key gen, 1 for encryption, 2 for decryption
    
    /* Place your initialization/startup code here (e.g. MyInst_Start()) */

    for(;;)
    {
        /* Place your application code here. */
        if (mode == 0) {
            LCD_Char_1_Position(0, 1);    
            LCD_Char_1_PrintString("GENERATING KEY"); // clean up the display    
            LCD_Char_1_Position(1, 1);
            uint8 key[16];
            GetRandom(key, 16);
            HexPrint(key, 16);
            mode = 1; // go into encryption mode after generating key
            char* test = "AA";
            uint8 test2[4];
            HexToByte(test, test2, 1);
            HexPrint(test2,1);
            
        } else if (mode == 1) {
            LCD_Char_1_ClearDisplay();
            LCD_Char_1_Position(0, 1);    
            LCD_Char_1_PrintString("ENCRYPTING");
        }
        CyDelay(delay);
        
    }
}

/* [] END OF FILE */
