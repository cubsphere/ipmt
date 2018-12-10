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

bool compare_lcp(elmt a, elmt b)
{
    return a.char_order < b.char_order
        || a.char_order == b.char_order && (a.suffix_order < b.suffix_order
        || (a.suffix_order == b.suffix_order && a.pos < b.pos));
}

int access(int *el, int y, int x, int textlen)
{
    return el[y * textlen + x];
}

void sort_index_lcp(char *text, int textlen, int *S, elmt *V)
{
    sort(V, V + textlen, compare_lcp);

    int r = 0;
    S[V[0].pos] = r;
    for (int i = 1; i < textlen; ++i)
    {
        if (V[i].char_order != V[i - 1].char_order | V[i].suffix_order != V[i - 1].suffix_order)
            ++r;

        S[V[i].pos] = r;
    }
}

void build_P_lcp(char *text, int textlen, int lenlog2plus1, int *P)
{
    fill_n(P, textlen, -1);
    elmt *V = new elmt[textlen];
    for (int i = 0; i < textlen; ++i)
    {
        V[i].char_order = text[i];
        V[i].suffix_order = -1;
        V[i].pos = i;
    }
    sort_index_lcp(text, textlen, P, V);
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
        S += textlen;
        sort_index_lcp(text, textlen, S, V);
    }
    delete[] V;
}

void sa_invert_lcp(int *P, int textlen, int *sa)
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

int lcp(int *P, int textlen, int lenlog2, int i, int j)
{
    if (i == j)
        return textlen - i;
    else
    {
        int l = 0;
        for (int q = lenlog2; (q >= 0) & (i < textlen) & (j < textlen); --q)
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

void fill_lrlcp(int *sa, int *P, int textlen, int lenlog2, int left, int right, int *L, int *R)
{
    if (right - left <= 1)
        return;

    int half = (left + right) / 2;
    L[half] = lcp(P, textlen, lenlog2, sa[left], sa[half]);
    R[half] = lcp(P, textlen, lenlog2, sa[right], sa[half]);
    fill_lrlcp(sa, P, textlen, lenlog2, left, half, L, R);
    fill_lrlcp(sa, P, textlen, lenlog2, half, right, L, R);
}

void lrlcp(int *sa, int *P, int textlen, int lenlog2plus1, int *L, int *R)
{
    for (int i = 0; i < textlen; ++i)
    {
        L[i] = 0;
        R[i] = 0;
    }
    fill_lrlcp(sa, P, textlen, lenlog2plus1 - 1, 0, textlen - 1, L, R);
}

void construct(char* text, int textlen, int* sa, int* L, int* R)
{
    int lenlog2plus1 = ceil(log2(textlen)) + 1;
    int *P = new int[textlen * lenlog2plus1];
    build_P_lcp(text, textlen, lenlog2plus1, P);
    sa_invert_lcp(P + textlen * (lenlog2plus1 - 1), textlen, sa);
    lrlcp(sa, P, textlen, lenlog2plus1, L, R);
    delete[] P;
}

int lcp_bf(char *str1, int str1len, char *str2, int str2len)
{
    int minnie = min(str1len, str2len);
    pair<char *, char *> pair = mismatch(str1, str1 + minnie, str2);
    return pair.first - str1;
}

int succ(char *text, int textlen, char *pat, int patlen, int *sa, int *L, int *R)
{
    if (strncmp(text + sa[textlen - 1], pat, patlen) < 0)
        return textlen;
    else if (strncmp(pat, text + sa[0], patlen) <= 0)
        return 0;
    else
    {
        int left = lcp_bf(text + sa[0], textlen - sa[0], pat, patlen);
        int right = lcp_bf(text + sa[textlen - 1], textlen - sa[textlen - 1], pat, patlen);
        int r = textlen - 1;
        int l = 0;
        while (r - l > 1)
        {
            int h = (l + r) / 2;
            if (left >= right)
            {
                if (L[h] > left)
                    l = h;
                else if (L[h] < left)
                {
                    r = h;
                    right = L[h];
                }
                else
                {
                    int half = left + lcp_bf(text + sa[h] + left, textlen - (sa[h] + left), pat + left, patlen - left);
                    if ((half < patlen) & (half < textlen - sa[h]) & (text[sa[h] + half] < pat[half]))
                    {
                        l = h;
                        left = half;
                    }
                    else
                    {
                        r = h;
                        right = half;
                    }
                }
            }
            else
            {
                if (R[h] > right)
                    r = h;
                else if (R[h] < right)
                {
                    l = h;
                    left = R[h];
                }
                else
                {
                    int half = right + lcp_bf(text + sa[h] + right, textlen - (sa[h] + right), pat + right, patlen - right);
                    if ((half < patlen) & (half < textlen - sa[h]) & (text[sa[h] + half] < pat[half]))
                    {
                        l = h;
                        left = half;
                    }
                    else
                    {
                        r = h;
                        right = half;
                    }
                }
            }
        }
        return r;
    }
}

int pred(char *text, int textlen, char *pat, int patlen, int *sa, int *L, int *R)
{
    if (strncmp(text + sa[textlen - 1], pat, patlen) <= 0)
        return textlen - 1;
    else if (strncmp(pat, text + sa[0], patlen) < 0)
        return -1;
    else
    {
        int left = lcp_bf(text + sa[0], textlen - sa[0], pat, patlen);
        int right = lcp_bf(text + sa[textlen - 1], textlen - sa[textlen - 1], pat, patlen);
        int r = textlen - 1;
        int l = 0;
        while (r - l > 1)
        {
            int h = (l + r) / 2;
            if (left >= right)
            {
                if (L[h] > left)
                    l = h;
                else if (L[h] < left)
                {
                    r = h;
                    right = L[h];
                }
                else
                {
                    int half = left + lcp_bf(text + sa[h] + left, textlen - (sa[h] + left), pat + left, patlen - left);
                    if ((half < patlen) & (half < textlen - sa[h]) & (text[sa[h] + half] > pat[half]))
                    {
                        r = h;
                        right = half;
                    }
                    else
                    {
                        l = h;
                        left = half;
                    }
                }
            }
            else
            {
                if (R[h] > right)
                    r = h;
                else if (R[h] < right)
                {
                    l = h;
                    left = R[h];
                }
                else
                {
                    int half = right + lcp_bf(text + sa[h] + right, textlen - (sa[h] + right), pat + right, patlen - right);
                    if ((half < patlen) & (half < textlen - sa[h]) & (text[sa[h] + half] > pat[half]))
                    {
                        r = h;
                        right = half;
                    }
                    else
                    {
                        l = h;
                        left = half;
                    }
                }
            }
        }
        return l;
    }
}

void search(char *text, int textlen, char *pat, int patlen, int *sa, int *L, int *R, vector<int> *occ)
{
    int left = succ(text, textlen, pat, patlen, sa, L, R);
    int right = pred(text, textlen, pat, patlen, sa, L, R);
    for(int i = left; i <= right; ++i)
    {
        occ->push_back(sa[i]);
    }
}