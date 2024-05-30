#include "../lib/utf8.c"
#include "../lib/shannon.c"

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

// types
typedef short int i16;

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

size_t num_of_ones(i16 block) {
  size_t noo = 0;
  for(uint i = 0; i < sizeof(i16)*BYTELEN; i++) 
    if (get_bit(block,i))
      noo++;
  return noo;
}

i16 *create_parity_blocks(uint* codepoints_array, size_t array_len) 
{
  const size_t LAST_BIT = sizeof(i16) * BYTELEN - 1;
  i16 *arr = malloc(array_len * sizeof(i16));

  for(uint i = 0; i < array_len; i++) {
    arr[i] = (i16)codepoints_array[i];
    if (num_of_ones(arr[i]) % 2) // if bit is even
      arr[i] = set_bit(arr[i],LAST_BIT);
 
    #ifdef DEBUG
printf("arr[%d] = %d\n",i,arr[i]);
    #endif /* ifdef DEBUG */
  }

  return arr;
}


char detect_single_error(i16 block) {
  const char ERROR = 1;
  const char OK    = 0;

  size_t noo = 0;

  const size_t BIT_BOUND = sizeof(i16) * BYTELEN - 1;

  for(uint i = 0; i < BIT_BOUND; i++) 
    if (get_bit(block, i))
      noo++;

  char last_bit = get_bit(block, BIT_BOUND);
  char bits_even = noo % 2;

#ifdef DEBUG
  printf("bits: [");
  for(uint i = 0; i < BIT_BOUND; i++)
    printf("%d",get_bit(block,i));
  printf("], last_bit : %s", (last_bit == EVENPARITY)? "EVEN" : "ODD");
#endif

  if (last_bit == EVENPARITY && !bits_even)
    return ERROR;
  if (last_bit == ODDPARITY && bits_even)
    return ERROR;
 
  
  return OK;
}


uint *evaluate (i16* block_array, size_t length)
{
  const uint ERROR = 7;
  const size_t LAST_BIT = sizeof(i16) * BYTELEN - 1;

  uint *arr = malloc(sizeof(uint) * (length + 1) );
  arr[length] = 0;
  for(uint i = 0; i < length; i++)
    if (detect_single_error(block_array[i])) {
      printf("DETECTED SINGLE ERROR IN BLOCK [%d]\n",i);
      arr[i] = ERROR; // 7 is bel symbol and was never meant to be used in normal UTF-8 or ASCII strings 
    }
    else {
      if (!get_bit(block_array[i], LAST_BIT))
        arr[i] =  block_array[i];//(set_bit((i16)arr[i],LAST_BIT));
      else 
        arr[i] =  (i16) clear_bit(block_array[i],LAST_BIT);//(set_bit((i16)arr[i],LAST_BIT));
    }

  return arr;
}


int main(void) {
 
  // const uint UA_ALPHABET_SIZE    = 32 + 6;
  // const uint UA_UTF8_SIZE        = 11; // 6 + 5 bytes
  float entropy                  = 0;
  // size_t bitwidht                = 0;
  // size_t bitwidht_compressed     = 0;
  // size_t bytes                   = 0;
  // size_t bytes_compressed        = 0;
  // int    string_character_count  = 0;


  // load file
  char* str = load_string("file.txt");
   
  // transform to codepoints
  uint* codepoints_array = u8_as_codepoint_array(str);
  printf("\n\n\tString Original:\t`%s`\n\n",str);

  // create and sort dictionary from codepoints
  size_t dict_size = 0;
  Character* dictionary = create_dictionary(codepoints_array, &dict_size, &entropy);
  sort_dictionary(dictionary, dict_size);

  // set up Tree Root
  Node* tree_root = malloc(sizeof(Node));
  tree_root->content_size = dict_size;
  tree_root->content = dictionary;
  tree_root->side = Top;
  tree_root->type = Root;

  // Build tree from the root
  build_tree(tree_root);


  // len
  size_t text_legnth = 0;
  for(uint i = 0; codepoints_array[i] != ARRAY_END; i++)
      text_legnth++;

  // allocate message array
  const size_t MAX_TREE_DEPTH = 10;
  char** compressed_sequence = malloc(sizeof(char*) * (text_legnth) );

  // fill it up with compressed identifiers using Shannons tree
  for (uint i = 0; i < text_legnth; i++) {
      uint depth = 0;
      compressed_sequence[i] = calloc(MAX_TREE_DEPTH,sizeof(char) );
      encode_with_tree(tree_root, codepoints_array[i], &depth, compressed_sequence[i]);
  }

  // print text string encoded
  printf(
          "\n\n\t Shennon-Fano coding for string: \n"
  );
  for (uint i = 0; i < text_legnth; i++) {
      if (i % 10 == 0)
          printf("\n\t\t");
      printf("[%s]\t",compressed_sequence[i]);
  }

  // one bit flip error detection
  printf(
          "\n\n\t Parity blocks for Shennon-Fano of string: \n"
  );


  // for(uint i = 0; i < text_legnth; i++) {
  //   printf("Error check arr[%d] : %d \t ",i, detect_single_error(par[i]));printf("\n");
  // }


  // decode text back into codepoints
  uint* codepoints = calloc(text_legnth+1, sizeof(uint));
  for (uint i = 0; i < text_legnth; i++) {
      const uint BEGINING = 0;
      codepoints[i] = decode_with_tree(
              tree_root, 
              compressed_sequence[i], 
              BEGINING
            );
  }
 

  // ERROR CHECK
  i16 *par = create_parity_blocks(codepoints,text_legnth);
  
  // random error 
  par[1]  = set_bit(par[1],15);
  par[23] = set_bit(par[23],2);
  par[33] = set_bit(par[33],6);
  //
  uint* errors = evaluate(par,text_legnth);

  // back to stirng 
  char* back = codepoint_array_as_u8str(codepoints);
  printf("\n\n\tString After compression and utf8 decoding:\n\t\t`%s`\n",back);


  printf(
          "\n\n\t Shennon-Fano DECODED: \n"
  );
  for (uint i = 0; i < text_legnth; i++) {
      if (i % 10 == 0)
          printf("\n\t\t");
      printf("[%d]\t",errors[i]);
  }





  // defer
  free(str);
  free(errors);
  free(par);
  free(back);
  free(codepoints);
  for(uint i = 0; i < text_legnth; i++) 
      free(compressed_sequence[i]);
}
