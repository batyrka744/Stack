typedef double StackElem_t;
const StackElem_t canary = 0xEBADA1;

#include <math.h>
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>

#ifdef STACK_DEBUG
#define ON_DBG(...)  __VA_ARGS__
#else
#define ON_DBG(...)
#endif

#define POISON_DATA NAN
#define POISON_SIZE -1488

#define STACK_INIT(stk, capacity) StackInit(&(stk), (capacity) ON_DBG(, #stk, __FILE__, __LINE__))
#define STACK_DESTROY(stk) StackDestroy(&(stk))
#define ASSERT_OK(stk) do {\
    errors err = StackError(stk);\
    if (err != ERR_OK) {\
        StackDump(stk, err, __FILE__, __LINE__, __func__);\
        assert(0);}\
    } while(0)




enum errors {ERR_OK = 0, ERR_NULL_PTR = 1, ERR_NULL_DATA = 2,
             ERR_WRONG_CAPACITY = 3, ERR_WRONG_SIZE = 4,
             ERR_OVERFLOW = 5, ERR_NO_MEMORY = 6, ERR_ARR_CANARY = 7, ERR_STK_CANARY = 8};



struct stack_t
    {
    ON_DBG(StackElem_t canary_left = canary;)

    StackElem_t* real_data;
    StackElem_t* data;
    int size, capacity;
    ON_DBG(const char* name; const char* file; int line;)

    ON_DBG(StackElem_t canary_right = canary;)
    };

errors StackInit(stack_t* stk, int capacity ON_DBG(, const char* name, const char* file, int line));

errors StackDestroy(stack_t* stk);

errors StackPush(stack_t* stk, StackElem_t value);

errors StackPop(stack_t* stk, StackElem_t* x);

errors StackError(stack_t* stk);

void StackDump(const stack_t* stk,  errors err_code, const char* file, int line, const char* func);

errors StackResize(stack_t* stk, size_t new_capacity);

const char* ErrCode(errors err);

void PrintTop(const stack_t* stk, errors err_code, const char* file, int line, const char*func);

void PrintBottom(const stack_t* stk);

void PrintTop_file (FILE* log_file, const stack_t* stk, errors err_code, const char* file, int line, const char*func);

void PrintBottom_file(FILE* log_file, const stack_t* stk);

void StackDump_file(const stack_t* stk,  errors err_code, const char* file, int line, const char* func);