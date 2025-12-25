#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include <stdio.h>

typedef uint8_t u8;
typedef uint16_t u16;
typedef uint32_t u32;
typedef uint64_t u64;

typedef int8_t i8;
typedef int16_t i16;
typedef int32_t i32;
typedef int64_t i64;

#define KiB (1 << 10)
#define MiB (1 << 20)
#define GiB (1 << 30)

//TODO the arena should use mmap
#define ARENA_MALLOC malloc
//see above TODO
#define ARENA_FREE free

struct Arena{
    u8* mem;
    u64 pos;
    u64 cap;
};

typedef struct Arena Arena;

//When creating an arena all of its cap is set to zero
Arena* ArenaCreate(u64 sz);
void ArenaDestroy(Arena* arena);

void* ArenaPush(Arena* arena, u64 sz);

#define ArenaPushArr(arena, type, cnt) (ArenaPush(arena, sizeof(type)*cnt))
#define ArenaPushStruct(arena, type) (ArenaPushArr(arena, type, 1))

i32 ArenaPop(Arena* arena, u64 sz);
#define ArenaPopArr(arena, type, cnt) (ArenaPop(arena, sizeof(type)*cnt))
#define ArenaPopStruct(arena, type) (ArenaPopArr(arena, type, 1))

Arena* ArenaCreate(u64 sz)
{
    Arena* arena;
    arena = ARENA_MALLOC(sz);
    if(arena == NULL){
        return NULL;
    }
    arena->mem = ARENA_MALLOC(sz);
    memset((void*)arena->mem, 0, sz);
    arena->cap = sz;
    arena->pos = 0;

    return arena;
}

void ArenaDestroy(Arena* arena)
{
    free(arena->mem);
    free(arena);
}

void* ArenaPush(Arena* arena, u64 sz)
{
    if((arena->pos + sz) > arena->cap){
        return NULL;
    }
    arena->pos = arena->pos + sz;
    u8* position = (arena->mem + arena->pos - sz);
    return position;
}

//returns 0 if trying to pop empty stack
i32 ArenaPop(Arena* arena, u64 sz)
{
    if((u64)(arena->pos - sz) < 0) return 0;
    printf("arena->pos = %ld, sz = %ld\n", arena->pos, sz);
    arena->pos = arena->pos - sz;
    return 1;
}

struct Foo{
    i32 x, y;
    i32* z;
};

typedef struct Foo Foo;

Foo* FooCreate(Arena* arena, i32 x, i32 y)
{
    printf("size of Foo = %ld\n", sizeof(Foo));
    //Foo is a pointer on the stack pointing to its position in the arena
    Foo* bar = ArenaPushStruct(arena, Foo);
    printf("bar address = %ld, mem adress = %ld, diff = %ld\n", (u64)bar, (u64)arena->mem, 
            (u64) bar - (u64) arena->mem);
    bar->x = x;
    bar->y = y;
    bar->z = ArenaPushArr(arena, i32, 100);
    bar->z[0] = 100;
    printf("accessing x from arena = %d\n", (i32) *(arena->mem));
    printf("accessing y from arena = %d\n", (i32) *(arena->mem + sizeof(int)));
    printf("accessing first z from arena = %d\n", (i32) *(arena->mem + sizeof(int)*2 + sizeof(size_t)));
    return bar;
}

int main(void)
{
    setbuf(stdout, NULL);
    Arena* arena = ArenaCreate(KiB);
    Foo* bar = FooCreate(arena, 5, 7);
    printf("bar(x, y) = %d, %d\n", bar->x, bar->y);
    printf("arena pos = %ld\n", arena->pos);
    ArenaPopArr(arena, i32, 100);
    printf("arena pos after pop arr = %ld\n", arena->pos);
    ArenaPopStruct(arena, Foo);
    printf("arena pos after pop = %ld\n", arena->pos);
    ArenaDestroy(arena);
    return 0;
}
