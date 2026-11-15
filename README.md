# Why a Base?


I use this as a minimal utility library. I wish to make this independent of the C standard library one day.

Right now, this implements basic allocators, some minimal logging, and a few machine related constants and a PCG random number generator. 


# Why use a Custom Allocator?

Malloc is HUGE. And it is SLOW. 

The reason why this is is because we expect the function to do too much. We expect it to just give us as much memory, all contiguous, whenever we want it. 

We give it back in the worst way possible too, as a fragmented list, in small pieces. It also has no transparency. Its easy to free the same pointer twice, and there is no enforement of a lifetime on the objects in question.

Relaxing some of these requirements on the API is enough to massively improve the performance of allocation, at the cost of enforcing a lifetime on our allocations. We can no longer allocate and deallocate in whatever order. 

# What allocator should we use?

The simplest allocator is a Linear Allocator, or an Arena. Its a block of memory with a pointer. We increment the pointer whenever we allocate. Then we free/clear the whole block at once. 
This is useful because it lets us re-use memory.

In a game, each frame has some data that needs to be allocated and rewritten all the time. Its easy to then reset the pointer to 0 when a frame ends,
after which, we start allocating again. We get good cache locality and everything remains very fast. It has use in all kinds of places. 

I also built this on top of mmap(), so I could use the existing virtual memory space to grow these arenas dynamically. 

It is a great way to allocate variables with the same lifetime to be freed all at once. Say we incrementally make graph of an NFA that needs to be freed. Instead of doing a Depth-First-Search and freeing
each node, we just allocate all the nodes to the same memory bucket (Arena), and then just free them all at once. It minimizes memory leak possibilities to a few large buckets containing most allocations, 
while keeping things fast. 

Pool allocators also work very well. However, they have a fixed size. So these are used in very specific situations where the order of allocation and deallocation is unknown, but the size is fixed. 
Malloc still works for everything else, though I only use it sparingly. This system helps me completely eliminate memory leaks. They are very easy to build on top of the Arena API


# Strings


C-strings are a pain. They are also a massive security vulnerability. So I have included a count based string API. 


# Logging & Errors:


These are hard to implement, because it requires calling a syscall. This slows us down. So I have both a memory bounded Error logging system (accumulates the first error, and does not accumumate future errors). 

It also has an unbounded Error (MESSAGE), where it just keeps appending messages onto the end of the log. We simply pass a log_context to a function that uses it. It plays well with function composition and is minimal. It works in an almost Monadic way (as monadic as you can get in C), where we can create a function called runWithLogs, which will allow us to run it with a log_context, with different composition rules for ERRORS and MESSAGES.


# Random Numbers:

Permutation Congruential Generators. [https://www.pcg-random.org/index.html]

These are not crytpographically secure, please do not use it for that purpose.





