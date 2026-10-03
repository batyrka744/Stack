#define STACK_DEBUG 

#include "New_Stack_Canary.h"

#define LOG_FILE "log.txt"



int main(int argc, char* argv[]) {

    #ifdef STACK_DEBUG
        const char* log_file_name = LOG_FILE;

        if (argc < 2) 
            printf("Name of file for logging was not declarated so used default file\n");
        else
            log_file_name = argv[1];
        
        FILE* log = fopen(log_file_name, "w");
        my_assert(log != NULL, "Log-file was not opened\n", -1);
    #endif

    stack_t s1 = {};  //не могу так как канарейки тоже обнулятся
    stack_t s2 = {};

    STACK_INIT(s1, 2, log);
    STACK_INIT(s2, 10, log);

    for (size_t i = 0; i < 8; i++) {
        StackPush(&s1, i * 2.5, log);
    }
    s2.data[-1] = 52;

    StackPush(&s2, 42, log);
    StackPush(&s2, 137, log);

    StackElem_t val = 0;
    while (s1.size > 0) {
        StackPop(&s1, &val, log);
        printf("s1: %lg\n", val);
    }

    StackPop(&s2, &val, log);
    printf("s2: %lg\n", val);

    STACK_DESTROY(s1, log);
    STACK_DESTROY(s2, log);

    fclose(log);

    return 0;
}

/**
 * @brief initialize structure
 * 
 * @param[in, out] stk structure(stack). Uses dynamic memory, need to clean
 * @param[in] capacity size of stack
 * @param[in, out] log file for logging
 * @return errors error code
 */
errors StackInit(stack_t* stk, int capacity ON_DBG(, FILE* log, const char* name, const char* file, int line)) {

    my_assert(capacity > 0, "Bad capacity for initialization\n", ERR_WRONG_CAPACITY);
    assert(stk != NULL);

    StackElem_t* temp = (StackElem_t*)calloc(capacity + 2, sizeof(StackElem_t));
    my_assert(temp != NULL, "Bad calloc in initialization\n", ERR_NO_MEMORY);

    stk->data = temp + 1;

    stk->data[-1] = canary;

    for (size_t i = 0; i < capacity; i++) {
        stk->data[i] = POISON_DATA;
    }
    stk->data[capacity] = canary;

    stk->capacity = capacity;
    stk->size = 0;

    #ifdef STACK_DEBUG
    stk->file = file;
    stk->name = name;
    stk->line = line;
    #endif 

    ASSERT_OK(stk, log);

    return ERR_OK;
}

/**
 * @brief cleans memory for structure and destroy it
 * 
 * @param[in, out] stk stack with parameters
 * @param[in, out] log file for logging
 * @return errors error code
 */
errors StackDestroy(stack_t* stk ON_DBG(, FILE* log)) {

    ASSERT_OK(stk, log);

    free(stk->data - 1);
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
 * @param[in, out] log file for logging
 * @return errors error code
 */
errors StackPush(stack_t* stk, StackElem_t value ON_DBG(, FILE* log)) {

    my_assert(CheckInputDouble(value), "The element is not allowed in this type\n", ERR_INVALID_ELEM);

    ASSERT_OK(stk, log);
    assert(!isnan(value));

    errors err = ERR_OK;

    if (stk->capacity == stk->size) {
        err = StackResize(stk, MEMORY_UP*stk->capacity + 1, log);
        my_assert(err == ERR_OK, "Bad StackResize\n", err);
        
    }

    stk->data[stk->size++] = value;

    ASSERT_OK(stk, log);

    return err;
}

/**
 * @brief recieves the last elemeny from the stack
 * 
 * @param[in, out] stk structure with the stack and its parameters 
 * @param[in, out] x pointer on recieved element
 * @param[in, out] log file for logging
 * @return errors error code
 */
errors StackPop(stack_t* stk, StackElem_t* x, ON_DBG(FILE* log)) {
    
    ASSERT_OK(stk, log);
    assert(x != NULL);

    errors err = ERR_OK;

    my_assert(stk->size > 0, "Pop nothing\n", ERR_WRONG_SIZE);

    *x = stk->data[--stk->size];
    
    stk->data[stk->size] = POISON_DATA;

    
    if (stk->size < (stk->capacity) / MEMORY_DOWN && stk->capacity > MEMORY_DOWN) {
        err = StackResize(stk, stk->capacity / MEMORY_DOWN, log);
        my_assert(err == ERR_OK, "Bad StackResize\n", err);
    }

    ASSERT_OK(stk, log);

    return err;
}

/**
 * @brief checks error stack structure
 * 
 * @param[in] stk structure with the stack and its parameters
 * @return errors error code
 */
errors StackError(stack_t* stk) {

    if (stk == NULL) 
        return ERR_NULL_PTR;

    if (!ValidMemory((void*)stk))
        return ERR_INVALID_STRUCT_MEMORY;
    
    if (CheckCanaryStruct(stk) != ERR_OK) 
        return ERR_STK_CANARY;
    
    if (stk->data == NULL) 
        return ERR_NULL_DATA;
    if (!ValidMemory((void*)stk->data))
        return ERR_INVALID_DATA_MEMORY;
    
    if (stk->capacity < 0) 
        return ERR_WRONG_CAPACITY;
    
    if (stk->size < 0) 
        return ERR_WRONG_SIZE;
    
    if (stk->size > stk->capacity) 
        return ERR_OVERFLOW;
    
    if (CheckCanaryArr(stk) != ERR_OK) 
        return ERR_ARR_CANARY;
    
    
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
 * @param[in, out] log file for logging
 */
void StackDump(const stack_t* stk,  errors err_code, const char* file, int line, const char* func, FILE* log) {

    if (stk == NULL) {
        printf("stack_t [NULL pointer]\n");
        assert(0);
    }

    PrintTop(stk, err_code, file, line, func);

    PrintBottom(stk);

    StackDump_file(stk, err_code, file, line, func, log);
}

/**
 * @brief resizes stack
 * 
 * @param[in, out] stk structure with the stack and its parameters
 * @param[in] new_capacity new size of stack
 * @param[in, out] log file for logging
 * @return errors error code
 */
errors StackResize(stack_t* stk, size_t new_capacity ON_DBG(, FILE* log)) {

    ASSERT_OK(stk, log);    

    int old_capacity = stk->capacity;

    StackElem_t* temp = (StackElem_t*)realloc(stk->data - 1, (new_capacity + 2)*sizeof(StackElem_t));
    my_assert(temp != NULL, "Bad realloc in StackResize\n", ERR_NO_MEMORY);

    stk->data = temp + 1;
    stk->data[-1] = canary;
    
    stk->capacity = new_capacity;

    for (size_t i = old_capacity; old_capacity < new_capacity && i < new_capacity; i++) {
        stk->data[i] = POISON_DATA;
    }
    stk->data[stk->capacity] = canary;

    ASSERT_OK(stk, log);

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
        case ERR_NULL_PTR:
            return "ERR_NULL_PTR";
        case ERR_NULL_DATA:
            return "ERR_NULL_DATA";
        case ERR_WRONG_CAPACITY:
            return "ERR_WRONG_CAPACITY";
        case ERR_WRONG_SIZE:
            return "ERR_WRONG_SIZE";
        case ERR_OVERFLOW:
            return "ERR_OVERFLOW";
        case ERR_NO_MEMORY:
            return "ERR_NO_MEMORY";
        case ERR_ARR_CANARY:
            return "ERR_ARR_CANARY";
        case ERR_STK_CANARY:
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
    printf("canary_left = %p\n", stk->canary_left);
    printf("canary_right = %p\n", stk->canary_right);
    printf("data [%p]\n{\n", stk->data);
}

/**
 * @brief prints the bottom of the dump
 * 
 * @param[in] stk structure with stack and its parameteres
 */
void PrintBottom(const stack_t* stk) {
    if (stk->data != NULL || stk->capacity <= 0) {
        printf("[data unavailable]\n}\n");
        return;
    }

    for (size_t i = 0; i < stk->capacity; i++) {
        printf("   ");

        if (i < stk->size)
            printf("*");
        else
            printf(" ");
        

        printf("[%zu] = %lg", i, stk->data[i]);
        if (isnan(stk->data[i])) 
            printf(" (POISON)");
        
        
        
        printf("\n");

        if (i >= DUMP_NUM_ELEMS) 
            printf("... (Change #define DUMP_NUM_ELEMS to see more)\n");
            break;
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
    fprintf(log_file, "canary_left = %p\n", stk->canary_left);
    fprintf(log_file, "canary_right = %p\n", stk->canary_right); 
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
 * @param[in] stk structure with the stack and its parameters
 * @param[in] err_code code of error
 * @param[in] file file with an error
 * @param[in] line line with an error
 * @param[in] func function with an error
 * @param[in, out] log file for logging
 */
void StackDump_file(const stack_t* stk,  errors err_code, const char* file, int line, const char* func, FILE* log) {

    PrintTop_file(log, stk, err_code, file, line, func);
    PrintBottom_file(log, stk);
    fclose(log);
}

/**
 * @brief checks if pointer is valid
 * 
 * @param[in] elem checked element
 * @return int 1 if memory is valid else 0
 */
int ValidMemory(void* elem) {
    if (elem == NULL) 
        return 0;

    size_t el = (size_t)elem;
    if (el < 0x000000000000FFFF) 
        return 0;
    if (el > 0x0000800000000000 && el < 0xFFFF7FFFFFFFFFFF)
        return 0;
    if (el > 0XFFFF800000000000 && el < 0xFFFFFFFFFFFFFFFF)
        return 0;

    return 1;
}

/**
 * @brief check if double value is valid
 * 
 * @param[in] value checked value
 * @return int 1 if valid else 0
 */
int CheckInputDouble(double value) {
    if (value == 0)
        return 1;
    if (value > 1.7e+308 || value < 1.7e-308)
        return 0;
    
    return 1;
}

/**
 * @brief checks canaries on the structure
 * 
 * @param[in] stk 
 * @return errors code of error 
 */
errors CheckCanaryStruct(stack_t* stk) {
    #ifndef NO_CANARY
        if (stk->canary_left != canary || stk->canary_left != canary) {
            return ERR_STK_CANARY;
        }
    #endif
    return ERR_OK;
}

/**
 * @brief checks canaries on the stack
 * 
 * @param[in] stk 
 * @return errors error code
 */
errors CheckCanaryArr(stack_t* stk) {
    my_assert(stk != NULL, "NULL stack in CheckArrCanary", ERR_NULL_PTR);
    if (stk->data[-1] != canary || stk->data[stk->capacity] != canary) {
        return ERR_ARR_CANARY;
    }
    return ERR_OK;
}
