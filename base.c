#include "base.h"

#define ARENA_BASE_POS (sizeof(MEM_ARENA))
#define ARENA_ALIGN (sizeof(void*))


U32 plat_get_pagesize(void);
void* plat_mem_reserve(U64 size);
B32 plat_mem_commit(void* ptr, U64 size);
B32 plat_mem_decommit(void* ptr, U64 size);
B32 plat_mem_release(void* ptr, U64 size);


MEM_ARENA* mem_arena_create(U64 commit_size, U64 reserve_size){
    U32 pagesize = plat_get_pagesize();

    reserve_size = ALIGN_UP_POW2(reserve_size, pagesize);
    commit_size = ALIGN_UP_POW2(commit_size, pagesize);

    MEM_ARENA* arena = plat_mem_reserve(reserve_size);

    if (!plat_mem_commit(arena, commit_size)) {
        return nullptr;
    }

    arena->reserve_size = reserve_size;
    arena->commit_size = commit_size;
    arena->pos = ARENA_BASE_POS;
    arena->commit_pos = commit_size;
    
    return arena;

}
void mem_arena_destroy(MEM_ARENA* arena){
    plat_mem_release(arena, arena->reserve_size);
}

//code that do the allocation.
void* mem_arena_push(MEM_ARENA* arena, U64 size){
    U64 pos_aligned = ALIGN_UP_POW2(arena->pos, ARENA_ALIGN);
    U64 new_pos = pos_aligned + size;
    if (new_pos > arena->reserve_size) { return nullptr; }

    if (new_pos > arena->commit_pos) {
        U64 new_commit_pos = new_pos;
        new_commit_pos += arena->commit_size - 1;
        new_commit_pos -= new_commit_pos % arena->commit_size;
        new_commit_pos = MIN(new_commit_pos, arena->reserve_size);

        U8* mem = (U8*)arena + arena->commit_pos;
        U64 commit_size = new_commit_pos - arena->commit_pos;

        if (!plat_mem_commit(mem, commit_size)) {
            return nullptr;
        }

        arena->commit_pos = new_commit_pos;
    }

    arena->pos = new_pos;

    return (U8*)arena + pos_aligned;
}
void mem_arena_pop(MEM_ARENA* arena, U64 size){
    size = MIN(size, arena->pos - ARENA_ASE_POS);
    arena->ops -= size;
}
void mem_arena_pop_to(MEM_ARENA* arena, U64 pos){
    U64 size = pos < arena->pos ? arena->pos - pos : 0;
    arena_pop(size);
}


void mem_arena_clear(MEM_ARENA* arena){
    arena_pop_to(arena, ARENA_BASE_POS);
}


#define STRING8_LIT(s) (STRING8){(U8*)(s), sizeof((s)) - 1}
#define STRING8_FMT(s) (int)(s).length, (s).buffer

STRING8 string8_substring(STRING8 input, U64 start, U64 end){
    end = MIN(input.length, end);
    start = MIN(start, end);
    STRING8 out = {
        .buffer = input.buffer + start, 
        .length = end - start
    };
    return out;
}
STRING8 string8_concatenate(MEM_ARENA* arena, STRING8 first, STRING8 last){
    U8* buffer = mem_arena_push(arena, first.length + last.length);
    STRING8 out = {.buffer = nullptr, .length = 0};
    if(!buffer){
        return out;
    }
    
    for(U64 i = 0; i < first.length; i++){
        buffer[i] = first.buffer[i];
    }
    for(U64 i = 0; i < last.length; i++){
        buffer[i + first.length] = last.buffer[i];
    }
    
    out.buffer = buffer;
    out.length = first.length + last.length;
    return out;
}



//creates a temporary scope to store our log info.
LOG_CONTEXT* log_context_begin(){

    static __thread MEM_ARENA* _log_arena = mem_arena_create(4096, 1024*1024);

    U64 restore_pos = _log_arena->pos;
    LOG_CONTEXT* init = mem_arena_push(_log_arena, sizeof(LOG_CONTEXT));
    *init = {
        .mem_source = _log_arena,
        .arena_restore_pos = restore_pos,
        .info_message_count = 0,
        .info_message_list_head = nullptr,
        .info_message_list_tail = nullptr,
        .error_message = {
            .buffer = nullptr,
            .length = 0
        };
    };
    
    return init;
}

//deletes and frees all log information
void log_context_end(LOG_CONTEXT *context){
    mem_arena_pop_to(context->mem_source, context->arena_restore_pos);
}

//emit functions.
void log_emit_error(LOG_CONTEXT* context, STRING8 msg){
    if(msg.length == 0){
        return;
    }

    if(context->error_message.length != 0){
        return;
    }


    context->error_message.buffer = mem_arena_push(context->mem_source, msg.length);
    if(!context->error_message.buffer){
        return;
    }

    //copy string
    for(U64 i = 0; i < msg.length; i++){
        context->error_message.buffer[i] = msg.buffer[i];
    }

    context->error_message.length = msg.length;

}

void log_emit_info(LOG_CONTEXT* context, STRING8 msg){
    if(msg.length == 0){
        return;
    } 

    STRING8_NODE* out = mem_arena_push(context->mem_source, msg.length + sizeof(STRING8_NODE));  

    out->next = nullptr;
    out->length = msg.length;
    
    //strcpy
    for(U64 i = 0; i < msg.length; i++){
        out->buffer[i] = msg.buffer[i];
    }

    //Add to Linked list end:
    
    //empty list special case
    if(context->info_message_count == 0){
        context->info_message_list_tail = out;
        context->info_message_list_head = out; 
    } else {
        context->info_message_tail.next = out;
        context->info_message_tail = out;
    }
    context->info_message_count++;
    return;
}

//peek functions. 
STRING8 log_unwrap_error(LOG_CONTEXT* context){
    return context->error_message;
}

STRING8 log_recieve_info(LOG_CONTEXT* context){
    if(context->info_message_count == 0){
        return nullptr;
    }

    STRING8_NODE* head = context->info_message_list_head;
    context->info_message_list_head = context->info_message_list_head.next;

    context->info_message_count--;

    STRING8 out = {
        .buffer = head->buffer;
        .length = head->length;
    };

    return out;
}















//######################################################
//######################################################
//
// PLATFORM SPECIFIC CONDITIONAL COMPILE WRAPPERS
//
//######################################################
//######################################################













#if defined(_WIN32)

#include <windows.h>

U32 plat_get_pagesize(void) {
    SYSTEM_INFO sysinfo = { 0 };
    GetSystemInfo(&sysinfo);

    return sysinfo.dwPageSize;
}

void* plat_mem_reserve(U64 size) {
    return VirtualAlloc(NULL, size, MEM_RESERVE, PAGE_READWRITE);
}

B32 plat_mem_commit(void* ptr, U64 size) {
    void* ret = VirtualAlloc(ptr, size, MEM_COMMIT, PAGE_READWRITE);
    return ret != NULL;
}

B32 plat_mem_decommit(void* ptr, U64 size) {
    return VirtualFree(ptr, size, MEM_DECOMMIT);
}

B32 plat_mem_release(void* ptr, U64 size) {
    return VirtualFree(ptr, size, MEM_RELEASE);
}


#elif defined(__linux__)

#define _DEFAULT_SOURCE

#include <unistd.h>
#include <sys/mman.h>

U32 plat_get_pagesize(void) {
    return (U32)sysconf(_SC_PAGESIZE);
}

void* plat_mem_reserve(U64 size) {
    void* out = mmap(NULL, size, PROT_NONE, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
    if (out == MAP_FAILED) {
        return NULL;
    }
    return out;
}

B32 plat_mem_commit(void* ptr, U64 size) {
    I32 ret = mprotect(ptr, size, PROT_READ | PROT_WRITE);
    return ret == 0;
}

B32 plat_mem_decommit(void* ptr, U64 size) {
    I32 ret = mprotect(ptr, size, PROT_NONE);
    if (ret != 0) return false;
    ret = madvise(ptr, size, MADV_DONTNEED);
    return ret == 0;
}

B32 plat_mem_release(void* ptr, U64 size) {
    I32 ret = munmap(ptr, size);
    return ret == 0;
}
#endif


