#include <math.h>
#include <algorithm>

using namespace std;

struct elmt
{
    long char_order;
    long suffix_order;
    long pos;
};

bool compare(elmt a, elmt b)
{
    return a.char_order < b.char_order
        || a.char_order == b.char_order && (a.suffix_order < b.suffix_order
        || (a.suffix_order == b.suffix_order && a.pos < b.pos));
}

long access(long *el, long y, long x, long textlen)
{
    return el[y * textlen + x];
}

void sort_index(char *text, long textlen, long *S, elmt *V)
{
    sort(V, V + textlen, compare);

    long r = 0;
    S[V[0].pos] = r;
    for (long i = 1; i < textlen; ++i)
    {
        if (V[i].char_order != V[i - 1].char_order | V[i].suffix_order != V[i - 1].suffix_order)
            ++r;

        S[V[i].pos] = r;
    }
}

void build_P(char *text, long textlen, long lenlog2plus1, long *P)
{
    fill_n(P, textlen, -1);
    elmt *V = new elmt[textlen];
    for (long i = 0; i < textlen; ++i)
    {
        V[i].char_order = text[i];
        V[i].suffix_order = -1;
        V[i].pos = i;
    }
    sort_index(text, textlen, P, V);
    long *S = P;
    for (long k = 1; k < lenlog2plus1; ++k)
    {
        long j = 1 << k - 1;
        for (long i = 0; i < textlen; ++i)
        {
            V[i].char_order = S[i];
            if (textlen <= i + j)
                V[i].suffix_order = -1;
            else
                V[i].suffix_order = S[i + j];
        }
        for (long i = 0; i < textlen; ++i)
        {
            V[i].pos = i;
        }
        S += textlen;
        sort_index(text, textlen, S, V);
    }
    delete[] V;
}

void sa_invert(long *P, long textlen, long *sa)
{
    for (long i = 0; i < textlen; ++i)
    {
        sa[i] = -1;
    }
    for (long i = 0; i < textlen; ++i)
    {
        sa[P[i]] = i;
    }
}

long lcp(long *P, long textlen, long lenlog2, long i, long j)
{
    if (i == j)
        return textlen - i;
    else
    {
        long l = 0;
        for (long q = lenlog2; (q >= 0) & (i < textlen) & (j < textlen); --q)
        {
            if (access(P, q, i, textlen) == access(P, q, j, textlen))
            {
                l += 1 << q;
                i += 1 << q;
                j += 1 << q;
            }
        }
        return l;
    }
}

void fill_lrlcp(long *sa, long *P, long textlen, long lenlog2, long left, long right, long *L, long *R)
{
    if (right - left <= 1)
        return;

    long half = (left + right) / 2;
    L[half] = lcp(P, textlen, lenlog2, sa[left], sa[half]);
    R[half] = lcp(P, textlen, lenlog2, sa[right], sa[half]);
    fill_lrlcp(sa, P, textlen, lenlog2, left, half, L, R);
    fill_lrlcp(sa, P, textlen, lenlog2, half, right, L, R);
}

void lrlcp(long *sa, long *P, long textlen, long lenlog2plus1, long *L, long *R)
{
    for (int i = 0; i < textlen; ++i)
    {
        L[i] = 0;
        R[i] = 0;
    }
    fill_lrlcp(sa, P, textlen, lenlog2plus1 - 1, 0, textlen - 1, L, R);
}

#include "string.h"

int main()
{
    char *hello = (char *)"ababacabacbabcc";
    long textlen = strlen(hello);
    long lenlog2plus1 = ceil(log2(textlen)) + 1;
    long *P = new long[textlen * lenlog2plus1];
    build_P(hello, textlen, lenlog2plus1, P);
    long *sa = new long[textlen];
    sa_invert(P + textlen * (lenlog2plus1 - 1), textlen, sa);
    long *L = new long[textlen];
    long *R = new long[textlen];
    lrlcp(sa, P, textlen, lenlog2plus1, L, R);

    delete[] P;
    delete[] sa;
    delete[] L;
    delete[] R;
}