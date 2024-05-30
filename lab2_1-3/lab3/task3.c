#include "../lib/utf8.c"
// #include "../lib/shannon.c"
#include "../lib/data.c"
#include <stdio.h>


/*
 * LZ-78
 * Lampel-Ziv alghorithm 1978 
 *
 * IMPLEMENTATION LOGIC:
 *    PSEUDO-CODE:
 *
 * Create empty dictionary
 * Create empty package - encoded message
 *
 * Iterate over sequance of characters {
 *   perform a check for each one:
 *      if one-char pattern is not in the dictionary
 *        insert pair to dictionary and package (0, char) // 0 - MEANS NO PAIR
 *      if multi-char pattern is not in the dictionary
 *        find prior occurance of supset characters of pattern
 *        insert index of first found one with the next char
 *        insert pair (index, nextchar)
 *      if pattern exists
 *        get character or characters from the dictionary
 * } 
 *
 * Decode package using filled dictionary 
*/

#define INVALID_INDEX -1
#define MAX_MESSAGE_SIZE  1024

typedef unsigned char       u8; 
typedef unsigned short int u16; 

typedef enum {
  OK,
  ERR_NULL,
  ERR_MALLOC,
  ERR_REALLOC,
  ERR_READ,
  ERR_WRITE,
  ERR_BUFFEROVERFLOW
} Err;

typedef struct {
  u16 index;
  u16 *sequance;
} Pair;

typedef struct {
  size_t length ;
  Pair*  content;
} Dictionary;

Dictionary dict_init() {
  return (Dictionary) {
    .length = 0,
    .content = 0
  };
}


size_t seq_len(u16 *seq) {
  size_t len = 0;
  for(uint i = 0; seq[i] != 0; i++)
    len++;
  return len;
}


char seq_cmp(u16 *fir, u16 *sec) {
  const char EQUAL = 1;

  size_t length_1 = seq_len(fir);
  size_t length_2 = seq_len(sec);

  if (length_1 != length_2)
    return !EQUAL;

  for(uint i = 0; fir[i] != 0; i++)
    if (fir[i] != sec[i])
      return !EQUAL;

  return EQUAL;
}


char dict_find(Dictionary *dict, u16 *ch) {
  if (dict == 0)
    return ERR_NULL;

  int index = -1;

  // iter over dict
  for(uint i = 0; i < dict->length; i++)
    // if found character or sequance in the dictionary
    if(seq_cmp(dict->content[i].sequance,ch))
      index = dict->content[i].index;
  // return its index
  return index;
}


Err dict_append(Dictionary *dict, u16* sequance) {
  // if NULL - err
  if (dict == 0)
    return ERR_NULL;

  if (dict_find(dict, sequance) != INVALID_INDEX || sequance == NULL || sequance[0] == 0) 
    return ERR_WRITE;

  // if SMALLER THEN 1 - ALLOC
  if (dict->length < 1)
    dict->content = malloc(sizeof(Pair));
  else // if BIGGER THEN 1 - REALLOC 
    dict->content = reallocarray(dict->content, dict->length+1, sizeof(Pair));


  size_t length = dict->length;

  dict->content[length].index = length+1;
  dict->content[length].sequance = calloc(1 + seq_len(sequance), sizeof(u16));

  for(uint i = 0; i < seq_len(sequance); i++)
    dict->content[length].sequance[i] = sequance[i];

  dict->length++;

  return OK;
}


size_t dict_find_index(Dictionary *d, u16 *seq_src) 
{

  // [ #0 #1 |2]
  //   ~~~~~ -> len 2
  // [2 - 1] = 1
  // same sequence but -1 char of the end

  u16* seq = calloc(seq_len(seq_src) + 1,sizeof(u16));

  for(uint i = 0; i < seq_len(seq_src); i++)
    seq[i] = seq_src[i];

  if (seq_len(seq) > 1)
    seq[seq_len(seq)-1] = 0;

#ifdef DEBUG
  printf("\t\tsrc : { ");
  for(uint i = 0; i < seq_len(seq_src); i++)
    printf("%u ", seq_src[i]);
  printf("}");

  printf("\t edit : { ");
  for(uint i = 0; i < seq_len(seq); i++)
    printf("%u ", seq[i]);
  printf("}\n");
#endif

  for (uint i = 0; i < d->length; i++)
    if (seq_cmp(seq, d->content[i].sequance)) {
      free(seq);
      return d->content[i].index;
    }

  free(seq);
  return INVALID_INDEX;
}

size_t dict_index_of(Dictionary *d, u16 *seq_src) 
{

  // [ #0 #1 |2]
  //   ~~~~~ -> len 2
  // [2 - 1] = 1
  // same sequence but -1 char of the end

  if (seq_len(seq_src) == 1) {
    return 0;
  }
  u16* seq = calloc(seq_len(seq_src) + 1,sizeof(u16));

  for(uint i = 0; i < seq_len(seq_src); i++)
    seq[i] = seq_src[i];

  if (seq_len(seq) > 1)
    seq[seq_len(seq)-1] = 0;

#ifdef DEBUG
  printf("\t\tsrc : { ");
  for(uint i = 0; i < seq_len(seq_src); i++)
    printf("%u ", seq_src[i]);
  printf("}");

  printf("\t edit : { ");
  for(uint i = 0; i < seq_len(seq); i++)
    printf("%u ", seq[i]);
  printf("}\n");
#endif

  for (uint i = 0; i < d->length; i++)
    if (seq_cmp(seq, d->content[i].sequance)) {
      free(seq);
      return d->content[i].index;
    }

  free(seq);
  return INVALID_INDEX;
}

void dict_print(Dictionary d) {
  printf("\tDICTIONARY:\n");
  for(uint i = 0; i < d.length; i++)
  {
    printf("\t\tindex: %d\t",d.content[i].index);

    u16 *seq = d.content[i].sequance;
    printf("sequance: [ ");
    for(uint c = 0; seq[c] != 0; c++)
      printf("%d ",seq[c]);
    printf("]\n");
  }
}


void dict_free(Dictionary *d) {
  for(uint i = 0; i < d->length; i++)
    free(d->content[i].sequance);
  free(d->content);
}


Err pair_append(Pair* p, Pair pair, Dictionary *d) {
  size_t l = 0; 

  for(uint i = 0; p[i].sequance != 0; i++)
    l++;

  if (l >= MAX_MESSAGE_SIZE)
    return ERR_BUFFEROVERFLOW; 

  size_t index = dict_index_of(d,pair.sequance);

  u16 *single_seq = calloc(2,sizeof(u16));
  single_seq[0] = pair.sequance[seq_len(pair.sequance)-1];

  p[l] = (Pair) {
    .index = (index == INVALID_INDEX)? 0 : index,
    .sequance = single_seq//pair.sequance
  };

    return OK;
}

Pair* lz78_compress(uint *array, Dictionary *dict) {

  // const size_t MAX_MESSAGE_SIZE = 1024;

  static Pair            el [MAX_MESSAGE_SIZE] = {0};     
  size_t         len  = 1;
  uint         arrow  = 0;
  u16*      sequance  = calloc(1,sizeof(u16));
  // u16*     next       = calloc(1,sizeof(u16));
  size_t  array_size  = 0;

  for(uint i = 0; array[i] != 0; i++)
    array_size++;

  printf("\n\n");
  printf("\tBEGIN BUILDING Dictionary :: COMPRESSING\n\n\n");
  for (size_t i = 0; array[i] != 0; i++) {
    sequance = reallocarray(sequance,seq_len(sequance)+2,sizeof(u16));

    sequance[seq_len(sequance)+1] = 0;
    sequance[seq_len(sequance)] = array[i];

    // if (dict_find(dict,sequance)!=INVALID_INDEX) {
    //   // // if next(seq) doesnt exist, but previous does
    //   // u16* seq_copy = calloc(seq_len(sequance),sizeof(u16));
    //   // for(uint i = 0; i < seq_len(sequance); i++)
    //   //   seq_copy[i] = sequance[i];
    //   // seq_copy[seq_len(sequance)-1] = 0;
    //   //
    //   // if (dict_find(dict, seq_copy))
    //   //    pair_append(el,(Pair) { dict_find(dict,sequance), sequance }, dict);
    //   // free(next);
    //
    //   printf("NEXT Sequence: [");
    //     for(uint i= 0; sequance[i] != 0; i++)
    //       printf(" %u", sequance[i]);
    //     printf(" ]\n");
    // }

    if(dict_find(dict,sequance)==INVALID_INDEX)
    {

      // if next(seq) doesnt exist, but previous does
      // u16* seq_copy = calloc(seq_len(sequance),sizeof(u16));
      // for(uint i = 0; i < seq_len(sequance); i++)
      //   seq_copy[i] = sequance[i];
      // seq_copy[seq_len(sequance)-1] = 0;
      //
      // if (dict_find(dict, seq_copy))
      //    pair_append(el,(Pair) { dict_find(dict,sequance), sequance }, dict);
      // // free(next);
      //
      // if (i > array_size-1) {
      //   pair_append(el,(Pair) { dict_find(dict,sequance), sequance }, dict);
      //   break;
      // }
      
      dict_append(dict,sequance);
      Err result = 
        pair_append(el,(Pair) { dict_find(dict,sequance), sequance }, dict);

      // compressed sequence
      printf( "\t( %zu, %d )",
             dict_index_of(dict, sequance),
             sequance[seq_len(sequance)-1]
             );

      printf("\t\t::");

      // dictionary entry
      printf("\t\t[%i] { ", dict_find(dict,sequance) );
      for(uint i = 0; sequance[i] != 0; i++)
        printf("%u ",sequance[i]);
      printf("}\n");


      free(sequance); 
      //^^^^^^^^^^^^^^
      // value will be copied later in pair_append() function

      sequance = calloc(1,sizeof(u16));
    }

  }
  free(sequance);
  return el;
}



// TEMP
//p
u16* lz78_decompress(Pair *compressed_data, size_t compressed_size, size_t *decoded_size) {
    u16* decoded_data = calloc(MAX_MESSAGE_SIZE, sizeof(u16));
    size_t decoded_index = 0;
    Dictionary dict = dict_init();

    for (size_t i = 0; i < compressed_size; i++) {
        u16 index = compressed_data[i].index;
        u16 *sequence = compressed_data[i].sequance;

        if (index == 0) {
            // Case when the pair indicates a new character
            decoded_data[decoded_index++] = sequence[0];
            dict.content = realloc(dict.content, (dict.length + 1) * sizeof(Pair));
            dict.content[dict.length].index = dict.length + 1;
            dict.content[dict.length].sequance = calloc(2, sizeof(u16));
            dict.content[dict.length].sequance[0] = sequence[0];
            dict.content[dict.length].sequance[1] = 0;
            dict.length++;
        } else {
            // Case when the pair refers to an existing sequence in the dictionary
            Pair entry = dict.content[index - 1]; // Index is 1-based in the LZ-78 compression
            size_t seq_le = seq_len(entry.sequance);
            for (size_t j = 0; j < seq_le; j++) {
                decoded_data[decoded_index++] = entry.sequance[j];
            }
            decoded_data[decoded_index++] = sequence[0];
            dict.content = realloc(dict.content, (dict.length + 1) * sizeof(Pair));
            dict.content[dict.length].index = dict.length + 1;
            dict.content[dict.length].sequance = calloc(seq_le + 2, sizeof(u16));
            for (size_t j = 0; j < seq_le; j++) {
                dict.content[dict.length].sequance[j] = entry.sequance[j];
            }
            dict.content[dict.length].sequance[seq_le] = sequence[0];
            dict.content[dict.length].sequance[seq_le + 1] = 0;
            dict.length++;
        }
    }

    dict_free(&dict);
    *decoded_size = decoded_index;
    return decoded_data;
}



int main() {

  // uint *tseq = calloc(14, sizeof(uint));
  // tseq[0] = 1080; 
  // tseq[1] = 1052; 
  // tseq[2] = 32; 
  // tseq[3] = 1081; 
  // tseq[4] = 1052; 
  // tseq[5] = 52; 
  // tseq[6] = 1080; tseq[7] = 1052; tseq[8] = 32; tseq[9] = 1080; tseq[10] = 48; tseq[11] = 32; tseq[12] = 0;

   // printf("\n\n\n\n UNIQUE: %d\n\n\n",count_unique_items(tseq));
  
   const uint UA_ALPHABET_SIZE    = 32 + 6;
   const uint UA_UTF8_SIZE        = 11; // 6 + 5 bytes
   double entropy                  = 0;
   size_t bitwidht                = 0;
   size_t bitwidht_compressed     = 0;
   size_t bytes                   = 0;
   size_t bytes_compressed        = 0;
   int    string_character_count  = 0;
  
   char* str = load_string("file.txt");
    
   // transform to codepoints
   uint* seq = u8_as_codepoint_array(str);
   printf("\n\n\tString Original:\t`%s`\n\n",str);
  
   entropy = calculate_entropy(seq);
  
   // complexity
   printf(
       "\n\n\n\n"
       "+\t\t" "String information uncompressed:" "\n"
       "|\t"   "\n"
       "|\t"   "Number of UTF-8 characters  : %d"       "\n"
       "|\t"   "Byte`s taken to store data  : %ld"      "\n"
       "|\t"   "Bit length (bytes * 8)      : %ld"      "\n"
       "|\t"   "Words in string             : %d"       "\n"
       "|\t"   "Etropy for the string       : %.2f"     "\n"
       "|\t"   "Kraft's value (UA+SYM = 38) : %.13lf"   "\n"
       "|\t"   "Shannons value for UA_UTF8  : %.2f"     "\n"
       "+\n\n\n"
       , 
  
       (string_character_count = u8strlen(str)   ), 
       (bytes                  = strlen(str)     ), 
       (bitwidht               = strlen(str)*8   ), 
       u8_words(str), 
       entropy,
       krafts_value(UA_UTF8_SIZE,UA_ALPHABET_SIZE),
       shannons_value(entropy, UA_UTF8_SIZE)
   );
  
  printf("Sequence: [");
  for(uint i= 0; seq[i] != 0; i++)
    printf(" %u", seq[i]);
  printf(" ]\n");

 
  Dictionary d = dict_init();
  Pair *arr = lz78_compress(seq, &d); 

  size_t arr_size = 0;
  printf("DATA: \n");
  for(uint i = 0; arr[i].sequance != 0; i++)
  {
    if (i % 5 == 0)
      printf("\n");
    printf( "( %i, %d)\t",
           arr[i].index,
           // arr[i].sequance[seq_len(arr[i].sequance)-1]
           arr[i].sequance[0]
           );
    arr_size++;
  }

  // complexity
  // printf(
  //     "\n\n\n\n"
  //     "+\t\t" "String information compressed:" "\n"
  //     "|\t"   "\n"
  //     "|\t"   "Byte`s taken to store data  : %ld"      "\n"
  //     "|\t"   "Bit length (bytes * 8)      : %ld"      "\n"
  //     "|\t"   "Words in string             : %d"       "\n"
  //     "|\t"   "Etropy for the string       : %.2f"     "\n"
  //     "|\t"   "Kraft's value (UA+SYM = 38) : %.13lf"   "\n"
  //     "|\t"   "Shannons value for UA_UTF8  : %.2f"     "\n"
  //     "+\n\n\n"
  //     , 
  //
  //     (long)1312,
  //     (bitwidht               = strlen(str)*8   ), 
  //     u8_words(str), 
  //     entropy,
  //     krafts_value(UA_UTF8_SIZE,UA_ALPHABET_SIZE),
  //     shannons_value(entropy, UA_UTF8_SIZE)
  // );

  size_t decoded_size = 0;
  u16* back = lz78_decompress(arr, arr_size, &decoded_size);


  printf("Sequence: [");
  for(uint i= 0; back[i] != 0; i++)
    printf(" %u", back[i]);
  printf(" ]\n");


  for(uint i = 0; i < decoded_size; i++)
    seq[i] = (uint)back[i];

  char* strb = codepoint_array_as_u8str(seq);
  printf("\tStrin back from the ducks of hell: \n\t\t%s",strb);


  dict_free(&d);
  free(seq);
  free(strb);
  free(str);
  free(back);

}
