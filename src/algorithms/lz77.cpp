#include <map>     // map hehe
#include <utility> // pair
#include <cmath>
#include <vector>
#include <algorithm>
#include <lz77.hpp>

#include <iostream>
#include <fstream>

using namespace std;

void print_fsm(map<pair<int,char>,int> fsm, const char* ab, int ablen, int m) {
  for (int i = 0; i < ablen; i++) {
    cout.write(&ab[i], 1);
    cout << " : ";
    for (int j = 0; j < m; j++) {
      cout << fsm[pair<int,char>(j, ab[i])];
      if (j < m-1) cout << " ";
    }
    cout << endl;
  }
}

// n = len(txt), m = len(pat)
void prefix_match_bf(int *p, int *l, char* txt, int n, char* pat, int m) {
  *p = 0;
  *l = 0;
  int j;
  for (int i = 0; i < n; i++) {
    j = 0;
    while (j < m-1 && txt[i+j] == pat[j]) {
      j++;
    }
    if (j > (*l)) {
      *p = i;
      *l = j;
    }
  }
}

// m = len(pat), l = len(ab)
map<pair<int,char>,int> build_fsm(char* pat, int m, const char* ab, int l) {
  // chave eh uma tupla, valor eh int
  map<pair<int, char>, int> delta;
  for (int j = 0; j < l; j++) {
    delta[pair<int, char>(0, ab[j])] = 0;
  }
  delta[pair<int,char>(0, pat[0])] = 1;
  int brd = 0;
  for (int i = 1; i < m; i++) {
    for (int j = 0; j < l; j++) {
      delta[pair<int,char>(i, ab[j])] = delta[pair<int,char>(brd, ab[j])];
    }
    delta[pair<int,char>(i,pat[i])] = i+1;
    brd = delta[pair<int,char>(brd,pat[i])];
  }
  for (int j = 0; j < l; j++) {
    delta[pair<int,char>(m, ab[j])] = delta[pair<int,char>(brd, ab[j])];
  }
  return delta;
}

// n = len(window), m = len(pat), l = len(ab)
void prefix_match(int* pos, int* maxlen, char* window, int n, char* pat,int m, const char* ab, int l) {
  map<pair<int,char>,int> fsm = build_fsm(pat, m, ab, l);
  *maxlen = 0;
  int cur = 0;
  *pos = 0;
  int ls = n - m;
  for (int i = 0; i < n; i++) {
    cur = fsm[pair<int, char>(cur, window[i])];
    if ((cur > *maxlen) && (i-cur+1 < ls)) {
      *maxlen = cur;
      *pos = i-cur+1;
    }
  }
  *maxlen = min(m-1, *maxlen);
}

// base = len(ab)
char* int_encode(int x, int size, const char* ab, int base, int* cs) {
  int codesize = (int) ceil(log(size)/log(base));
  *cs = codesize;
  char* code = new char[codesize];
  int bit, i = codesize-1;
  while (x) {
    bit = x % base;
    code[i--] = ab[bit];
    x /= base;
  }
  for (; i >= 0; i--) {
    code[i] = ab[0];
  }
  return code;
}

// base = len(ab), xl = len(x)
int int_decode(const char* x, int xl, const char* ab, int base) {
  int power = 1, val = 0;
  for (int c = xl-1; c >=0; c--) {
    val = val + idx(ab, base, x[c]) * power;
    power *= base;
  }
  return val;
}

vector<char>* lz77_encode(const char* txt, int tl, int ls, int ll, const char* ab, int ablen) {
  int n = tl+ls;
  char* W = new char[n];
  int i = 0;
  for (; i < ls; i++) {
    W[i] = ab[0];
  }
  for (; i < n; i++) {
    W[i] = txt[i-ls];
  }
  int j = ls, p, l, cs;
  vector<char>* code = new vector<char>();;
  char* code_part;
  while (j < n) {
    prefix_match(&p, &l, &W[j-ls], min(n, j+ll) - (j-ls), &W[j], min(n,j+ll)-j, ab, ablen);
    code_part = int_encode(p, ls, ab, ablen, &cs);
    for (int i = 0; i < cs; i++) {
      code->push_back(code_part[i]);
    }
    delete [] code_part;
    code_part = int_encode(l, ll, ab, ablen, &cs);
    for (int i = 0; i < cs; i++) {
      code->push_back(code_part[i]);
    }
    delete [] code_part;
    code->push_back(W[j+l]);
    j += l+1;
  }
  delete[] W;
  return code;
}

vector<char>* lz77_decode(vector<char>* code, int ls, int ll, const char* ab, int ablen) {
  vector<char> txt;
  for (int i = 0; i < ls; i++) {
    txt.push_back(ab[0]);
  }
  int l = ablen;
  int bs = (int) ceil(log(ls)/log(l));
  int bl = (int) ceil(log(ll)/log(l));
  int j = 0;
  int sb_init = 0;
  int p;
  char c;
  int codelen = code->size();
  while (j < codelen) {
    p = int_decode(&(*code)[j], bs, ab, ablen);
    j += bs;
    l = int_decode(&(*code)[j], bl, ab, ablen);
    j += bl;
    c = (*code)[j];
    j += 1;
    for (int i = 0; i < l; i++) {
      txt.push_back(txt[sb_init+p+i]);
    }
    txt.push_back(c);
    sb_init += (l+1);
  }
  vector<char>* ret = new vector<char>(txt.begin()+ls, txt.end());
  return ret;
}

int idx(const char* s, int l, char f) {
  for (int i = 0; i < l; i++) {
    if (s[i] == f) return i;
  }
  return -1;
}
/*
int main() {
  ifstream ftxt("/home/pedro/around.txt");
  string content((istreambuf_iterator<char>(ftxt)),
		 (istreambuf_iterator<char>()));
  const char* ascab = "wadehnortuA\n\r,";
  int ls = 512, ll = 128;
  vector<char>* code = lz77_encode(&content[0], content.length(),ls,ll,ascab, 17);
  ofstream fzip;
  fzip.open("/home/pedro/Desktop/blabla");
  fzip.write(&(*code)[0], code->size());
  fzip.close();
  ftxt.close();
  int otxtlen;
  vector<char>* otxt = lz77_decode(code, ls, ll, ascab, 17);
  ofstream funzip;
  for (int i = 0; i < otxt->size(); i++) {
    cout << (*otxt)[i];
  }
  cout << endl;
  delete code;
  delete otxt;
  //  funzip.open("/home/pedro/desktop/blabla.txt");
  //  funzip.write(&otxt[0], otxt.size());
  return 0;
}
*/
