typedef double StackElem_t;
const StackElem_t canary = 0xEBADA1;

#include <math.h>
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>


#define BLACK "\x1b[30m"
#define RED "\x1b[31m"
#define GREEN "\x1b[32m"
#define YELLOW "\x1b[33m"
#define BLUE "\x1b[34m"
#define MAGENTA "\x1b[35m"
#define CYAN "\x1b[36m"
#define WHITE "\x1b[37m"
#define RESET "\x1b[0m"


#define my_assert(exp, wrong, ret) if(!(exp)) {\
    printf(YELLOW "%s:%d: [Assertation failed: %s, file %s, line %d]\n" RESET, __FILE__, __LINE__, wrong,  __FILE__, __LINE__);\
    return ret;} 


#define DUMP_NUM_ELEMS 100
#define MEMORY_UP 2
#define MEMORY_DOWN 4


#ifdef STACK_DEBUG
#define ON_DBG(...)  __VA_ARGS__
#else
#define ON_DBG(...)
#endif

#define POISON_DATA NAN
#define POISON_SIZE -1488

#define STACK_INIT(stk, capacity, log) StackInit(&(stk), (capacity) ON_DBG(, (log), #stk, __FILE__, __LINE__))
#define STACK_DESTROY(stk, log) StackDestroy(&(stk), (log))

#ifdef STACK_DEBUG
    #define ASSERT_OK(stk, log) do {\
        errors err = StackError(stk);\
        if (err != ERR_OK) {\
            StackDump(stk, err, __FILE__, __LINE__, __func__, log);\
            assert(0);}\
        } while(0) //без do никак, при подстановке будет фигня
#else
    #define ASSERT_OK(stk);
#endif


enum errors {ERR_OK = 0,
             ERR_NULL_PTR = 1, 
             ERR_NULL_DATA = 2,
             ERR_WRONG_CAPACITY = 3, 
             ERR_WRONG_SIZE = 4,
             ERR_OVERFLOW = 5, 
             ERR_NO_MEMORY = 6, 
             ERR_ARR_CANARY = 7, 
             ERR_STK_CANARY = 8,
             ERR_INVALID_STRUCT_MEMORY = 9,
             ERR_INVALID_DATA_MEMORY = 10,
             ERR_INVALID_ELEM = 11};



struct stack_t
    {
    #ifndef NO_CANARY
        StackElem_t canary_left = canary;
    #endif

    StackElem_t* real_data;
    StackElem_t* data;
    int size, capacity;
    const char* name; const char* file; int line;

    #ifndef NO_CANARY
        StackElem_t canary_right = canary;
    #endif
    };

errors StackInit(stack_t* stk, int capacity ON_DBG(, FILE* log, const char* name, const char* file, int line));

errors StackDestroy(stack_t* stk ON_DBG(, FILE* log));

errors StackPush(stack_t* stk, StackElem_t value ON_DBG(, FILE* log));

errors StackPop(stack_t* stk, StackElem_t* x, ON_DBG(FILE* log));

errors StackError(stack_t* stk);

void StackDump(const stack_t* stk,  errors err_code, const char* file, int line, const char* func, FILE* log);

errors StackResize(stack_t* stk, size_t new_capacity ON_DBG(, FILE* log));

const char* ErrCode(errors err);

void PrintTop(const stack_t* stk, errors err_code, const char* file, int line, const char*func);

void PrintBottom(const stack_t* stk);

void PrintTop_file (FILE* log_file, const stack_t* stk, errors err_code, const char* file, int line, const char*func);

void PrintBottom_file(FILE* log_file, const stack_t* stk);

void StackDump_file(const stack_t* stk,  errors err_code, const char* file, int line, const char* func, FILE* log);

int ValidMemory(void* elem);

int CheckInputDouble(double value);

errors CheckCanaryStruct(stack_t* stk);

errors CheckCanaryArr(stack_t* stk);
