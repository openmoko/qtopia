int main( int , char ** ) {
    char *version = "unknown";
#if __GNUC__ >= 4
    version = "GCC VERSION 4";
#endif

    char *foo = new char[4];
    delete foo;

    return 0;
}

