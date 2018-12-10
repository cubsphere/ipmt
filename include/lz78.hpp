#include <deque>
#include <string>

std::deque<char>* int_encode78(int x, const char* ab, const int base);
std::deque<char>* gprime(std::deque<char>* y,  const char* ab, const int ablen);
std::deque<char>* cw_encode(std::deque<char>* w, const char* ab, const int ablen);
std::deque<char>* encode(std::string const& txt, const char* ab, const int ablen);
std::deque<char>* decode(std::deque<char>* code, const char* ab, const int ablen);
int int_decode(const char* x, int xl, const char* ab, int base);
int idx(const char* s, int l, char f);
