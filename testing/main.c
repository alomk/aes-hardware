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

#define BLOCK_SIZE 16 // in bytes
#define KEY_SIZE 32 // in bytes
#define ROUNDS 14

#define ROTL32(x,shift) ((uint32) ((x) >> (shift)) | ((x) << (32 - (shift))))
#define ROTR32(x,shift) ((uint32) ((x) << (shift)) | ((x) >> (32 - (shift))))
// shifts are opposite due to endianness

typedef uint8 u8;
typedef uint16 u16;
typedef uint32 u32;

// static lookup tables for aes S-box and inverse
u8 sbox[256] = {0x63, 0x7c, 0x77, 0x7b, 0xf2, 0x6b, 0x6f, 0xc5, 0x30, 0x1, 0x67, 0x2b, 0xfe, 0xd7, 0xab, 0x76, 0xca, 0x82, 0xc9, 0x7d, 0xfa, 0x59, 0x47, 0xf0, 0xad, 0xd4, 0xa2, 0xaf, 0x9c, 0xa4, 0x72, 0xc0, 0xb7, 0xfd, 0x93, 0x26, 0x36, 0x3f, 0xf7, 0xcc, 0x34, 0xa5, 0xe5, 0xf1, 0x71, 0xd8, 0x31, 0x15, 0x4, 0xc7, 0x23, 0xc3, 0x18, 0x96, 0x5, 0x9a, 0x7, 0x12, 0x80, 0xe2, 0xeb, 0x27, 0xb2, 0x75, 0x9, 0x83, 0x2c, 0x1a, 0x1b, 0x6e, 0x5a, 0xa0, 0x52, 0x3b, 0xd6, 0xb3, 0x29, 0xe3, 0x2f, 0x84, 0x53, 0xd1, 0x0, 0xed, 0x20, 0xfc, 0xb1, 0x5b, 0x6a, 0xcb, 0xbe, 0x39, 0x4a, 0x4c, 0x58, 0xcf, 0xd0, 0xef, 0xaa, 0xfb, 0x43, 0x4d, 0x33, 0x85, 0x45, 0xf9, 0x2, 0x7f, 0x50, 0x3c, 0x9f, 0xa8, 0x51, 0xa3, 0x40, 0x8f, 0x92, 0x9d, 0x38, 0xf5, 0xbc, 0xb6, 0xda, 0x21, 0x10, 0xff, 0xf3, 0xd2, 0xcd, 0xc, 0x13, 0xec, 0x5f, 0x97, 0x44, 0x17, 0xc4, 0xa7, 0x7e, 0x3d, 0x64, 0x5d, 0x19, 0x73, 0x60, 0x81, 0x4f, 0xdc, 0x22, 0x2a, 0x90, 0x88, 0x46, 0xee, 0xb8, 0x14, 0xde, 0x5e, 0xb, 0xdb, 0xe0, 0x32, 0x3a, 0xa, 0x49, 0x6, 0x24, 0x5c, 0xc2, 0xd3, 0xac, 0x62, 0x91, 0x95, 0xe4, 0x79, 0xe7, 0xc8, 0x37, 0x6d, 0x8d, 0xd5, 0x4e, 0xa9, 0x6c, 0x56, 0xf4, 0xea, 0x65, 0x7a, 0xae, 0x8, 0xba, 0x78, 0x25, 0x2e, 0x1c, 0xa6, 0xb4, 0xc6, 0xe8, 0xdd, 0x74, 0x1f, 0x4b, 0xbd, 0x8b, 0x8a, 0x70, 0x3e, 0xb5, 0x66, 0x48, 0x3, 0xf6, 0xe, 0x61, 0x35, 0x57, 0xb9, 0x86, 0xc1, 0x1d, 0x9e, 0xe1, 0xf8, 0x98, 0x11, 0x69, 0xd9, 0x8e, 0x94, 0x9b, 0x1e, 0x87, 0xe9, 0xce, 0x55, 0x28, 0xdf, 0x8c, 0xa1, 0x89, 0xd, 0xbf, 0xe6, 0x42, 0x68, 0x41, 0x99, 0x2d, 0xf, 0xb0, 0x54, 0xbb, 0x16};
u8 inverse_sbox[256] = {0x52, 0x9, 0x6a, 0xd5, 0x30, 0x36, 0xa5, 0x38, 0xbf, 0x40, 0xa3, 0x9e, 0x81, 0xf3, 0xd7, 0xfb, 0x7c, 0xe3, 0x39, 0x82, 0x9b, 0x2f, 0xff, 0x87, 0x34, 0x8e, 0x43, 0x44, 0xc4, 0xde, 0xe9, 0xcb, 0x54, 0x7b, 0x94, 0x32, 0xa6, 0xc2, 0x23, 0x3d, 0xee, 0x4c, 0x95, 0xb, 0x42, 0xfa, 0xc3, 0x4e, 0x8, 0x2e, 0xa1, 0x66, 0x28, 0xd9, 0x24, 0xb2, 0x76, 0x5b, 0xa2, 0x49, 0x6d, 0x8b, 0xd1, 0x25, 0x72, 0xf8, 0xf6, 0x64, 0x86, 0x68, 0x98, 0x16, 0xd4, 0xa4, 0x5c, 0xcc, 0x5d, 0x65, 0xb6, 0x92, 0x6c, 0x70, 0x48, 0x50, 0xfd, 0xed, 0xb9, 0xda, 0x5e, 0x15, 0x46, 0x57, 0xa7, 0x8d, 0x9d, 0x84, 0x90, 0xd8, 0xab, 0x0, 0x8c, 0xbc, 0xd3, 0xa, 0xf7, 0xe4, 0x58, 0x5, 0xb8, 0xb3, 0x45, 0x6, 0xd0, 0x2c, 0x1e, 0x8f, 0xca, 0x3f, 0xf, 0x2, 0xc1, 0xaf, 0xbd, 0x3, 0x1, 0x13, 0x8a, 0x6b, 0x3a, 0x91, 0x11, 0x41, 0x4f, 0x67, 0xdc, 0xea, 0x97, 0xf2, 0xcf, 0xce, 0xf0, 0xb4, 0xe6, 0x73, 0x96, 0xac, 0x74, 0x22, 0xe7, 0xad, 0x35, 0x85, 0xe2, 0xf9, 0x37, 0xe8, 0x1c, 0x75, 0xdf, 0x6e, 0x47, 0xf1, 0x1a, 0x71, 0x1d, 0x29, 0xc5, 0x89, 0x6f, 0xb7, 0x62, 0xe, 0xaa, 0x18, 0xbe, 0x1b, 0xfc, 0x56, 0x3e, 0x4b, 0xc6, 0xd2, 0x79, 0x20, 0x9a, 0xdb, 0xc0, 0xfe, 0x78, 0xcd, 0x5a, 0xf4, 0x1f, 0xdd, 0xa8, 0x33, 0x88, 0x7, 0xc7, 0x31, 0xb1, 0x12, 0x10, 0x59, 0x27, 0x80, 0xec, 0x5f, 0x60, 0x51, 0x7f, 0xa9, 0x19, 0xb5, 0x4a, 0xd, 0x2d, 0xe5, 0x7a, 0x9f, 0x93, 0xc9, 0x9c, 0xef, 0xa0, 0xe0, 0x3b, 0x4d, 0xae, 0x2a, 0xf5, 0xb0, 0xc8, 0xeb, 0xbb, 0x3c, 0x83, 0x53, 0x99, 0x61, 0x17, 0x2b, 0x4, 0x7e, 0xba, 0x77, 0xd6, 0x26, 0xe1, 0x69, 0x14, 0x63, 0x55, 0x21, 0xc, 0x7d}; 

typedef enum {false, true } bool;

enum Mode {Generation, Encryption, Decryption};

char rec_buf[BLOCK_SIZE*2];  // this stores the user-inputted string
u8 byte_buf[BLOCK_SIZE]; // this is the result of converting rec_buf to bytes
int rec_pointer = -1;   // this is our current index into the string
bool buf_full = false;

//u8 key[KEY_SIZE];

int mode = Generation; //0 for key gen, 1 for encryption, 2 for decryption

CY_ISR(RX_INT) {
    if (rec_pointer < BLOCK_SIZE*2 - 1) { // need to multiply by 2 because we are reading a hex string  
        rec_pointer++;
        rec_buf[rec_pointer] = UART_ReadRxData(); // filling up rec_buf with our input
        buf_full = rec_pointer == BLOCK_SIZE*2 - 1;
    }
}

// this function does multiplication of two numbers in the galois field used in aes
u8 gmul(u8 a, u8 b) {
        u8 p = 0;
        u8 counter;
        u8 hi_bit_set;
        for (counter = 0; counter < 8; counter++) {
                if (b & 1) 
                        p ^= a;
                hi_bit_set = (a & 0x80);
                a <<= 1;
                if (hi_bit_set) 
                        a ^= 0x1b; /* x^8 + x^4 + x^3 + x + 1 */
                b >>= 1;
        }
        return p;
}

// this function applys a column mixing matrix to a row of bytes
void mix_column(u8 *r, bool invert) {
    u8 copy[4];
    memcpy(copy, r, 4);

    if (invert) {
        r[0] = gmul(copy[0], 0xe) ^ gmul(copy[1],0xb) ^ gmul(copy[2],0xd) ^ gmul(copy[3], 0x9);
        r[1] = gmul(copy[0], 0x9) ^ gmul(copy[1],0xe) ^ gmul(copy[2],0xb) ^ gmul(copy[3], 0xd);
        r[2] = gmul(copy[0], 0xd) ^ gmul(copy[1],0x9) ^ gmul(copy[2],0xe) ^ gmul(copy[3], 0xb);
        r[3] = gmul(copy[0], 0xb) ^ gmul(copy[1],0xd) ^ gmul(copy[2],0x9) ^ gmul(copy[3], 0xe);
    } else {
        r[0] = gmul(copy[0], 0x02) ^ gmul(copy[1],0x03) ^ copy[2] ^ copy[3];
        r[1] = copy[0] ^ gmul(copy[1],0x02) ^ gmul(copy[2],0x03) ^ copy[3];
        r[2] = copy[0] ^ copy[1] ^ gmul(copy[2],2) ^ gmul(copy[3],3);
        r[3] = gmul(copy[0], 0x03) ^ copy[1] ^ copy[2] ^ gmul(copy[3], 0x02);
    }
}

// this function substitutes 4 bytes of w into dst using the sbox
void sub_word(u32 w, u32* dst) {
    u8* tmp = (u8*) dst;
    u8* tmp2 = (u8*) &w;
    tmp[0] = sbox[tmp2[0]];
    tmp[1] = sbox[tmp2[1]];
    tmp[2] = sbox[tmp2[2]];
    tmp[3] = sbox[tmp2[3]];
}

// this function takes a 256 bit key and produces 15 128-bit keys
void KeySchedule(u8* round_keys, u8* key) {
   u8 rcon[11] = {0x00, 0x01, 0x02, 0x04, 0x08, 0x10, 0x20, 0x40, 0x80, 0x1B, 0x36};
   u8 rcon_buf[4] = {0x00, 0x00, 0x00, 0x00};

   // states needed to calculate entire key schedule
   u32 w_prev = 0; // previous word
   u32 w_in = 0; // word at index i-n
   u32 w = 0; // current word
   u32 w_buf = 0; // word buffer for performing operations on
   
   int n = 8; // key size in 32-bit words

   int counter = 0;
   while(counter < 60) { //4*ROUNDS
       if (counter < n) {
           w = *((u32*) (key + counter*4));
       } else if (counter % n == 0) {
           rcon_buf[0] = rcon[counter/n];
           sub_word(ROTL32(w_prev, 8), &w_buf);
           w = w_in ^ w_buf ^ *((u32*) rcon_buf);
       } else if (n > 6 && counter % n == 4) {
           sub_word(w_prev, &w_buf);
           w = w_in ^ w_buf;
       } else {
           w = w_in ^ w_prev;
       }

       memcpy(round_keys + counter*4, &w, 4);
    
       // updating state variables 
       w_prev = w;
       counter++;
       
       if (counter - n >= 0) {
           memcpy((u8*) &w_in, 4*(counter - n) + round_keys, 4);
       }
   }

}

// this function takes a 128-bit block and performs right or left cyclic shifts as in the AES specification
void ShiftRows(u8* block, bool invert) {
    u8 row[4];
    u32* block_handle = (u32*) row;
    
    int i = 1; //row 1 is not changed
    while(i < 4) {
        // due to an aes block being numbered column wise, a "row" is actually a column
       row[0] = block[i]; 
       row[1] = block[4+i];
       row[2] = block[8+i];
       row[3] = block[12+i];
    
       if (invert) {
          block_handle[0] = ROTR32(block_handle[0], 8*i); // rotate by 1, 2, or 3 bytes
       } else {
          block_handle[0] = ROTL32(block_handle[0], 8*i); 
       }
    
       // writing our shifted rows back into the block
       block[i] = row[0];
       block[4+i] = row[1];
       block[8+i] = row[2];
       block[12+i] = row[3];
        
       i++;
    }
}

// this function calls mix_column on each column of the block
void MixColumns(u8* block, bool invert) {
    mix_column(block, invert); // due to aes block counting column wise, we actually take the row which is easier
    mix_column(block + 4, invert);
    mix_column(block + 8, invert);
    mix_column(block + 12, invert);
}

// this function XORs 16 bytes of p with 16 bytes of key
void AddKey(u8* p, u8* key) {
    int i = 0;
    while (i < 16) {
        p[i] = p[i] ^ key[i];
        i++;
    }
}

// this function pads a byte array with 0's up to len starting at idx
void PadChunk(u8* bytes, int idx, int len) {
    int i = idx;
    while (i < len) {
        bytes[i] = 0x00;
        i++;
    }
}

// this function encrypts len number of bytes
void Encrypt(u8* block, int len, u8* round_keys){
    
    // adding first round key
   AddKey(block, round_keys);

   int j = 0;
   int i = 1;
   while (i < ROUNDS) {
       while (j < 16) { // 16 bytes in one chunk
           block[j] = sbox[block[j]]; // substitution step 
           j++;
       }

       ShiftRows(block, 0); // shift rows step
       MixColumns(block, 0); // mix columns step
       AddKey(block, round_keys + i*16); // add key step

       j = 0;
       i++;
   }

   // final round
   while (j < 16) {
       block[j] = sbox[block[j]]; // substitution step 
       j++;
   }

   ShiftRows(block, 0);
   AddKey(block, round_keys + 14*16); // add final key
}

void Decrypt(u8* block, int len, u8* round_keys){ 
   int j = 0;
   int i = ROUNDS - 1;

   // adding first round key
   AddKey(block, round_keys + ROUNDS*16);
   ShiftRows(block, 1);

   while (j < 16) {
       block[j] = inverse_sbox[block[j]];
       j++;
   }

   j = 0;
   while (i > 0) {
       AddKey(block, round_keys + i*16); // add key step

       MixColumns(block, 1); // inverse mix columns step
       ShiftRows(block, 1); // inverse shift rows step
       
       j = 0;
       while (j < 16) {
           block[j] = inverse_sbox[block[j]]; // inverse substitution step 
           j++;
       }

       i--;
   }

   AddKey(block, round_keys); // add first round key 

}

// This function generates true random values using the temperatures recorded by the LED
void GetRandom(u8* buf, int n ) {
    u8 ret = 0;
    u16 prev,curr = 0;
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

int CharToInt(char input)
{
  if(input >= '0' && input <= '9')
    return input - '0';
  if(input >= 'A' && input <= 'F')
    return input - 'A' + 10;
  if(input >= 'a' && input <= 'f')
    return input - 'a' + 10;

  return -1; // error case
}

// This function takes an even length hex string and fills a byte array with the parsed values
void HexToByte(char* hex_string, u8* buf, int len) {
  int i = 0;
  while(i < len*2){
    buf[i/2] = CharToInt(hex_string[i])*16 + CharToInt(hex_string[i + 1]);
    i+=2;
  }

  // assuming all necessary sanitization+padding happens before this function is called
}


// This function takes an array of len bytes and prints out a hex string over UART
void HexPrint(u8* buf, int len) {
    u8 ret[2*len + 1]; //each number in buffer will have two hex digits
    int i = 0;
    while (i < len) {
        sprintf(ret+(i*2), "%02hX", buf[i]);
        i++;
    }
    ret[2*len] = 0x00;
    UART_PutString("\n\r"); // crlf
    UART_PutString((char*)ret);
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
    
    u8 key[KEY_SIZE]; // 256 bit key
    u8 round_keys[BLOCK_SIZE * (ROUNDS + 1)]; // 14 rounds + final key
  
    int delay = 100; // milliseconds
    
    for(;;)
    {
        /* Place your application code here. */
        if (mode == Generation) {
            LCD_Char_1_Position(0, 1);    
            LCD_Char_1_PrintString("GENERATING KEY"); // clean up the display    
            LCD_Char_1_Position(1, 1);
            
            GetRandom(key, KEY_SIZE);
            UART_PutString("Secret Key:");
            HexPrint(key, KEY_SIZE);
            KeySchedule(round_keys, key); // run key schedule
            
            mode = 1; // go into encryption mode after generating key
         
        } else { // encryption/decryption mode
            LCD_Char_1_ClearDisplay();
            LCD_Char_1_Position(0, 1);
            if (mode == Encryption) {
                LCD_Char_1_PrintString("ENCRYPTING");
            } else {
                LCD_Char_1_PrintString("DECRYPTING");
            }
            
            if(switch1_Read() == 0 || buf_full) {
                ButtonPressed: // label so we can automatically do this routine when buffer is full
                
                HexToByte(rec_buf, byte_buf, BLOCK_SIZE);
                
                if (rec_pointer % 2 == 0) {
                    UART_PutString("\n\rWARNING: ODD LENGTH INPUT");   
                }
        
                if (!buf_full) // if the user did not fill the buffer, we pad it with 0's
                    PadChunk(byte_buf, rec_pointer+1, BLOCK_SIZE);
               
                
                if (mode == Encryption) {
                    Encrypt(byte_buf, BLOCK_SIZE, round_keys);
                } else {
                    Decrypt(byte_buf, BLOCK_SIZE, round_keys);
                }
                
                
                HexPrint(byte_buf, BLOCK_SIZE);
                
                memset(byte_buf, 0, BLOCK_SIZE); // clearing byte_buf
                memset(rec_buf,0,BLOCK_SIZE*2); // clearing rec_buf
                
                buf_full = false;
                rec_pointer = -1;
                    
                while (switch1_Read() == 0) {} // ensure only one press processed at a time
        }

        if(switch2_Read() == 0) { // if button 2 pressed
            buf_full = false; // reset this block
            rec_pointer = -1; //
            
            if (mode == Encryption) { // if in encryption mode, go to decryption
                mode = Decryption;
            } else { // otherwise we are in decryption mode and go to encryption
                mode = Encryption;   
            }
            while (switch2_Read() == 0); // ensure only one press processed at a time
        }
    }
    CyDelay(delay);
        
         
}
}

/* [] END OF FILE */
