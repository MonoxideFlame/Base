
#define ARENA
#define ARENA_SCRATCH
#define ERRORS
#define STRING

#include "base_test.h"


void write_log(LOG_CONTEXT* context, U8 code){
    log_emit_info(context, STRING8_LIT("Log emission"));
    log_emit_error(context, code, STRING8_LIT("WRITING ERROR aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaappppppppp"));
}

int main(void) {
    LOG_CONTEXT l = log_context_start();
    
    for(U32 i = 0; i < 5; i++){
        write_log(&l, i);
    }

    log_print_info(&l);

    printf("%.*s\n", STRING8_FMT(log_error_unwrap(&l)));

    log_context_end(&l);    
    return 0;
}
