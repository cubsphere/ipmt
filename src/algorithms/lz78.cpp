#include "lz78.hpp"
#include <iostream>
#include <map>
#include <vector>
#include <deque>
#include <string>
#include <string_view>
#include <utility>
#include <cmath>

using namespace std;

class Dict
{
public:
  map<string, int> data;
  vector<string> datainv;
  Dict()
  {
    data[""] = 0;
    datainv.push_back("");
  };

  pair<int, int> index(string const &txt)
  {
    for (int i = txt.size() - 1; i >= 0; i--)
    {
      map<string, int>::iterator f;
      string substr_to_consider = txt.substr(0, i);
      f = data.find(substr_to_consider);
      if (f != data.end())
      {
        return pair<int, int>(data[substr_to_consider], i);
      }
    }
    return pair<int, int>(0, 0);
  }

  string fond(int i)
  {
    return datainv[i];
  }

  void add(string w)
  {
    data[w] = data.size();
    datainv.push_back(w);
  }

  operator string() const
  {
    string ret = "";
    for (int i = 0; i < datainv.size(); i++)
    {
      ret = ret + to_string(i) + ":" + datainv[i] + "\n";
    }
    return ret;
  }
};

// base = len(ab), xl = len(x)
int int_decode(const char *x, int xl, const char *ab, int base)
{
  int power = 1, val = 0;
  for (int c = xl - 1; c >= 0; c--)
  {
    val = val + idx(ab, base, x[c]) * power;
    power *= base;
  }
  return val;
}

int idx(const char *s, int l, char f)
{
  for (int i = 0; i < l; i++)
  {
    if (s[i] == f)
      return i;
  }
  return -1;
}

deque<char> *int_encode(int x, const char *ab, const int base)
{
  if (x == 0)
  {
    deque<char> *ret = new deque<char>(ab, ab + 1);
    return ret;
  }
  int codesize = (int)ceil(log(x) / log(base));
  int bit, i = codesize - 1;
  deque<char> *code = new deque<char>();
  while (x)
  {
    bit = x % base;
    code->push_front(ab[bit]);
    x /= base;
  }
  return code;
}

deque<char> *gprime(deque<char> *y, const char *ab, const int ablen)
{
  if (y->size() <= 1)
  {
    deque<char> *ret = new deque<char>();
    ret->insert(ret->begin(), y->begin(), y->end());
    delete y;
    return ret;
  }
  else
  {
    deque<char> *ret = gprime(int_encode(y->size() - 2, ab, ablen), ab, ablen);
    ret->insert(ret->end(), y->begin(), y->end());
    return ret;
  }
}

deque<char> *cw_encode(deque<char> *w, const char *ab, const int ablen)
{
  w->push_front(ab[1]);
  deque<char> *temp = gprime(w, ab, ablen);
  delete w;
  temp->push_back(ab[0]);
  return temp;
}

deque<char> *encode(string_view const &txt, const char *ab, const int ablen)
{
  deque<char> *code = new deque<char>();
  int n = txt.size();
  int i = 0;
  Dict D = Dict();
  while (i < n)
  {
    pair<int, int> jl = D.index(txt.substr(i, n - i));
    int j = get<0>(jl);
    int l = get<1>(jl);
    deque<char> *cj = int_encode(j, ab, ablen);
    cj = cw_encode(cj, ab, ablen);
    code->insert(code->end(), cj->begin(), cj->end());
    code->push_back(txt[i + l]);
    D.add(txt.substr(i, l + 1));
    i += l + 1;
    delete cj;
  }
  return code;
}

deque<char> *decode(deque<char> *code, const char *ab, const int ablen)
{
  Dict D = Dict();
  int i = 0;
  int n = code->size();
  deque<char> *txt = new deque<char>();
  while (i < n)
  {
    string w;
    w.insert(w.begin(), code->begin() + i, code->begin() + i + 1);
    int l = int_decode(&w[0], 1, ab, ablen);
    i += 1;
    while (1)
    {
      if ((*code)[i] == ab[0])
      {
        string dic_entry = D.fond(int_decode(&w[1], w.size() - 1, ab, ablen));
        txt->insert(txt->end(), dic_entry.begin(), dic_entry.end());
        i += 1;
        char c = (*code)[i];
        txt->push_back(c);
        i += 1;
        D.add(dic_entry + c);
        break;
      }
      w = "";
      for (int j = i; j < i + l + 2; j++)
      {
        w.push_back((*code)[j]);
      }
      i = i + l + 2;
      l = int_decode(&w[0], w.size(), ab, ablen);
    }
  }
  return txt;
}
/*
int main() {
  string txt = "aabcbcbcbacbabcbabccbabb";
  const char* ab = "abc";
  deque<char>* code = encode(txt, ab, 3);
  for (int i = 0; i < code->size(); i++) {
    cout << (*code)[i];
  }
  cout << endl;
  deque<char>* decoded = decode(code, ab, 3);
  for (int i = 0; i < decoded->size(); i++) {
    cout << (*decoded)[i];
  }
  cout << endl;
  delete decoded;
  delete code;
}
*/
