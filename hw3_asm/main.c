// Type your code here, or load an example.
#include <stdio.h>
#include <stdlib.h>
//#include <string.h>


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

int p( long int x){
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

void m(struct node* list, void (*func)(long int)){
    if (!list) {
        return;
    }
    func(list->value);
    m(list->next, func);
}

struct node *f(struct node* list,struct node* new_list, int (*func)(long int)){
    if (!list) return new_list;
    
    if (func(list->value)) {
        return f(list->next, add_element(new_list, list->value ), func);
    } else {
        return f(list->next, new_list, func);
    }
}

int main(){
    struct node* list = NULL;
    for (size_t i = data_length; i > 0; i--) {
        list = add_element(list, data[i-1]);
    }
    m(list,print_int);
    puts(empty_str);

    struct node* new_list = f(list, NULL,p);
    m(new_list, print_int);

    puts(empty_str);

    return 0;
}