#include <math.h>
#include <algorithm>
#include <string.h>
#include <vector>

using namespace std;

struct elmt
{
    int char_order;
    int suffix_order;
    int pos;
};

bool compare(elmt a, elmt b)
{
    return a.char_order < b.char_order || a.char_order == b.char_order && (a.suffix_order < b.suffix_order || (a.suffix_order == b.suffix_order && a.pos < b.pos));
}

void sort_index(char *text, int textlen, int *S, elmt *V)
{
    sort(V, V + textlen, compare);

    int r = 0;
    S[V[0].pos] = r;
    for (int i = 1; i < textlen; ++i)
    {
        if (V[i].char_order != V[i - 1].char_order | V[i].suffix_order != V[i - 1].suffix_order)
            ++r;

        S[V[i].pos] = r;
    }
}

void build_P(char *text, int textlen, int lenlog2plus1, int *P)
{
    fill_n(P, textlen, -1);
    elmt *V = new elmt[textlen];
    for (int i = 0; i < textlen; ++i)
    {
        V[i].char_order = text[i];
        V[i].suffix_order = -1;
        V[i].pos = i;
    }
    sort_index(text, textlen, P, V);
    int *S = P;
    for (int k = 1; k < lenlog2plus1; ++k)
    {
        int j = 1 << k - 1;
        for (int i = 0; i < textlen; ++i)
        {
            V[i].char_order = S[i];
            if (textlen <= i + j)
                V[i].suffix_order = -1;
            else
                V[i].suffix_order = S[i + j];
        }
        for (int i = 0; i < textlen; ++i)
        {
            V[i].pos = i;
        }
        sort_index(text, textlen, S, V);
    }
    delete[] V;
}

void sa_invert(int *P, int textlen, int *sa)
{
    for (int i = 0; i < textlen; ++i)
    {
        sa[i] = -1;
    }
    for (int i = 0; i < textlen; ++i)
    {
        sa[P[i]] = i;
    }
}

void construct(char *text, int textlen, int *sa)
{
    int lenlog2plus1 = ceil(log2(textlen)) + 1;
    int *P = new int[textlen];
    build_P(text, textlen, lenlog2plus1, P);
    sa_invert(P, textlen, sa);
    delete[] P;
}

int succ(char *text, int textlen, char *pat, int patlen, int *sa)
{
    if (strncmp(text + sa[textlen - 1], pat, patlen) < 0)
        return textlen;
    else if (strncmp(pat, text + sa[0], patlen) <= 0)
        return 0;
    else
    {
        int r = textlen - 1;
        int l = 0;
        while (1 < r - l)
        {
            int h = (l + r) / 2;
            if (strncmp(text + sa[h], pat, patlen) >= 0)
                r = h;
            else
                l = h;
        }
        return r;
    }
}

int pred(char *text, int textlen, char *pat, int patlen, int *sa)
{
    if (strncmp(text + sa[textlen - 1], pat, patlen) <= 0)
        return textlen - 1;
    else if (strncmp(pat, text + sa[0], patlen) < 0)
        return -1;
    else
    {
        int r = textlen - 1;
        int l = 0;
        while (1 < r - l)
        {
            int h = (l + r) / 2;
            if (strncmp(text + sa[h], pat, patlen) <= 0)
                l = h;
            else
                r = h;
        }
        return l;
    }
}

void search(char *text, int textlen, char *pat, int patlen, int *sa, vector<int> *occ)
{
    int left = succ(text, textlen, pat, patlen, sa);
    int right = pred(text, textlen, pat, patlen, sa);
    for (int i = left; i <= right; ++i)
    {
        occ->push_back(sa[i]);
    }
}