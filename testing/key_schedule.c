#include <stdint.h>    /* to be sure the bit widths are correct*/
#include <stdio.h>     /* for printf */

typedef uint8_t u8;
typedef uint32_t u32;

int main () {
  int i;
  u8 x=0xcb;

  for( i = 0; i < 11; i++) {
     x = (x<<1) ^ (x&0x80 ? 0x1B : 0x00);
     printf("%02i: 0x%02x \n",i-1,x);
  }

  return(0);
}
