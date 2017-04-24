int main() {
    *((unsigned short*) 0xB800F) = 0x0C3A;
    for(;;) ;
}
