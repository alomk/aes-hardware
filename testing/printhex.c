#include <stdlib.h>
#include <stdio.h>

// This function takes a hex string and fills a byte array with the parsed values
void HexToByte(char* hex_string, char* buf, int len) {
    int i = 0;
    while ( i < len) {
        sscanf(hex_string+(2*i), "%02hhx", &buf[i]);
        i++;
    }
    // assuming all necessary sanitization+padding happens before this function is called
}

// This function takes an array of len bytes and prints out a hex string over UART
void HexPrint(char* buf, int len) {
    char ret[2*len]; //each number in buffer will have two hex digits
    int i = 0;
    while (i < len) {
        sprintf(ret+(i*2), "%02hhX", buf[i]);
        i++;
    }
    printf("%s\n", ret);
}

int main() {
    printf("Starting test\n");


    char buf[4];
    char* test2 = "deadbeef";

    HexToByte(test2, buf, 4);
    HexPrint(buf, 4);
}
