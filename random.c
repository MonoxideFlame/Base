#ifndef random_h
#define random_h


#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <stdbool.h>

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
typedef I32 B32;
typedef I8 B8;

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



typedef struct {
    U64 state;
    U64 inc;
} PRNG_STATE;


static PRNG_STATE s_prng_state = {
    0x853c49e6748fea9bULL, 0xda3e39cb94b95bdbULL
};
// *Really* minimal PCG32 code / (c) 2014 M.E. O'Neill / pcg-random.org
// Licensed under Apache License 2.0 (NO WARRANTY, etc. see website)

U32 prng_rand_r(PRNG_STATE* rng) {
    U64 oldstate = rng->state;
    // Advance internal state
    rng->state = oldstate * 6364136223846793005ULL + (rng->inc);
    // Calculate output function (XSH RR), uses old state for max ILP
    U32 xorshifted = ((oldstate >> 18u) ^ oldstate) >> 27u;
    U32 rot = oldstate >> 59u;
    return (xorshifted >> rot) | (xorshifted << ((-rot) & 31));
}

void prng_seed_r(PRNG_STATE* rng, U64 init_state, U64 init_seq){
    rng->state = 0ULL;
    rng->inc = (init_seq << 1u) | 1u;
    prng_rand_r(rng);
    rng->state += init_state;
    prng_rand_r(rng);
}

void prng_seed(U64 init_state, U64 init_seq){
    prng_seed_r(&s_prng_state, init_state, init_seq);
}

U32 prng_rand(void){
    return prng_rand_r(&s_prng_state);
}



//returns a number in range [0, 1]
float prng_randf_r(PRNG_STATE* rng){
    return (prng_rand_r(rng)/((float)U32_MAX));
}


//returns approximate normal deviate. Done using central limit theorem on 12 uniformly random integers. The range is +/- 6
double prng_randd_r(PRNG_STATE* rng){
    U64 rand_int = prng_rand_r(rng);
    rand_int = rand_int << 32;
    rand_int |= prng_rand_r(rng);
    return (rand_int/((double)U64_MAX));
}


double prng_rand_norm_approx_r(PRNG_STATE* rng){
    I64 sum = 0;
    for(int i = 0 ; i < 12; i++){
        sum += prng_rand_r(rng);
    }
    sum -= (6ull * U32_MAX);
    return sum / ((double)U32_MAX);
}


int main(void){
    for(int i = 0; i < 10000000; i++){
        printf("%f\n", prng_rand_norm_approx_r(&s_prng_state));
    }
    return 0;
}
#endif
