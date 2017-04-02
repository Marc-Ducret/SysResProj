void *memcpy(char *dst, char *src, int n) {
    char *p = dst;
    while(n--)
        *dst++ = *src++;
    return p;
}

void memset(char *dst, char src, int len) {
    // Copy len times src from the address dst.
    while (len>0) {
        *dst = src;
        dst++;
        len--;
    }
}

int strEqual(char *strA, char *strB) {
    int i = 0;
    while(strA[i] == strB[i]) {
        if(strA[i] == 0) return 1;
        i++;
    }
    return 0;
}
