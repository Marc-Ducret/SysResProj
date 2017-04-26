int main() {
    for(unsigned char i = 0;; i++) *((unsigned short*)0x88000000 + i) = 0x0D00 + i;
}
