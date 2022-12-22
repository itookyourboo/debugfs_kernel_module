#include <stdio.h>

int main(int argc, char *argv[]) {
    char *path;
    char* vendor;
    char* device;
    if (argc < 3) {
        fprintf( stderr, "not enough args\n" );
        return -1;
    }
    path = argv[1];
    vendor = argv[2];
    device = argv[3];
    printf("path = %s, vendor = %s, device = %s\n", path, vendor, device);

    FILE *kmod_args = fopen("/sys/kernel/debug/kmod/kmod_args", "w");
    fprintf(kmod_args, "%s %s %s\n ", path, vendor, device);
    fclose(kmod_args);

    FILE *kmod_result = fopen("/sys/kernel/debug/kmod/kmod_result", "r");
    char c;
    while (fscanf(kmod_result, "%c", &c) != EOF) {
        printf("%c", c);
    }
    fclose(kmod_result);
    return 0;
}


