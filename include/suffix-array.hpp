#ifndef SUFFIX_ARRAY_H
#define SUFFIX_ARRAY_H

#include <vector>

using namespace std;

void construct(char* text, int textlen, int* sa);
void search(char *text, int textlen, char *pat, int patlen, int *sa, vector<int> *occ);

#endif