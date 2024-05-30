#pragma once

#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <math.h>
#include "utf8.c"

#define iter(SIZE) for(uint i = 0; i<SIZE; i++)

typedef struct {
  uint  codepoint;
  float probability;
} Character;


typedef struct Node {
  enum { Left, Right , Top  } side           ;
  enum { Root, Branch, Leaf } type           ;
  Character*                  content        ;
  size_t                      content_size   ;
  struct Node*                next[2]        ;
} Node;


typedef struct {
  size_t     dictionary_size;
  Character* dictionary;
  Node       root;
} Tree;


// generic swap for numeric types
// void swap(float *xp, float *yp) 
// { 
//   float temp = *xp; 
//   *xp = *yp; 
//   *yp = temp; 
// } 

#define swap(A,B) _swap(&(A), &(B), sizeof(A)) 
void _swap(void * a, void * b, size_t len)
{
    unsigned char * p = a, * q = b, tmp;
    for (size_t i = 0; i != len; ++i)
    {
        tmp = p[i];
        p[i] = q[i];
        q[i] = tmp;
    }
}


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


void sort_dictionary(Character* arr, int n) 
{ 
    int i, j, min_idx; 
  
    for (i = 0; i < n-1; i++) 
    { 
        min_idx = i; 
        for (j = i+1; j < n; j++) 
          if (arr[j].probability < arr[min_idx].probability) 
            min_idx = j; 
  
        if(min_idx != i) 
          swap(arr[min_idx], arr[i]); 
    } 
} 


// BOILERPLATER
float self_information(float chance) {
  return ( -log2(chance) );
}

float shannons_value(float entropy, uint wordcount)
{
  return entropy / wordcount;
}

double krafts_value(uint symbols_count, uint alphabet_size)
{
  return pow(2, -symbols_count) * alphabet_size;
}



// parse string 
// break it down to codepoints
// find all individual one 
// get their probability
// create an array 
// return it
Character* create_dictionary(uint* codepoints, size_t* dictionary_size, float *entropy) 
{
  // count total amount
  uint codepoints_count = 0;
  for(uint i = 0; codepoints[i] != ARRAY_END; i++)
    codepoints_count++;

  // copy codepoints 
  uint* codepoints_buffer = malloc(sizeof(uint) * (codepoints_count+1) );
  for(uint i = 0; codepoints[i] != ARRAY_END; i++)
    codepoints_buffer[i] = codepoints[i];
  codepoints_buffer[codepoints_count] = ARRAY_END;

  // sort codepoints_buffer
  sort(codepoints_buffer, codepoints_count);


  // find max_count of codepoint
  uint cur_count = 0;
  uint max_count = 0;
  uint last_codepoint = 0;
  for(uint i = 0; codepoints_buffer[i] != 0; i++) {
    uint el = codepoints_buffer[i];

    if (cur_count > max_count)
      max_count = cur_count;

    if (el == last_codepoint)
      cur_count++;
    else {
      if (last_codepoint != 0) {
        // printf("\n\tcodepoint (%d) was found -> %d",last_codepoint,cur_count);
        float chance = (float)cur_count / (float)codepoints_count;
        (*entropy) +=  chance * self_information(chance);
      }
      last_codepoint = el;
      cur_count = 1;
    }
  }

  
  last_codepoint = 0;
  const uint EMPTY = 1;


  // create new Character entry for each unique character? lol
  //
  const size_t UNSET = 1;
  Character* dictionary = malloc(UNSET);

  for(uint i = 0; codepoints_buffer[i] != 0; i++) {
    uint el = codepoints_buffer[i];

    if (el == last_codepoint)
      cur_count++;
    else {
      
      if (last_codepoint != 0) {

        float probability =   (float)cur_count/(float)codepoints_count ;

        Character c = {last_codepoint, probability};
        (*dictionary_size)++;

        dictionary = realloc(dictionary, sizeof(Character) * (*dictionary_size) );
        dictionary[(*dictionary_size) - 1] = c;
        printf("\n\tcodepoint (%d) was found -> %d times",last_codepoint,cur_count);
      }
      
      last_codepoint = el;
      cur_count = 1;
    }
  }


  free(codepoints_buffer);
  return dictionary;
}

char* u8_encode_test(uint codepoint, uint nbyte) {

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


// Recursive function to find split_threshold based on probabilities
float split_index(Character* arr, int size) {
    float sum_left = 0.0f;
    float sum_right = 0.0f;
    int left_index = 0;
    int right_index = size - 1;
    float sum = 0;
    while (left_index <= right_index) {
        if (sum_left <= sum_right) {
            sum_left += arr[left_index].probability;
            left_index++;
        } else {
            sum_right += arr[right_index].probability;
            right_index--;
        }
    }

#ifdef DEBUG
    // Print the split
    printf("Left array:\n");
    for (int i = 0; i < left_index; i++) {
        printf("%.3f ", arr[i].probability);
    }
    printf("\nSum of left array: %.3f\n\n", sum_left);

    printf("Right array:\n");
    for (int i = right_index + 1; i < size; i++) {
        printf("%.3f ", arr[i].probability);
    }
    printf("\nSum of right array: %.3f\n", sum_right);
#endif

    for(uint i = 0; i < size; i++)
      if (sum == sum_left)
      {
        return i;
      }
      else 
        sum += arr[i].probability;
    return 0;
}


void build_tree(Node* branch) 
{

  Character *right_split, *left_split;
  uint split_i;
  float sum = 0;
  size_t left_size;
  size_t right_size;

  // find the center
  split_i= split_index(branch->content, branch->content_size);

  right_size = branch->content_size - split_i;
  left_size = branch->content_size - right_size;

  // Allocate splits

  right_split = malloc(sizeof(Character) * right_size);
  left_split  = malloc(sizeof(Character) * left_size );

  // set left
  for(uint i = 0; i < split_i; i++)
    // printf("child: %d,\tparent: %d\n",i,i);
    left_split[i] = branch->content[i];

  // set right
  for(uint i = split_i; i < branch->content_size; i++) 
    // printf("child: %d,\tparent: %d\n",i-split_i,i);
    right_split[i-split_i] = branch->content[i];


  if (branch->type != Leaf) {

    // Create Left node
    Node* left_node = malloc(sizeof(Node));
    left_node->content_size = left_size;
    left_node->content      = left_split;
    left_node->side         = Left;
    if (left_size == 1)
      left_node->type       = Leaf;
    else 
      left_node->type       = Branch;

    // Create Right node
    Node* right_node = malloc(sizeof(Node));
    right_node->content_size = right_size;
    right_node->content      = right_split;
    right_node->side         = Right;
    if (right_size == 1)
      right_node->type       = Leaf;
    else 
      right_node->type       = Branch;
  
    // Set Next
    branch->next[0] = left_node ;
    branch->next[1] = right_node;

    build_tree(branch->next[0]);
    build_tree(branch->next[1]);
  }
  else {
    free(left_split);
    free(right_split);
  }

  // printf("left_s: %ld\t right_s: %ld\t\n\n",left_size,right_size);
}


void free_tree(Node* branch) {
    if (branch == NULL) {
        return;
    }

    if (branch->type != Leaf) {
        free_tree(branch->next[0]); 
        free_tree(branch->next[1]); 
    }
    free(branch->content); // Free content array of the current node
    free(branch); // Free the current node itself
}

void encode_with_tree(Node* node,uint codepoint, uint* depth, char* bytes_storage) {
    char byte;

    if (node->type == Leaf)
        goto end;

    byte = 1;
    for(uint i = 0; i < node->next[0]->content_size; i++)
        if (codepoint == node->next[0]->content[i].codepoint) 
            if (node->next[0]->side == Left)
                byte = 0;
    sprintf(bytes_storage,"%s%d",bytes_storage,byte);
    (*depth)++;

    // printf("depth: %d\n",*depth);
    encode_with_tree(node->next[byte], codepoint, depth, bytes_storage);
end:;
}


int decode_with_tree(Node* n, char* sequence, uint current_pos) {
 
    int result;

    if (n->type == Leaf) {
 
#ifdef DEBUG
        printf("curr_pos: %s\t codepoints: [","-");
        for(uint i = 0; i < n->content_size;i++)
            printf("%d,",n->content[0].codepoint);
        printf("]\n");
#endif /* ifdef DEBUG */

        return n->content[0].codepoint;
    }
    
    char curr_pos = sequence[current_pos]-'0';

#ifdef DEBUG
    printf("curr_pos: %d\t codepoints: [",curr_pos);
    for(uint i = 0; i < n->content_size;i++)
        printf("%d,",n->content[i].codepoint);
    printf("]\n");
#endif /* ifdef DEBUG */

    result = decode_with_tree(n->next[curr_pos],sequence, current_pos+1);
    return result;
}




// uint u8_words(const char* str){
//   uint wc = 0;
//   uint seen_letter = 0;
//
//   for (uint i = 0; str[i] != 0; i++)
//   {
//     if (str[i] != ' ')
//       seen_letter = 1;
//
//
//     if (str[i] == ' ' 
//         || str[i] == '\n' 
//         || str[i] == '\t'
//         || str[i] == '\0'
//         && seen_letter
//        ) 
//     {
//       wc++;
//     }
//   }
//
//   if (seen_letter && wc >= 1)
//     wc ++;
//   else if (seen_letter)
//     wc ++;
//
//   return wc;
// }



size_t shannon_bits(char** compressed_sequence, size_t text_legnth)
{
    uint bits = 0;
    for (uint i = 0; i < text_legnth; i++)
        bits += strlen(compressed_sequence[i]);
    return bits;
}

