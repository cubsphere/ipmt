#include <math.h>
#include <algorithm>
#include <string.h>
#include <vector>

using namespace std;

struct elmt
{
    uint_least32_t char_order;
    uint_least32_t suffix_order;
    uint_least32_t pos;
};

bool compare(elmt a, elmt b)
{
    return a.char_order < b.char_order
        || a.char_order == b.char_order && (a.suffix_order < b.suffix_order
        || (a.suffix_order == b.suffix_order && a.pos < b.pos));
}

uint_least32_t access(uint_least32_t *el, uint_least32_t y, uint_least32_t x, uint_least32_t textlen)
{
    return el[y * textlen + x];
}

void sort_index(char *text, uint_least32_t textlen, uint_least32_t *S, elmt *V)
{
    sort(V, V + textlen, compare);

    uint_least32_t r = 0;
    S[V[0].pos] = r;
    for (uint_least32_t i = 1; i < textlen; ++i)
    {
        if (V[i].char_order != V[i - 1].char_order | V[i].suffix_order != V[i - 1].suffix_order)
            ++r;

        S[V[i].pos] = r;
    }
}

void build_P(char *text, uint_least32_t textlen, uint_least32_t lenlog2plus1, uint_least32_t *P)
{
    fill_n(P, textlen, -1);
    elmt *V = new elmt[textlen];
    for (uint_least32_t i = 0; i < textlen; ++i)
    {
        V[i].char_order = text[i];
        V[i].suffix_order = -1;
        V[i].pos = i;
    }
    sort_index(text, textlen, P, V);
    uint_least32_t *S = P;
    for (uint_least32_t k = 1; k < lenlog2plus1; ++k)
    {
        uint_least32_t j = 1 << k - 1;
        for (uint_least32_t i = 0; i < textlen; ++i)
        {
            V[i].char_order = S[i];
            if (textlen <= i + j)
                V[i].suffix_order = -1;
            else
                V[i].suffix_order = S[i + j];
        }
        for (uint_least32_t i = 0; i < textlen; ++i)
        {
            V[i].pos = i;
        }
        S += textlen;
        sort_index(text, textlen, S, V);
    }
    delete[] V;
}

void sa_invert(uint_least32_t *P, uint_least32_t textlen, uint_least32_t *sa)
{
    for (uint_least32_t i = 0; i < textlen; ++i)
    {
        sa[i] = -1;
    }
    for (uint_least32_t i = 0; i < textlen; ++i)
    {
        sa[P[i]] = i;
    }
}

uint_least32_t lcp(uint_least32_t *P, uint_least32_t textlen, uint_least32_t lenlog2, uint_least32_t i, uint_least32_t j)
{
    if (i == j)
        return textlen - i;
    else
    {
        uint_least32_t l = 0;
        for (uint_least32_t q = lenlog2; (q >= 0) & (i < textlen) & (j < textlen); --q)
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

void fill_lrlcp(uint_least32_t *sa, uint_least32_t *P, uint_least32_t textlen, uint_least32_t lenlog2, uint_least32_t left, uint_least32_t right, uint_least32_t *L, uint_least32_t *R)
{
    if (right - left <= 1)
        return;

    uint_least32_t half = (left + right) / 2;
    L[half] = lcp(P, textlen, lenlog2, sa[left], sa[half]);
    R[half] = lcp(P, textlen, lenlog2, sa[right], sa[half]);
    fill_lrlcp(sa, P, textlen, lenlog2, left, half, L, R);
    fill_lrlcp(sa, P, textlen, lenlog2, half, right, L, R);
}

void lrlcp(uint_least32_t *sa, uint_least32_t *P, uint_least32_t textlen, uint_least32_t lenlog2plus1, uint_least32_t *L, uint_least32_t *R)
{
    for (int i = 0; i < textlen; ++i)
    {
        L[i] = 0;
        R[i] = 0;
    }
    fill_lrlcp(sa, P, textlen, lenlog2plus1 - 1, 0, textlen - 1, L, R);
}

void construct(char* text, uint_least32_t textlen, uint_least32_t* sa, uint_least32_t* L, uint_least32_t* R)
{
    uint_least32_t lenlog2plus1 = ceil(log2(textlen)) + 1;
    uint_least32_t *P = new uint_least32_t[textlen * lenlog2plus1];
    build_P(text, textlen, lenlog2plus1, P);
    sa_invert(P + textlen * (lenlog2plus1 - 1), textlen, sa);
    lrlcp(sa, P, textlen, lenlog2plus1, L, R);
    delete[] P;
}

uint_least32_t lcp_bf(char *str1, uint_least32_t str1len, char *str2, uint_least32_t str2len)
{
    uint_least32_t minnie = min(str1len, str2len);
    pair<char *, char *> pair = mismatch(str1, str1 + minnie, str2);
    return pair.first - str1;
}

uint_least32_t succ(char *text, uint_least32_t textlen, char *pat, uint_least32_t patlen, uint_least32_t *sa, uint_least32_t *L, uint_least32_t *R)
{
    if (strncmp(text + sa[textlen - 1], pat, patlen) < 0)
        return textlen;
    else if (strncmp(pat, text + sa[0], patlen) <= 0)
        return 0;
    else
    {
        uint_least32_t left = lcp_bf(text + sa[0], textlen - sa[0], pat, patlen);
        uint_least32_t right = lcp_bf(text + sa[textlen - 1], textlen - sa[textlen - 1], pat, patlen);
        uint_least32_t r = textlen - 1;
        uint_least32_t l = 0;
        while (r - l > 1)
        {
            uint_least32_t h = (l + r) / 2;
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
                    uint_least32_t half = left + lcp_bf(text + sa[h] + left, textlen - (sa[h] + left), pat + left, patlen - left);
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
                    uint_least32_t half = right + lcp_bf(text + sa[h] + right, textlen - (sa[h] + right), pat + right, patlen - right);
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

uint_least32_t pred(char *text, uint_least32_t textlen, char *pat, uint_least32_t patlen, uint_least32_t *sa, uint_least32_t *L, uint_least32_t *R)
{
    if (strncmp(text + sa[textlen - 1], pat, patlen) <= 0)
        return textlen - 1;
    else if (strncmp(pat, text + sa[0], patlen) < 0)
        return -1;
    else
    {
        uint_least32_t left = lcp_bf(text + sa[0], textlen - sa[0], pat, patlen);
        uint_least32_t right = lcp_bf(text + sa[textlen - 1], textlen - sa[textlen - 1], pat, patlen);
        uint_least32_t r = textlen - 1;
        uint_least32_t l = 0;
        while (r - l > 1)
        {
            uint_least32_t h = (l + r) / 2;
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
                    uint_least32_t half = left + lcp_bf(text + sa[h] + left, textlen - (sa[h] + left), pat + left, patlen - left);
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
                    uint_least32_t half = right + lcp_bf(text + sa[h] + right, textlen - (sa[h] + right), pat + right, patlen - right);
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

void search(char *text, uint_least32_t textlen, char *pat, uint_least32_t patlen, uint_least32_t *sa, uint_least32_t *L, uint_least32_t *R, vector<uint_least32_t> *occ)
{
    uint_least32_t left = succ(text, textlen, pat, patlen, sa, L, R);
    uint_least32_t right = pred(text, textlen, pat, patlen, sa, L, R);
    for(uint_least32_t i = left; i <= right; ++i)
    {
        occ->push_back(sa[i]);
    }
}