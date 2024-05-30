#include "../lib/utf8.c"
// #include "../lib/shannon.c"

// # GOAL:
// + encode data with parity checks in mind
// + make shure all the data is valid and not 
// corrupted.
// + if corrupted idicate error
//
// # EXPLANATION:
//
// + encode all data into parity blocks (16 bit) / (2 bytes)
// + make shure last bit that indicated sign is parity bit
// if N of 1`s is even its {1} else {0}
// + make shure error detection works

// #define DEBUG

// consts
#define BYTELEN 8
#define EVENPARITY 1
#define ODDPARITY 0
#define BELL 7
#define MAX_BLOCK_SIZE 2047
#define BLOCK_BITS 16
#define PARITY_BITS 4
#define NO_ERROR 0
#define UNIT_PARITY_ERROR 1
#define BLOCK_PARITY_ERROR 2

// types
typedef uint16_t u16;
typedef uint16_t H16BB; // hamming 16 bit block

// function pointer type mask
typedef uint (*check_operation) (u16);

#include <math.h>
#include <stdbool.h>
#include <stdio.h>
 
char ipo2(int n)
{
    if (n == 0)
        return false;
    return (ceil(log2(n)) == floor(log2(n)));
}

unsigned char get_bit(int num, int i)
{
    return ((num & (1 << i)) != 0);
}

int set_bit(int num, int i)
{
    return num | (1 << i);
}

int clear_bit(int num, int i)
{
    int mask = ~(1 << i);
    return num & mask;
}

uint even_count_col1(u16 blk) {
  // check for even bits in even columns 
  // x  1  x  1
  // x  1  x  1
  // x  1  x  1
  // x  1  x  1
  uint bit_count = 0;
  for(uint i = 0; i < BLOCK_BITS; i++)
    if(!(i % 2 == 0) && !(i == 0)) 
    // if(!(i % 2 == 0) && !(i == 0) && !(ipo2(i))) 
      if(get_bit(blk, i))
        bit_count++;
  return bit_count;
}

uint even_count_col2(u16 blk) {
  // check for columns CFC2
  // x  x  1  1
  // x  x  1  1
  // x  x  1  1
  // x  x  1  1
  uint bit_count = 0;
  for(uint i = 0; i < BLOCK_BITS; i++)
    if(!(i % 4 < 2) && !(i == 0)) 
    // if(!(i % 4 < 2) && !(i == 0)&& !(ipo2(i))) 
      if(get_bit(blk, i))
        bit_count++;

  return bit_count;
}

uint even_count_row1(u16 blk) {
  // check for rows CFR1
  // x  x  x  x
  // 1  1  1  1
  // x  x  x  x
  // 1  1  1  1
  uint bit_count = 0;
  for(uint i = 0; i < BLOCK_BITS; i++)
    if(!(i % 8 < 4) && !(i == 0) ) 
    // if(!(i % 8 < 4) && !(i == 0) && !(ipo2(i))) 
      if(get_bit(blk, i))
        bit_count++;

  return bit_count;
}

uint even_count_row2(u16 blk) {
  // check for rows CFR2
  // x  x  x  x
  // x  x  x  x
  // 1  1  1  1
  // 1  1  1  1
  uint bit_count = 0;
  for(uint i = 0; i < BLOCK_BITS; i++)
    // if(!(i % 16 < 8) && !(i == 0)&& !(ipo2(i))) 
    if(!(i % 16 < 8) && !(i == 0)) 
      if(get_bit(blk,i))
        bit_count++;
    
  return bit_count;
}

H16BB encode_block(u16 data) {
  #define COLUMN_1 0
  #define COLUMN_2 1
  #define ROW_1    2
  #define ROW_2    3

  H16BB blk = 0;
  uint  blk_i = 0;
  char bit_count      [4] = {0};

  if (data > MAX_BLOCK_SIZE)
    return 0;

  // data = convert11to16(data);

  for(uint i = 0; i < BLOCK_BITS; i++)
    if(!ipo2(i) && !(i == 0)) {
      char data_bit = get_bit(data,blk_i);
      blk = (data_bit)? set_bit(blk,i) : clear_bit(blk,i);
      blk_i++;
    }
  


  // blk = clear_bit(blk, 15);

  bit_count[COLUMN_1] = even_count_col1(blk);
  bit_count[COLUMN_2] = even_count_col2(blk);
  bit_count[ROW_1] = even_count_row1(blk);
  bit_count[ROW_2] = even_count_row2(blk);


  // printf("\nBIT COUNTS: { %d, %d, %d, %d }\n",bit_count[0],bit_count[1],bit_count[2],bit_count[3]);

  // FLIP PAIRT CHECK BITS
  for(uint i = 0; i < PARITY_BITS; i++) {
    if(bit_count[i] % 2 == 1)   
      blk = set_bit(blk,pow(2, i));
    
    // printf("PARITY BIT INDEX: %d",(int)pow(2,i));
  }

  return blk;
}



u16 decode_block(H16BB blk) {

  // GET BITS
  uint blk_i = 0;
  char bits[BLOCK_BITS-(4-1)];


  for(uint i = 0; i < BLOCK_BITS; i++)
    if(!ipo2(i) && !(i == 0)) {
      bits[blk_i] = get_bit(blk, i);
      blk_i++;
    }


  // printf("[");
  // for(uint i = 0; i < 11; i++)
  //     printf(" %d",bits[i]);
  // printf(" ]");
  
  // SET BITS
  u16 r = 0;
  

  for (uint i = 0; i < BLOCK_BITS-4-1; i++)
    r = (bits[i])? set_bit(r, i) : clear_bit(r, i); 
  
  return r;
  // return convert16to11(r);
  // return convert11to16(r);
}

char detect_error(H16BB blk) {
  check_operation checks[PARITY_BITS] = {
    even_count_col1,
    even_count_col2,
    even_count_row1,
    even_count_row2,
  };
  uint total_bits = 0;
  char error = 0;

  // NORMAL
  // x  1  0  1
  // 0  0  0  0
  // 0  0  0  1
  // 0  0  0  1


  // ERROR
  // x  1  0  1
  // 0  0  0  0
  // 0  0  0  1
  // 0  0  0  0
  
  size_t parity_bit[PARITY_BITS] = { 1, 2, 4, 8 };

  // which means pairs should aways be even
  for (uint i = 0; i < PARITY_BITS; i++) 
    // if (NOTEVEN) = ERROR
    if (checks[i] (blk) % 2) {
    // if (checks[i] (blk) % 2 == 0 && !parity_bit[i]) {
      printf("\n\t\tDETECTED ERROR IN { %d } check NOC: (%d)",i,checks[i](blk));
      error = UNIT_PARITY_ERROR;
    }

  // calculate parity bits
  for(uint i = 0; i < BLOCK_BITS; i++)
    if(i != 0 && get_bit(blk, i))
      total_bits++;

  // if parity bit is even but any of the checks is errored
  // this means two bit errors
  if (total_bits % 2 && error == UNIT_PARITY_ERROR)
    error = BLOCK_PARITY_ERROR;

  return error;
}

void debug(H16BB blk, u16 orig) {

  printf("\tFUll block :\n [");
  for(uint i = 0; i < BLOCK_BITS; i++)
      printf("%d", get_bit(blk,i));
  printf("]\n");

  for(uint i = 0; i < BLOCK_BITS; i++) {
    printf("%d ",get_bit(blk,i));
    if (i % 4 > 2 )
      printf("\n");
  }

}

void print_block(H16BB blk, short full) {
  
  printf("[ ");
  for(uint i = 0; i < BLOCK_BITS; i++)
    if (!full && (i < 3 || i > 3 + 11))
      printf("x");
    else
      printf("%d", get_bit(blk,i));
  printf("]\t");
}


uint find_flipped_bit(u16 block) {
  uint error_bit = 0;
  uint error_check = 0;

  check_operation checks[PARITY_BITS] = {
    even_count_col1,
    even_count_col2,
    even_count_row1,
    even_count_row2,
  };

  // Perform parity checks to detect error bit
  for (uint i = 0; i < PARITY_BITS; i++) {
    error_check = checks[i](block);
    if (error_check % 2 == 1) {
      error_bit += 1 << (i);
    }
  }

  return error_bit; // Return the index of the flipped bit
}

void hamming_correct(u16 *block) {
  size_t error_bit = find_flipped_bit(*block);
  if (get_bit(*block, error_bit))
    *block = clear_bit(*block, error_bit);
  else 
    *block = set_bit(*block, error_bit);
}




// int main(void) {
//  
//   uint d = 1052;
//   u16 data = (u16)d;
//
//   printf("ORIGINAL DATA: \n\t");
//   print_block(data, 1);
//
//   H16BB b = encode_block(data);
//
//
//   // printf("DECODED: %d\n",extractDataFromHamming(b));
//   printf("DECODED: %d\n",decode_block(b));
//
//   debug(b,data);
//   
//   // b = set_bit(b, 14);
//   // b = clear_bit(b, 6);
//
//   printf("\nAFTER ERROR\n");
//   debug(b,data);
//
//   if (detect_error(b) == UNIT_PARITY_ERROR) {
//     printf("SINGLE ERROR");
//     hamming_correct(&b);
//     printf("\n ERROR AT INDEX %d",find_flipped_bit(b));
//   }
//   else if (detect_error(b) == BLOCK_PARITY_ERROR)
//     printf("DOUBLE BIT ERROR");
//
//
//   // printf("\n CORRUPTED BIT = %d\n",find_flipped_bit(b));
//
//   printf("AFTER CORRECTION \n");
//   debug(b,data);
//
//   // for (uint i = 0; i < BLOCK_BITS; i++)
//   // {
//   //   if (i > 4)
//   //     test = set_bit(test, i);
//   //   else 
//   //     test = clear_bit(test,i);
//   // }
//
//   printf("\n\n");
//   
//   print_block(b, 1);
//
//   // printf("11 bit ver of %u: ",convert11to16(test));
//   // print_block(convert11to16(test),1);
//   //
//   //
//   // printf("\n 16 bit ver of %u: ",test);
//   // print_block(test,1); 
//   // printf("\ntest: %u\n",test);
//
// }

 int main(void) {
   // load file
   char* str = load_string("file.txt");
    
   // transform to codepoints
   uint* codepoints_array = u8_as_codepoint_array(str);
   printf("\n\n\tString Original:\t`%s`\n\n",str);


   printf("\n\n\tString as UTF8 DECIMAL codepoints:\n\n");
   for(uint i = 0; codepoints_array[i] != 0; i++)
     if (i % 20 == 1)
       printf("\t%u \n",codepoints_array[i]);
     else 
       printf("%u ",codepoints_array[i]);




   printf("\n\nhamming encode for autocorrection: \n\n");
   size_t len = u8strlen(str);
   H16BB hamming[len];

   for(uint i = 0; i < len; i++) {
     u16 data = (u16)codepoints_array[i];
     H16BB b = encode_block(data);
     hamming[i] = b;
     
     if (i % 5 == 1)
       printf("\n\t");

     print_block(b,1);
     // printf(" [%d] ",b);
   }



  // CREATE ERRORS FOR TESTING
  printf("\nCREATE ERRORS FOR TESTING : INDEXES{1: {bit5},\t2: {bit5,bit7}}\n");

  hamming[1] = set_bit(hamming[1], 5);
  hamming[2] = set_bit(hamming[2], 5);
  hamming[2] = set_bit(hamming[2], 7);


   const uint TAIL_NULL = 1;
   uint* codepoints_back = calloc(len+TAIL_NULL, sizeof(uint));


   printf("\n\nCheck for errors, print result\n\n");
  for (uint i = 0; i < len; i++) {
    H16BB item = hamming[i];
    if (detect_error(item)==UNIT_PARITY_ERROR) {
      printf("\t DETECTED SINGLE ERROR IN "
             "(codepoint: %u,bit: %u)",
             i,find_flipped_bit(item));

      printf(" :: RECOVERING\n");
      hamming_correct(&item);
      codepoints_back[i] = (uint) decode_block(item);
    }
    else if (detect_error(item)==BLOCK_PARITY_ERROR) {
      codepoints_back[i] = -1;
      printf("\t DETECTED TWO OR MORE ERRORS! :: UNRECOVERABLE!\n");
    }
    else 
      codepoints_back[i] = (uint)decode_block(item);
  }


   printf("\n\n\tUTF8 DECIMAL codepoints AFTER HAMMING:\n\n");
   for(uint i = 0; codepoints_back[i] != 0; i++)
     if (i % 20 == 1)
       printf("\t%u \n",codepoints_back[i]);
     else 
       printf("%u ",codepoints_back[i]);

    char* str_back;

    printf("\n\n\n\nUTF8 string after CORRECTION: \n\t%s\n",(str_back = codepoint_array_as_u8str(codepoints_back)));

   // defer
   free(str);
   free(str_back);
   // free(back);
   free(codepoints_array);
   free(codepoints_back);
   // for(uint i = 0; i < text_legnth; i++) 
   //     free(compressed_sequence[i]);
 }
