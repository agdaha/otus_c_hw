#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include "charsets.h"

int main(int argc, char** argv)
{
    const UTF8Char *chartable = NULL;

    if(argc != 4)
    {
        printf("USAGE: %s <inputFile> <charSet> <outputFile> \n", argv[0]);
        printf("\tcharsets: CP-1251, KOI8-R, ISO-8859-5\n");
        return EXIT_FAILURE;
    }

    FILE* file = fopen(argv[1], "r");
    if(!file)
    {
        printf("Failed to open %s for reading!\n", argv[1]);
        return EXIT_FAILURE;
    }

    char *charset = argv[2];

    FILE* outFile = fopen(argv[3], "w");
    if(!outFile)
    {
        printf("Failed to open %s for writing!\n", argv[3]);
        return EXIT_FAILURE;
    }

    if (strcmp(charset, "CP-1251") == 0) {
        chartable = cp1251_to_utf8;
    } else if (strcmp(charset, "KOI8-R") == 0) {
        chartable = koi8r_to_utf8;
    } else if (strcmp(charset, "ISO-8859-5") == 0) {
        chartable = iso88595_to_utf8;
    } else {
        printf("Error: Unsupported charset '%s'\n", charset);
        printf("\t supported charsets: CP-1251, KOI8-R, ISO-8859-5\n");
        return EXIT_FAILURE;
    }

    int c;
    while ((c = fgetc(file)) != EOF) {
        
        if (c < 0x80) {
            if (fputc(c, outFile) == EOF) {
                printf("Error: Failed to write output: %s\n", strerror(errno));
                return EXIT_FAILURE;
            }
        } else {
            if (c < 0x80 || c > 0xFF) {
                printf("Error: Invalid byte 0x%02X \n",(unsigned char)c);
                return EXIT_FAILURE;
            }
            
            size_t index = c - 0x80;
            const UTF8Char *utf8_char = &chartable[index];
            if (utf8_char->length <= 0) {
                printf("Error: Undefined character 0x%02X\n", (unsigned char)c);
                return EXIT_FAILURE;
            }
            
            if (fwrite(utf8_char->bytes, 1, utf8_char->length, outFile) != (size_t)utf8_char->length) {
                printf("Error: Failed to write output: %s\n", strerror(errno));
                return EXIT_FAILURE;
            }
        }
    }

    fclose(file);
    fclose(outFile);
    return EXIT_SUCCESS;
}
