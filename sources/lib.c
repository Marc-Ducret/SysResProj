void *memcpy(char *dst, char *src, int n) {
    char *p = dst;
    while(n--)
        *dst++ = *src++;
    return p;
}

int strEqual(char *strA, char *strB) {
    int i = 0;
    while(strA[i] == strB[i]) {
        if(strA[i] == 0) return 1;
        i++;
    }
    return 0;
}
