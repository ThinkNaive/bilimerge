#include "bilimerge.h"

void getnextval(uint8_t s[], int len, int nextval[])
{
    int i = 0;
    int j = -1;
    nextval[i] = j;

    while (i < len - 1)
        if (j == -1 || s[i] == s[j])
            if (s[++i] != s[++j])
                nextval[i] = j;
            else
                nextval[i] = nextval[j];
        else
            j = nextval[j];
}

int kmp(uint8_t s[], uint8_t t[], int len_s, int len_t)
{
    int i = 0;
    int j = 0;
    int nextval[len_t];
    getnextval(t, len_t, nextval);

    while (i < len_s && j < len_t)
        if (j == -1 || s[i] == t[j])
        {
            ++i;
            ++j;
        }
        else
            j = nextval[j];
    if (j >= len_t)
        return i - len_t;
    else
        return -1;
}