#ifndef RECORDS_H
#define RECORDS_H

#define MAX_RECORDS 10
#define MAX_NAME_LEN 20

typedef struct
{
    char name[MAX_NAME_LEN];
    int score;
} Record;

void load_records(Record *list);

void save_records(Record *list);

void add_record(Record *list, const char *name, int score);

#endif