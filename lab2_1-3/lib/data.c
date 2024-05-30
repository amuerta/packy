#include <math.h>
#include <stdlib.h>

int count_unique_items(uint *arr) {


  size_t l = 0;
  for(uint i = 0; arr[i] != 0; i++)
    l++;
  
  int unique_count = 0;
  int seen[2048]; // max item size 

  for (int i = 0; arr[i] != 0; i++) {
    if (!seen[arr[i]]) { 
      seen[arr[i]] = 1; 
      unique_count++;   
    }
  }
  return unique_count;
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


double calculate_entropy(uint *arr) {
  int count = 0;
  int seen[2048] = {0}; // assuming only up to UA / RU chars in UTF8

  // Count total items and mark each item as seen
  for (uint i = 0; arr[i] != 0; i++) {
    count++;
    seen[arr[i]]++;
  }

  double entropy = 0;

  for (int i = 0; i < 2048; i++) {
    if (seen[i] > 0) {
      double probability = (double)seen[i] / count;
      entropy +=  probability * self_information(probability);
    }
  }
  return entropy;
}
