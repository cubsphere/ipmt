#ifndef LZ77_H
#define LZ77_H

#include <map>
#include <vector>
#include <utility>

void prefix_match_bf(int *p, int *l, char* txt, int n, char* pat, int m);
std::map<std::pair<int,char>,int> build_fsm(char* pat, int m, char* ab, int l);
void prefix_match(int* pos, int* maxlen, char* window, int n, char* pat,int m, char* ab, int l);
char* int_encode(int x, int size, char* ab, int base, int* cs);
int int_decode(char* x, int xl, char* ab, int base);
std::vector<char>* lz77_encode(const char* txt, int tl, int ls, int ll, char* ab, int ablen);
std::vector<char>* lz77_decode(std::vector<char>* code, int ls, int ll, char* ab, int ablen);
int idx(char* s, int l, char f);

#endif
