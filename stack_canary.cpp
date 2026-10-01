#define STACK_DEBUG 

#include "Stack_canary.h"
#include <stdio.h>
#include <assert.h>
#include <stdlib.h>

#define LOG_FILE "log.txt"

int main() {

    stack_t s1 = {};
    stack_t s2 = {};

    fclose(fopen(LOG_FILE, "w"));

    STACK_INIT(s1, 2);
    STACK_INIT(s2, 10);

    for (size_t i = 0; i < 8; i++) {
        StackPush(&s1, i * 2.5);
    }
    //s2.data[-1] = 52;

    StackPush(&s2, 42);
    StackPush(&s2, 137);

    StackElem_t val = 0;
    while (s1.size > 0) {
        StackPop(&s1, &val);
        printf("s1: %lg\n", val);
    }

    StackPop(&s2, &val);
    printf("s2: %lg\n", val);

    STACK_DESTROY(s1);
    STACK_DESTROY(s2);

    return 0;
}

/**
 * @brief initialize structure
 * 
 * @param[in, out] stk structure(stack). Uses dynamic memory, need to clean
 * @param[in] capacity size of stack
 * @return errors error code
 */
errors StackInit(stack_t* stk, int capacity ON_DBG(, const char* name, const char* file, int line)) {

    if (capacity <= 0) return ERR_WRONG_CAPACITY;

    assert(stk != NULL);
    stk->real_data = (StackElem_t*)calloc(capacity + 2, sizeof(StackElem_t));
    if (stk->real_data == NULL) {
        return ERR_NO_MEMORY;
    }
    stk->real_data[0] = canary;
    stk->data = stk->real_data + 1;
    for (size_t i = 0; i < capacity; i++) {
        stk->data[i] = POISON_DATA;
    }
    stk->real_data[capacity + 1] = canary;

    stk->capacity = capacity;
    stk->size = 0;

    #ifdef STACK_DEBUG
    stk->file = file;
    stk->name = name;
    stk->line = line;
    #endif 

    ASSERT_OK(stk);

    return ERR_OK;
}

/**
 * @brief cleans memory for structure and destroy it
 * 
 * @param[in, out] stk stack with parameters
 * @return errors error code
 */
errors StackDestroy(stack_t* stk) {

    ASSERT_OK(stk);

    free(stk->real_data);
    stk->real_data = NULL;
    stk->data = NULL;
    stk->size = POISON_SIZE;
    stk->capacity = POISON_SIZE;

    #ifdef STACK_DEBUG
    stk->name = NULL;
    stk->file = NULL;
    stk->line = POISON_SIZE;
    #endif

    return ERR_OK;
}

/**
 * @brief adds element to stack
 * 
 * @param[in, out] stk structure with the stack and its parameters
 * @param[in, out] value element that need to push
 * @return errors error code
 */
errors StackPush(stack_t* stk, StackElem_t value) {

    ASSERT_OK(stk);
    assert(!isnan(value));

    errors err = ERR_OK;

    if (stk->capacity == stk->size) {
        err = StackResize(stk, 2*stk->capacity + 1);
        if (err != ERR_OK) {
            return err;
        }
    }

    stk->data[stk->size++] = value;

    ASSERT_OK(stk);

    return err;
}

/**
 * @brief recieves the last elemeny from the stack
 * 
 * @param[in, out] stk structure with the stack and its parameters 
 * @param[in, out] x pointer on recieved element
 * @return errors error code
 */
errors StackPop(stack_t* stk, StackElem_t* x) {
    
    ASSERT_OK(stk);
    assert(x != NULL);

    errors err = ERR_OK;

    if (stk->size <= 0) {
        printf("Pop nothing\n");
        return ERR_WRONG_SIZE;
    }

    *x = stk->data[--stk->size];
    
    stk->data[stk->size] = POISON_DATA;

    
    if (stk->size < (stk->capacity) / 4 && stk->capacity > 4) {
        err = StackResize(stk, stk->capacity/2);
        if (err != ERR_OK) {
            return err;
        }
    }

    ASSERT_OK(stk);

    return err;
}

/**
 * @brief checks error stack structure
 * 
 * @param[in] stk structure with the stack and its parameters
 * @return errors error code
 */
errors StackError(stack_t* stk) {

    if (stk == NULL) {
        return ERR_NULL_PTR;
    }
    if (stk->canary_left != canary || stk->canary_right != canary) {
        return ERR_STK_CANARY;
    }
    if (stk->data == NULL) {
        return ERR_NULL_DATA;
    }
    if (stk->capacity < 0) {
        return ERR_WRONG_CAPACITY;
    }
    if (stk->size < 0) {
        return ERR_WRONG_SIZE;
    }
    if (stk->size > stk->capacity) {
        return ERR_OVERFLOW;
    }
    if (stk->real_data[0] != canary || stk->real_data[stk->capacity + 1] != canary) {
        return ERR_ARR_CANARY;
    }

    return ERR_OK;
}

/**
 * @brief prints dump with information about error
 * 
 * @param[in] stk structure with the stack and its parameters
 * @param[in] err_code error code
 * @param[in] file file with place with error
 * @param[in] line line with error
 * @param[in] func function with error
 */
void StackDump(const stack_t* stk,  errors err_code, const char* file, int line, const char* func) {

    if (stk == NULL) {
        printf("stack_t [NULL pointer]\n");
        assert(0);
    }

    PrintTop(stk, err_code, file, line, func);

    PrintBottom(stk);

    StackDump_file(stk, err_code, file, line, func);

}

/**
 * @brief resizes stack
 * 
 * @param[in, out] stk structure with the stack and its parameters
 * @param[in] new_capacity new size of stack
 * @return errors error code
 */
errors StackResize(stack_t* stk, size_t new_capacity) {

    ASSERT_OK(stk);    

    int old_capacity = stk->capacity;

    StackElem_t* temp = (StackElem_t*)realloc(stk->real_data, (new_capacity + 2)*sizeof(StackElem_t));
    if (temp == NULL) {
        return ERR_NO_MEMORY;
    }

    stk->real_data = temp;
    stk->real_data[0] = canary;
    stk->data = stk->real_data + 1;
    
    stk->capacity = new_capacity;

    for (size_t i = old_capacity; old_capacity < new_capacity && i < new_capacity; i++) {
        stk->data[i] = POISON_DATA;
    }
    stk->real_data[stk->capacity + 1] = canary;

    ASSERT_OK(stk);

    return ERR_OK;
}

/**
 * @brief gets name of error by error code
 * 
 * @param[in] err error code
 * @return const char* name of error
 */
const char* ErrCode(errors err) {

    switch(err) {
        case 1:
            return "ERR_NULL_PTR";
        case 2:
            return "ERR_NULL_DATA";
        case 3:
            return "ERR_WRONG_CAPACITY";
        case 4:
            return "ERR_WRONG_CAPACITY";
        case 5:
            return "ERR_OVER_FLOW";
        case 6:
            return "ERR_NO_MEMORY";
        case 7:
            return "ERR_ARR_CANARY";
        case 8:
            return "ERR_STK_CANARY";
        default:
            return "ERR_OK";
    }

    return "ERR_OK";
}

/**
 * @brief prints top of the dump
 * 
 * @param[in] stk structure with the stack and its parameters
 * @param[in] err_code code of error
 * @param[in] file file with an error
 * @param[in] line line with an error
 * @param[in] func function with an error
 */
void PrintTop(const stack_t* stk, errors err_code, const char* file, int line, const char*func) {

    for (size_t i = 0; i < 19; i++) printf("=");
    printf(" STACK DUMP ");
    for (size_t i = 0; i < 19; i++) printf("=");


    printf("\nError code: %d (%s)\n", err_code, ErrCode(err_code));

    printf("Called from: %s() at %s:%d\n", func, file, line);

    #ifdef STACK_DEBUG
    printf("\nstack_t \"%s\"[%p] Created at %s:%d\n", stk->name, stk, stk->file, stk->line);
    #endif
    printf("capacity = %d\n", stk->capacity);
    printf("size = %d\n", stk->size);         
    printf("data [%p]\n{\n", stk->data);
}

/**
 * @brief prints the bottom of the dump
 * 
 * @param[in] stk structure with stack and its parameteres
 */
void PrintBottom(const stack_t* stk) {
    if (stk->data == NULL || stk->capacity <= 0) {
        printf("[data unavailable]\n}\n");
        return;
    }
    for (size_t i = 0; i < stk->capacity; i++) {
        printf("   ");

        if (i < stk->size) {
            printf("*");
        } else {
            printf(" ");
        }

        printf("[%zu] = %lg", i, stk->data[i]);
        if (isnan(stk->data[i])) {
            printf(" (POISON)");
        }
        
        printf("\n");
    }

    printf("}\n");

    for (size_t i = 0; i < 50; i++) printf("=");
    printf("\n");
}

/**
 * @brief prints top of the dump into the file
 * 
 * @param[in] log_file pointer on file for dump
 * @param[in] stk structure with the stack and its parameters
 * @param[in] err_code code of error
 * @param[in] file file with an error
 * @param[in] line line with an error
 * @param[in] func function with an error
 */
void PrintTop_file (FILE* log_file, const stack_t* stk, errors err_code, const char* file, int line, const char*func)  {

    for (size_t i = 0; i < 19; i++) fprintf(log_file, "=");
    fprintf(log_file, " STACK DUMP ");
    for (size_t i = 0; i < 19; i++) fprintf(log_file, "=");


    fprintf(log_file, "\nError code: %d (%s)\n", err_code, ErrCode(err_code));

    fprintf(log_file, "Called from: %s() at %s:%d\n", func, file, line);

    #ifdef STACK_DEBUG
    fprintf(log_file, "\nstack_t \"%s\"[%p] Created at %s:%d\n", stk->name, stk, stk->file, stk->line);
    #endif
    fprintf(log_file, "capacity = %d\n", stk->capacity);
    fprintf(log_file, "size = %d\n", stk->size);         
    fprintf(log_file, "data [%p]\n{\n", stk->data);
}

/**
 * @brief prints the bottom of the dump
 * 
 * @param[in, out] log_file pointer to the file for dump
 * @param[in] stk structure with stack and its parameteres
 */
void PrintBottom_file(FILE* log_file, const stack_t* stk) {
    if (stk->data == NULL || stk->capacity <= 0) {
        fprintf(log_file, "[data unavailable]\n}\n");
        return;
    }
    for (size_t i = 0; i < stk->capacity; i++) {
        fprintf(log_file, "   ");

        if (i < stk->size) {
            fprintf(log_file, "*");
        } else {
            fprintf(log_file, " ");
        }

        fprintf(log_file, "[%zu] = %lg", i, stk->data[i]);
        if (isnan(stk->data[i])) {
            fprintf(log_file, " (POISON)");
        }
        
        fprintf(log_file, "\n");
    }

    fprintf(log_file, "}\n");

    for (size_t i = 0; i < 50; i++) fprintf(log_file, "=");
    fprintf(log_file, "\n");
}

/**
 * @brief prints dump into the file
 * 
 * @param[in] log_file pointer on file for dump
 * @param[in] stk structure with the stack and its parameters
 * @param[in] err_code code of error
 * @param[in] file file with an error
 * @param[in] line line with an error
 * @param[in] func function with an error
 */
void StackDump_file(const stack_t* stk,  errors err_code, const char* file, int line, const char* func) {
    FILE* log_file = fopen(LOG_FILE, "w");
    PrintTop_file(log_file, stk, err_code, file, line, func);
    PrintBottom_file(log_file, stk);
    fclose(log_file);
}