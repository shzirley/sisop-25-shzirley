#ifndef __STD_LIB_H__
#define __STD_LIB_H__

#include "std_type.h"

int div(int a, int b);
int mod(int a, int b);

void memcpy(byte* dst, byte* src, unsigned int size);
unsigned int strlen(char* str);
bool strcmp(char* str1, char* str2);
void strcpy(char* dst, char* src);
int strncmp(char* s1, char* s2, int n);
void clear(byte* buf, unsigned int size);
char* strchr(char* str, char c);

/**
 * TODO: Add your general helper function here
 * ...
 */

#endif // __STD_LIB_H__
