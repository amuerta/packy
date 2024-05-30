#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

// UTF-8 only headerfile

#define ARRAY_END 0
// #define EOL '\n'
//
// #define HEAD_4BYTE_BITMASK 0xF0
// #define HEAD_3BYTE_BITMASK 0xE0
// #define HEAD_2BYTE_BITMASK 0xC0
// #define PAYLOAD_BITMASK 0x3F
//

// #define MAX_STRING_SIZE 256
#define EOL '\n'
#define END NULL
// #define PRINT_AS_CODEPOINTS 0
// #define PRINT_AS_BYTES      1

#define HEAD_4BYTE_BITMASK 0x07
#define HEAD_3BYTE_BITMASK 0x0F
#define HEAD_2BYTE_BITMASK 0x1F

#define PAYLOAD_BITMASK 0x3F

// #define U8_GRAPHEME_SIZE 4 

// generic functions for terminating on error
void die(char* message) {
  fprintf(stderr,"error:\t%s\n",message);
  exit(-1);
}

char* load_string(char* fpath) {
  FILE* f = fopen(fpath, "r");
  char* string_buffer = malloc(1);
  memset(string_buffer, 0, 1);

  if (!f) { 
    free(string_buffer);
    die("failed to open file!");
  }

  if (!string_buffer) {
    free(string_buffer);
    die("failed to use malloc! buy more ram!?");
  }

  char c = 0;

  while( (c=fgetc(f)) != EOF )
  {
    
    if (c == EOL)
      break;
   
    string_buffer = realloc(string_buffer, strlen(string_buffer)+1+1);
    sprintf(string_buffer,"%s%c",string_buffer, c);
  }

  return string_buffer;
}

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


// char* u8_encode(uint codepoint, size_t size) {
//     char* encoded;
//     switch (size) {
//         case 1:
//             encoded = (char*)malloc(2); // 1 byte for the codepoint + 1 byte for null terminator
//             encoded[0] = (char)codepoint;
//             encoded[1] = '\0';
//             break;
//         case 2:
//             encoded = (char*)malloc(3);
//             encoded[0] = (char)((codepoint >> 6) | HEAD_2BYTE_BITMASK);
//             encoded[1] = (char)((codepoint & PAYLOAD_BITMASK) | 0x80);
//             encoded[2] = '\0';
//             break;
//         case 3:
//             encoded = (char*)malloc(4);
//             encoded[0] = (char)((codepoint >> 12) | HEAD_3BYTE_BITMASK);
//             encoded[1] = (char)(((codepoint >> 6) & PAYLOAD_BITMASK) | 0x80);
//             encoded[2] = (char)((codepoint & PAYLOAD_BITMASK) | 0x80);
//             encoded[3] = '\0';
//             break;
//         case 4:
//             encoded = (char*)malloc(5);
//             encoded[0] = (char)((codepoint >> 18) | HEAD_4BYTE_BITMASK);
//             encoded[1] = (char)(((codepoint >> 12) & PAYLOAD_BITMASK) | 0x80);
//             encoded[2] = (char)(((codepoint >> 6) & PAYLOAD_BITMASK) | 0x80);
//             encoded[3] = (char)((codepoint & PAYLOAD_BITMASK) | 0x80);
//             encoded[4] = '\0';
//             break;
//         default:
//             encoded = NULL;
//             printf("Invalid codepoint size\n");
//             break;
//     }
//     return encoded;
// }

// char* u8_encode(uint codepoint, size_t size) {
//     char* encoded = (char*)malloc(size + 1); // Allocate memory for the encoded string
//     if (encoded == NULL) {
//         printf("Memory allocation failed\n");
//         return NULL;
//     }
//
//     encoded[size] = '\0'; // Null terminate the string
//
//     switch (size) {
//         case 1:
//             encoded[0] = (char)codepoint;
//             break;
//         case 2:
//             encoded[0] = (char)((codepoint >> 6) | HEAD_2BYTE_BITMASK);
//             encoded[1] = (char)((codepoint & PAYLOAD_BITMASK) | 0x80);
//             break;
//         case 3:
//             encoded[0] = (char)((codepoint >> 12) | HEAD_3BYTE_BITMASK);
//             encoded[1] = (char)(((codepoint >> 6) & PAYLOAD_BITMASK) | 0x80);
//             encoded[2] = (char)((codepoint & PAYLOAD_BITMASK) | 0x80);
//             break;
//         case 4:
//             encoded[0] = (char)((codepoint >> 18) | HEAD_4BYTE_BITMASK);
//             encoded[1] = (char)(((codepoint >> 12) & PAYLOAD_BITMASK) | 0x80);
//             encoded[2] = (char)(((codepoint >> 6) & PAYLOAD_BITMASK) | 0x80);
//             encoded[3] = (char)((codepoint & PAYLOAD_BITMASK) | 0x80);
//             break;
//         default:
//             printf("Invalid codepoint size\n");
//             free(encoded); // Free memory before returning NULL
//             return NULL;
//     }
//
//     return encoded;
// }

char* u8_encode(uint codepoint, uint nbyte) {

    char* dest = malloc(nbyte+1);
    memset(dest, 0, nbyte+1);
    uint8_t *byte = (uint8_t *)dest;
    
    switch (nbyte) {
        case 1:
            *byte = (uint8_t)codepoint;
            break;
        case 2:
            *byte = (uint8_t)(((codepoint >> 6) & HEAD_2BYTE_BITMASK) | 0xC0);
            byte++;
            *byte = (uint8_t)((codepoint & PAYLOAD_BITMASK) | 0x80);
            break;
        case 3:
            *byte = (uint8_t)(((codepoint >> 12) & HEAD_3BYTE_BITMASK) | 0xE0);
            byte++;
            *byte = (uint8_t)(((codepoint >> 6) & PAYLOAD_BITMASK) | 0x80);
            byte++;
            *byte = (uint8_t)((codepoint & PAYLOAD_BITMASK) | 0x80);
            break;
        case 4:
            *byte = (uint8_t)(((codepoint >> 18) & HEAD_4BYTE_BITMASK) | 0xF0);
            byte++;
            *byte = (uint8_t)(((codepoint >> 12) & PAYLOAD_BITMASK) | 0x80);
            byte++;
            *byte = (uint8_t)(((codepoint >> 6) & PAYLOAD_BITMASK) | 0x80);
            byte++;
            *byte = (uint8_t)((codepoint & PAYLOAD_BITMASK) | 0x80);
            break;
        default:
            printf("Invalid UTF-8 sequence\n");
            break;
    }
    return dest;
}




// Get codepoint for UTF-8 grapheme using 2 oftenly used functions
uint u8char(const char *u8char) 
{
    uint codepoint = 0;
    uint num_bytes = u8_nbyte(u8char);
    // printf("NOB: %d\n", num_bytes);
    return u8_char_decode(u8char, num_bytes);
}

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

char* codepoint_array_as_u8str(uint* cpa) {
  // size_t cp_count = 0;
  char* string = malloc(1);
  size_t grapheme_size = 1;

  const size_t TAIL = 1;

  // for(uint i = 0; cpa[i] != ARRAY_END; i++)
  //   cp_count++;

  memset(string, 0, 1);

  for(uint i = 0; cpa[i] != ARRAY_END; i++)
  {
    uint codepoint = cpa[i];

    if (codepoint <= 127) // support only for half of UTF-8 symbols
      grapheme_size = 1;
    else
      grapheme_size = 2;

    string = realloc(string, strlen(string)+grapheme_size+TAIL);
    char* grapheme = u8_encode(codepoint, grapheme_size);
    // printf("grapheme: %s\n",grapheme);
    // strcat(string, grapheme);
    sprintf(string, "%s%s", string,grapheme);
    free(grapheme);
  }
  return string;
}
