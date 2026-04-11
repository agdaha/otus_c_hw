#define _GNU_SOURCE
#include "records.h"
#include <stdio.h>
#include <string.h>

void load_records(Record *list)
{
    FILE *f = fopen("records.dat", "rb");
    if (!f)
    {
        for (int i = 0; i < MAX_RECORDS; i++)
        {
            strcpy(list[i].name, "---");
            list[i].score = 0;
        }
        return;
    }
    fread(list, sizeof(Record), MAX_RECORDS, f);
    fclose(f);
}

void save_records(Record *list)
{
    FILE *f = fopen("records.dat", "wb");
    if (f)
    {
        fwrite(list, sizeof(Record), MAX_RECORDS, f);
        fclose(f);
    }
}

void add_record(Record *list, const char *name, int score)
{
    for (int i = 0; i < MAX_RECORDS; i++)
    {
        if (score > list[i].score)
        {
            for (int j = MAX_RECORDS - 1; j > i; j--)
                list[j] = list[j - 1];
            //strncpy(list[i].name, name, MAX_NAME_LEN);
            strlcpy(list[i].name, name, sizeof(list[i].name));
            list[i].score = score;
            break;
        }
    }
    save_records(list);
}