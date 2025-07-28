#define SAARENAH_IMPLEMENTATION
#include "saarenah.h"

#include <stdio.h>
#include <assert.h>
#include <string.h>

void test_sa_arena_basic() {
    sa_Arena arena = {0};
    arena.size = 1024;

    void *ptr1 = sa_arenaAlloc(&arena, 100);
    assert(ptr1 != NULL);

    void *ptr2 = sa_arenaAlloc(&arena, 200);
    assert(ptr2 != NULL);
    assert((unsigned char *)ptr2 >= (unsigned char *)ptr1 + 100);

    void *ptr3 = sa_arenaCAlloc(&arena, 10, sizeof(int));
    assert(ptr3 != NULL);
    for (int i = 0; i < 10; i++) {
        assert(((int *)ptr3)[i] == 0);
    }

    // Test realloc
    void *ptr4 = sa_arenaReAlloc(&arena, ptr2, 200, 400);
    assert(ptr4 != NULL);
    assert(memcmp(ptr4, ptr2, 200) == 0);

    sa_arenaReset(&arena);
    assert(arena.offset == 0);

    void *ptr5 = sa_arenaAlloc(&arena, 512);
    assert(ptr5 == ptr1); // memory reused

    sa_arenaDestroy(&arena);
    assert(arena.mem == NULL);
    assert(arena.size == 0);
}

void test_sa_garena_basic() {
    sa_GArena *arena = sa_garenaCreate(64);
    assert(arena != NULL);

    void *a = sa_garenaAlloc(arena, 32);
    assert(a != NULL);

    void *b = sa_garenaAlloc(arena, 48); // Should trigger new block
    assert(b != NULL);
    assert(arena->next != NULL);

    void *c = sa_garenaCAlloc(arena, 10, sizeof(int));
    assert(c != NULL);
    for (int i = 0; i < 10; i++) {
        assert(((int *)c)[i] == 0);
    }

    void *d = sa_garenaReAlloc(arena, b, 48, 96);
    assert(d != NULL);
    assert(memcmp(d, b, 48) == 0);

    sa_garenaReset(arena);
    sa_GArena *cur = arena;
    while (cur) {
        assert(cur->offset == 0);
        cur = cur->next;
    }

    sa_garenaDestroy(arena);
}

void test_cpp_api() {
#ifdef __cplusplus
    sa::Arena arena(1024);
    void *a = arena.alloc(64);
    void *b = arena.realloc(a, 64, 128);
    assert(b != NULL);
    arena.reset();

    sa::GArena g(32);
    void *x = g.alloc(64);
    void *y = g.realloc(x, 64, 128);
    assert(y != NULL);
    g.reset();
#endif
}

int main() {
    test_sa_arena_basic();
    test_sa_garena_basic();
    test_cpp_api();

    printf("O All saarenah tests passed.\n");
    return 0;
}
