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

void sort_index(char *text, long textlen, long *P, elmt *V)
{
    sort(V, V + textlen, compare);

    long r = 0;
    P[V[0].pos] = r;
    for (long i = 1; i < textlen; ++i)
    {
        if (V[i].char_order != V[i - 1].char_order || V[i].suffix_order != V[i - 1].suffix_order)
            ++r;

        P[V[i].pos] = r;
    }
}

void build_P(char *text, long textlen, long* P)
{
    fill_n(P, textlen, -1);
    long lenlog2plus1 = ceil(log2(textlen)) + 1;
    elmt *V = new elmt[textlen];
    for (long i = 0; i < textlen; ++i)
    {
        V[i].char_order = text[i];
        V[i].suffix_order = -1;
        V[i].pos = i;
    }
    sort_index(text, textlen, P, V);

    for (long k = 1; k < lenlog2plus1; ++k)
    {
        long j = 1 << k - 1;
        for (long i = 0; i < textlen; ++i)
        {
            V[i].char_order = access(P, 0, i, textlen);
            if (textlen <= i + j)
                V[i].suffix_order = -1;
            else
                V[i].suffix_order = access(P, 0, i + j, textlen);
        }
        for (long i = 0; i < textlen; ++i)
        {
            V[i].pos = i;
        }
        sort_index(text, textlen, P, V);
    }
    delete[] V;
}

int main()
{
    char *hello = (char *)"ababab";
    long* P = new long[6];
    build_P(hello, 6, P);
}