#ifndef SUFFIX_ARRAY_H
#define SUFFIX_ARRAY_H

#include <vector>

using namespace std;

void construct(char* text, long textlen, long* sa, long* L, long* R);
void search(char *text, long textlen, char *pat, long patlen, long *sa, long *L, long *R, vector<long> *occ);

#endif