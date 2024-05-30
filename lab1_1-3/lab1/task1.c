#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <math.h>

#define MAX_STRING_SIZE 256
#define EOL '\n'
#define END NULL
#define PRINT_AS_CODEPOINTS 0
#define PRINT_AS_BYTES      1

#define HEAD_4BYTE_BITMASK 0x07
#define HEAD_3BYTE_BITMASK 0x0F
#define HEAD_2BYTE_BITMASK 0x1F

#define PAYLOAD_BITMASK 0x3F

#define U8_GRAPHEME_SIZE 4 

// macro for debugging
// #define DEBUG

// generic termaintion function
void die(char* error) {
  fprintf(stderr,"%s", error);
  exit(-1);
} 

// generic swap for numbers
void swap(int *p1, int *p2) {
    int temp = *p1;
    *p1 = *p2;
    *p2 = temp;
}


// loads the string from a file,
// expects the string to be within 256 bytes.
//
// On error terminates with -1
//
// On string bigger than a buffer, prints warning
char* load_string(char* fpath) {
  FILE* f = fopen(fpath, "r");
  char* string_buffer = malloc(MAX_STRING_SIZE);
  memset(string_buffer, 0, MAX_STRING_SIZE);

  if (!f) { 
    free(string_buffer);
    die("failed to open file!");
  }

  if (!string_buffer) {
    free(string_buffer);
    die("failed to use malloc! buy more ram!?");
  }

  for (uint i = 0; i < MAX_STRING_SIZE; i++)
  {
    char c = fgetc(f);
    
    if (c == EOF || c == EOL)
      break;
   
    sprintf(string_buffer,"%s%c",string_buffer, c);
  }

  return string_buffer;
}

// gets length of the string taking UTF-8 in 
// consideration.
// 
// does not check for encoding validity
int u8strlen(const char *s)
{
  int len=0;
  while (*s) {
    if ((*s & 0xC0) != 0x80) 
      len++;
    s++;
  }
  return len;
}

// Get number of bytes in UTF-8 grapheme
uint u8_nbyte(const char *u8char) {
    const uint8_t *byte = (const uint8_t *)u8char;

  // UTF-8 encoding:
  //
  // 1   byte: Head
  // 0-3 byte: Payload
  //
  // IN case of grapheme taking only one byte, 
  // treat it like int_8:
  //  0xxxxxxx
  //  ^~~~~~~~--> Payload
  //  |
  //  |
  //  means unsighned
  // 
  // encoding table:
  // 
  //  BYTE1    BYTE2    BYTE3    BYTE4
  //
  // [0xxxxxxx]
  // [110xxxxx 10xxxxxx]
  // [1110xxxx 10xxxxxx 10xxxxxx]
  // [11110xxx 10xxxxxx 10xxxxxx 10xxxxxx]
  //
  // UTF-8

  int num_bytes = 0;
    if ((*byte & 0x80) == 0x00) { // checking for pattern:        10xxxxxx
        num_bytes = 1;
    } else if ((*byte & 0xE0) == 0xC0) { // checking for pattern: 110xxxxx
        num_bytes = 2;
    } else if ((*byte & 0xF0) == 0xE0) { // checking for pattern: 1110xxxx
        num_bytes = 3;
    } else if ((*byte & 0xF8) == 0xF0) { // checking for pattern: 1111xxxx
        num_bytes = 4;
    }

    return num_bytes;
}

// Decode UTF-8 grapheme -> src string, with known amount of bytes -> nbyte
int u8_char_decode(const char *src ,uint nbyte) {

  const uint8_t *byte = (const uint8_t *)src;
  uint codepoint = 0;

    // Decode the UTF-8 sequence the rough way
    switch (nbyte) {
        case 1:
            codepoint = *byte;
            break;
        case 2:
            codepoint = (*byte & HEAD_2BYTE_BITMASK) << 6;
            byte++;
            codepoint |= (*byte & PAYLOAD_BITMASK);
            break;
        case 3:
            codepoint = (*byte & HEAD_3BYTE_BITMASK) << 12;
            byte++;
            codepoint |= (*byte & PAYLOAD_BITMASK) << 6;
            byte++;
            codepoint |= (*byte & PAYLOAD_BITMASK);
            break;
        case 4:
            codepoint = (*byte & HEAD_4BYTE_BITMASK) << 18;
            byte++;
            codepoint |= (*byte & PAYLOAD_BITMASK) << 12;
            byte++;
            codepoint |= (*byte & PAYLOAD_BITMASK) << 6;
            byte++;
            codepoint |= (*byte & PAYLOAD_BITMASK);
            break;
        default:
            printf("Invalid UTF-8 sequence\n");
            break;
    }
  return codepoint;
}

// Get codepoint for UTF-8 grapheme using 2 oftenly used functions
uint u8char(const char *u8char) 
{
    uint codepoint = 0;
    uint num_bytes = u8_nbyte(u8char);
    return u8_char_decode(u8char, num_bytes);
}




// convert to codepoint array all characters in UTF-8 string
uint* u8_as_codepoint_array(const char *u8_str) {
  
  uint* buffer = malloc( (u8strlen(u8_str) + 1)  * sizeof(uint));
  uint codepoint;
  
  buffer[u8strlen(u8_str)] = 0;

  uint u8len = u8strlen(u8_str);

#ifdef DEBUG
  printf("LEN OF U8STR is = %d\t ",u8len);
#endif 

  for(uint i = 0; i < u8len; i++)
  {
    codepoint = u8char(u8_str);
    uint symsize = u8_nbyte(u8_str);

#ifdef DEBUG
    printf("codepoint: `%d` with size: %d\n",codepoint, symsize);
#endif

    buffer[i] = codepoint;
    for(uint j = 0; j < symsize; j++) 
      u8_str++;
  }
  return buffer;
}


// convert to array of char* , all characters in UTF-8 string
char** u8_as_str_array(const char *u8_str) {
  
  char** buffer = malloc( ( u8strlen(u8_str) + 1 ) * sizeof(char*));
   buffer[u8strlen(u8_str)] = END; // easier to iterate over like a string.

  for (uint i = 0; i < u8strlen(u8_str); i++) {
    buffer[i] = malloc( sizeof(char) * U8_GRAPHEME_SIZE); // its 4 bytes.
    memset(buffer[i], 0, 4);
  }
  uint buffer_i = 0;
  uint index = 0;
  uint codepoint;
  uint bytecount;


  uint u8len = u8strlen(u8_str);

#ifdef DEBUG
  printf("LEN OF U8STR is = %d\t ",u8len);
#endif 

  for(uint i = 0; i < u8len; i++)
  {
    codepoint = u8char(u8_str);
    uint symsize = u8_nbyte(u8_str);

#ifdef DEBUG
    printf("char : `%d` with size: %d\n",codepoint, symsize);
#endif

    for(uint j = 0; j < symsize; j++)  {
      sprintf(buffer[i],"%s%c", buffer[i],*u8_str);
      u8_str++;
    }
  }
  return buffer;
}


uint u8_words(const char* str){
  uint wc = 0;
  uint seen_letter = 0;

  for (uint i = 0; str[i] != 0; i++)
  {
    if (str[i] != ' ')
      seen_letter = 1;


    if (str[i] == ' ' 
        || str[i] == '\n' 
        || str[i] == '\t'
        || str[i] == '\0'
        && seen_letter
       ) 
    {
      wc++;
    }
  }

  if (seen_letter && wc >= 1)
    wc ++;
  else if (seen_letter)
    wc ++;

  return wc;
}


// sort 
void sort(uint* arr, int n)
{
    int i, key, j;
    for (i = 1; i < n; i++) {
        key = arr[i];
        j = i - 1;

        while (j >= 0 && arr[j] > key) {
            arr[j + 1] = arr[j];
            j = j - 1;
        }
        arr[j + 1] = key;
    }
}

float self_information(float chance) {
  return ( -log2(chance) );
}


float shannons_value(float entropy, uint wordcount)
{
  return entropy / wordcount;
}

double krafts_value(uint symbols_count, uint alphabet_size)
{
  return pow(2,-5.0) * 32;
  //return pow(2, -symbols_count) * alphabet_size;
}


// calculate max occurances of an item
// create and array of apropriate size
// put each occurances to its apropriate location
// in size array.
uint* frequency_array(uint* codepoints, uint* unique_codepoint, float* entropy) {

  uint len = 0;
  uint last_codepoint = 0;
  uint cur_count = 0;
  uint max_count = 0;

  for(uint i = 0; codepoints[i] != 0; i++)
    len++;

  sort(codepoints, len);

  // find max_count of codepoint
  for(uint i = 0; codepoints[i] != 0; i++) {
    uint el = codepoints[i];

    if (cur_count > max_count)
      max_count = cur_count;

    if (el == last_codepoint)
      cur_count++;
    else {
      last_codepoint = el;
      cur_count = 1;
    }
  }

  *unique_codepoint = max_count;
  
  // create buffer and memeset it to 0
  last_codepoint = 0;
  // cur_count = 1;
  const uint EMPTY = 1;

  uint* count_array = malloc(sizeof(uint) * (max_count+1+1));
  for(uint i = 0; i < max_count; i++)
    count_array[i] = 1;
  count_array[max_count+1] = 0;

  // assign each occurance to its count (index)
  for(uint i = 0; codepoints[i] != 0; i++) {
    uint el = codepoints[i];

    if (el == last_codepoint)
      cur_count++;
    else {
      if (last_codepoint != 0) {
        printf("\n\tcodepoint (%d) was found -> %d",last_codepoint,cur_count);
        float chance = (float)cur_count / (float)len;
        (*entropy) +=  chance * self_information(chance);
      }
      count_array[cur_count] = last_codepoint;
      last_codepoint = el;
      cur_count = 1;
    }
  }

  return count_array;
}



// print array of strings (each one is UTF-8 character)
void array_print(char** arr) {
  printf("\t");
    for(uint i = 0; arr[i] != 0; i++)
      if (strcmp(arr[i], " ") == 0)
        printf("\n\t");
      else
        printf("[%s] ",arr[i]);
}

// print values as codepoint or grouped decimal binary values
void u8_print(const char *u8_str, int flags) {
  uint codepoint;
  uint bytecount;
  
  char* type = (flags == PRINT_AS_CODEPOINTS)? "codepoint" : "raw bytes";

  printf(
          "\n\n\n"
          "\t\t UTF-8 string encoded as decimal - "
          "%s:\n\n", type
  );

  uint u8len = u8strlen(u8_str);


  for(uint i = 0; i < u8len; i++)
  {
    codepoint = u8char(u8_str);
    uint symsize = u8_nbyte(u8_str);
    if (flags == PRINT_AS_CODEPOINTS)
        printf("\t\t[%d]",codepoint);
    else if (flags == PRINT_AS_BYTES)
        printf("\t\t[");
    for(uint j = 0; j < symsize; j++) {
        if (flags == PRINT_AS_BYTES) 
            printf("[%d]",*u8_str);
        
        u8_str++;
    }

    if (flags == PRINT_AS_BYTES)
        printf("] -> \t char[%d]\n",i);
  }

  printf("\n\n\n");
  
}



// convert binary uint to number
uint binary_to_decimal(uint binary) {
  uint decimal = 0;
  uint weight  = 1;
  uint rem     = 0;

  while(binary != 0)
  {
    rem      =  binary % 10;
    decimal  += rem * weight;
    binary   /= 10;
    weight   *= 2;
  }

  return decimal;
}

// convert decimal to uint binary
void binary(uint8_t decimal) {
   printf(" ");
  int numBits = sizeof(decimal) * 8;

  // Loop through each bit from left to right
  for (int i = numBits - 1; i >= 0; i--) {
    // Use bitwise AND to check if the bit is 1 or 0
    if (decimal & (1 << i)) {
      printf("1");
    } else {
      printf("0");
    }
  }
  printf(" ");
}

// print grays of a decimal
void grays(uint8_t decimal)
{
  // xor next
  uint8_t grays = decimal ^ (decimal >> 1);
  binary(grays);
}


// print decimal content of string
void slice_print(char* string)
{
    printf("\n\n\t\t "
            "Raw decimal values of incoded UTF-8 string"
            "(negative indicate non ASCII characters): \n\n"
    );
    
    for(uint i = 0; i < strlen(string); i++)
        printf("[%d]\t",(uint)string[i]);
    printf("\n");
}

// print each codepoint as binary
void binary_print(char* string) 
{
  printf("\n\t\tBinary of string:\n");

  const uint8_t *byte = (const uint8_t *)string;
  for(uint i = 0; byte[i] != '\0'; i++)
  {
    if (i % 10 == 0) 
      printf("\n");
    binary(byte[i]);
  }
  printf("\n\n");
}


// print grays content of a string
void grays_print(char* string) {

  printf("\n\t\tGrays Binary of string:\n");

  const uint8_t *byte = (const uint8_t *)string;
  for(uint i = 0; byte[i] != '\0'; i++)
  {
    if (i % 10 == 0) 
      printf("\n");
    grays(byte[i]);
  }
  printf("\n\n");
}

int main(int argc, char** argv) {
 
  if (argc != 2) 
    die("Not enough arguments,"
        "expected file path to "
        "be provided as first "
        "argument.");

  // load str from file
  char* string = load_string(argv[1]);
 

  // print str
  printf(
      "\n\n"
      "\tString: '%s'"
      "\n\n", string
  );

  // print utf-8 bytes untoched
  slice_print(string);

  // print codepoints
  u8_print(string, PRINT_AS_CODEPOINTS);

  // utf8 string as bytes and Grays code
  binary_print(string);
  grays_print(string);

  uint*  cpa   = u8_as_codepoint_array(string);
  char** u8arr = u8_as_str_array(string);
  
  // print utf8 symbols
  printf(
      "\n\n"
      "\tString broken down to its symbols:"
      "\n\n" 
  );
  array_print(u8arr);

  printf("\n\n\tCODEPOINTS SORTED:\n");
  printf("strlen: %d\n", u8strlen(string));

  for(uint i = 0; cpa[i] != 0; i++)
    printf("\t%d", cpa[i]);


  float entropy = 0.0;
  uint unique = 0;
  uint* freq = frequency_array(cpa, &unique, &entropy);

  printf(
      "\n\n\t" "Chart of codepoints rarity:" "\n"
  );

  for(uint i = 0; freq[i]!=0; i++)
    if (freq[i]!=1) {
      unique++;
      printf("\tcodepoint: [ %d ], count ->\t%d\t [",freq[i],i);
      for(uint c =0; c < i; c++)
        printf("#");
      printf("]\n");
    }

  const uint UA_UTF8_SIZE     = 11;
  const uint UA_ALPHABET_SIZE = 38;

  // Stats
  printf(
      "\n\n\n\n"
      "+\t\t" "String information:" "\n"
      "|\t"   "\n"
      "|\t"   "Number of UTF-8 characters  : %d"       "\n"
      "|\t"   "Byte`s taken to store data  : %ld"      "\n"
      "|\t"   "Bit length (bytes * 8)      : %ld"      "\n"
      "|\t"   "Unique characters count     : %d"       "\n"
      "|\t"   "Words in string             : %d"       "\n"
      "|\t"   "Etropy for the string       : %.2f"     "\n"
      "|\t"   "Kraft's value (UA+SYM = 38) : %.13lf"   "\n"
      "|\t"   "Shannons value for UA_UTF8  : %.2f"     "\n"
      "+\n"
      , 

      u8strlen(string) , 
      strlen(string), 
      strlen(string)*8, 
      unique, 
      u8_words(string), 
      entropy,
      krafts_value(UA_UTF8_SIZE,UA_ALPHABET_SIZE),
      shannons_value(entropy, UA_UTF8_SIZE)
  );


  // defer block
  if (string)
    free(string);

  if (cpa) 
    free(cpa);

  if (freq)
    free(freq);

  if (u8arr) {
    for(uint i = 0; u8arr[i] != 0; i++)
      free(u8arr[i]);
    free(u8arr);
  }
  //
}

