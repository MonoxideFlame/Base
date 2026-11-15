#ifndef base_h
#define base_h

#include <stdint.h>
#include <stdio.h>

enum BOOL = {false = 0, true = 1};
typedef int8_t I8;
typedef int16_t I16;
typedef int32_t I32;
typedef int64_t I64;
typedef uint8_t U8;
typedef uint16_t U16;
typedef uint32_t U32;
typedef uint64_t U64;
typedef float F32;
typedef double F64;
typedef I8 B8;
typedef I32 B32;

static I8 I8_MAX = (I8)0x7full;
static I16 I16_MAX = (I16)0x7fffull;
static I32 I32_MAX = (I32)0x7fffffffull;
static I64 I64_MAX = (I64)0x7fffffffffffffffull;

static I8 I8_MIN = (I8)0x80ull;
static I16 I16_MIN = (I16)0x8000ull;
static I32 I32_MIN = (I32)0x80000000ull;
static I64 I64_MIN = (I64)0x8000000000000000ull;

static U8 U8_MAX = (U8)0xffull;
static U16 U16_MAX = (U16)0xffffull;
static U32 U32_MAX = (U32)0xffffffffull;
static U64 U64_MAX = (U64)0xffffffffffffffffull;

static F32 F32_MACHINE_EPSILON = 1.1920929e-7f;
static F64 F64_MACHINE_EPSILON = 2.22044604925e-16;

#define nullptr ((void*)0ull)

#define KiB(n) ((U64)(n) << 10)
#define MiB(n) ((U64)(n) << 20)
#define GiB(n) ((U64)(n) << 30)

#define MIN(a, b) (((a) < (b)) ? (a) : (b))
#define MAX(a, b) (((a) > (b)) ? (a) : (b))
#define CLAMP(a,x,b) (((x)<(a))?(a):(((b)<(x))?(b):(x)))
#define ALIGN_UP_POW2(n, p) (((U64)(n) + ((U64)(p) - 1)) & (~((U64)(p) - 1)))



typedef struct {
    U64 reserve_size;
    U64 commit_size;

    U64 pos;
    U64 commit_pos;
} MEM_ARENA;



MEM_ARENA* mem_arena_create(U64 commit_size, U64 reserve_size);
void mem_arena_destroy(MEM_ARENA* arena);

//code that do the allocation.
void* mem_arena_push(MEM_ARENA* arena, U64 size);
void mem_arena_pop(MEM_ARENA* arena, U64 size);
void mem_arena_pop_to(MEM_ARENA* arena, U64 pos);


void mem_arena_clear(MEM_ARENA* arena);

typedef struct {
    U8* buffer;
    U64 length;
} STRING8;


#define STRING8_LIT(s) (STRING8){(U8*)(s), sizeof((s)) - 1}
#define STRING8_FMT(s) (int)(s).length, (s).buffer

STRING8 string8_substring(STRING8 input, U64 start, U64 end);
STRING8 string8_concatenate(MEM_ARENA* arena, STRING8 first, STRING8 last);

typedef struct STRING8_NODE{
    struct STRING8_NODE* next;
    U64 length;
    U8 buffer[];
} STRING8_NODE;

typedef struct {
    MEM_ARENA* mem_source;
    U64 arena_restore_pos;

    U64 info_message_count;
    STRING8_NODE* info_message_list_head;
    STRING8_NODE* info_message_list_tail;
    
    STRING8 error_message;

} LOG_CONTEXT;
//creates a temporary scope to store our log info.
LOG_CONTEXT* log_context_begin();
//deletes and frees all log information
void log_context_end(LOG_CONTEXT *context);

//emit functions.
void log_emit_error(LOG_CONTEXT* context, STRING8 msg);
void log_emit_info(LOG_CONTEXT* context, STRING8 msg);

//peek functions. 
STRING8 log_unwrap_error(LOG_CONTEXT* context);
STRING8 log_receive_info(LOG_CONTEXT* context);




#endif


