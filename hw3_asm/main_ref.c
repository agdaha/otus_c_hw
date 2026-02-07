// Type your code here, or load an example.
#include <stdio.h>
#include <stdlib.h>
#include <string.h>


long int data[]={4, 8, 15, 16, 23, 42};
const size_t data_length = sizeof(data)/sizeof(long int);
char *empty_str = "";

struct node {
    long int value;
    struct node *next;
};

void print_int(long int x){
    printf("%ld ", x);
    fflush(0); // не совсем понял для чего это надо
}

int is_odd( long int x){
    return x & 1;
}

struct node *add_element(struct node* n, long int v){
    struct node *ptr = (struct node*)malloc(sizeof(struct node));
    if (ptr==NULL) {
        abort();
    }
    ptr->value = v;
    ptr->next = n;
    return ptr;
}

void map(struct node* list, void (*func)(long int)){
    while(list){
        func(list->value);
        list = list->next;
    }
}

struct node *filter(struct node* list,int (*func)(long int)){
    struct node *new_list = NULL;
    while(list){
        if (func(list->value)){
            new_list=add_element(new_list,list->value);
        }
        list = list->next;
    }
    return new_list;
}

void clear(struct node* list){
    while(list){
        struct node *ptr=list;
        list = list->next;
        free(ptr);
    }
}

int main(){
    struct node* list = NULL;
    for (size_t i = data_length; i > 0; i--) {
        list = add_element(list, data[i-1]);
    }
    map(list,print_int);
    puts(empty_str);
    
    struct node* new_list = filter(list, is_odd);
    map(new_list, print_int);
    puts(empty_str);

    clear(list);
    clear(new_list);

    return 0;
}