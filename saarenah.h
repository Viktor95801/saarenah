
/*
# The MIT License

Copyright (c) 2025 Viktor Hugo C.M.G.

Permission is hereby granted, free of charge, to any person obtaining a copy
    of this software and associated documentation files (the “Software”), to deal
    in the Software without restriction, including without limitation the rights
    to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
    copies of the Software, and to permit persons to whom the Software is
    furnished to do so, subject to the following conditions:
The above copyright notice and this permission notice shall be included in
    all copies or substantial portions of the Software.
THE SOFTWARE IS PROVIDED “AS IS”, WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
    IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
    FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
    AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
    LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
    OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
    THE SOFTWARE.
*/

#ifndef libSAARENAH_HeaderFile
#define libSAARENAH_HeaderFile

#include <assert.h>
#include <string.h>

#ifndef saMalloc
#include <stdlib.h>
// the malloc to use, defaults to stdlib malloc, but you can provide your own malloc function by doing #define saMalloc ... before including the header file, only matters on implementation file
#define saMalloc malloc
#endif // saAlloc
#ifndef saFree
#include <stdlib.h>
#define saFree free
#endif // saFree

#define saMax(a, b) ((a) > (b) ? (a) : (b))
#define saMin(a, b) ((a) < (b) ? (a) : (b))
#define saClamp(val, min, max) \
    saMin(saMax(val, min), max)

#define saMemAlign(size) (((size) + 7) & ~7)

// Normal arena type, not expandeable
typedef struct {
    unsigned char *mem;
    size_t size;
    size_t offset;
} sa_Arena;

// Growable arena type (pun intended)
typedef struct sa_GArena {
    unsigned char *mem;
    size_t size;
    size_t offset;
    // what makes this work
    struct sa_GArena *next;
} sa_GArena;

#ifdef __cplusplus
extern "C" {
#endif // __cplusplus

void *sa_arenaAlloc(sa_Arena *a, size_t size);
void *sa_arenaCAlloc(sa_Arena *a, size_t count, size_t size);
void *sa_arenaReAlloc(sa_Arena *a, const void *data, size_t size, size_t new_size);

void sa_arenaReset(sa_Arena *a);
void sa_arenaDestroy(sa_Arena *a); // since saArena is not supposed to be allocated on the heap, this just frees a->mem

// May get you a game company or a growable arena
sa_GArena *sa_garenaCreate(size_t size);
void *sa_garenaAlloc(sa_GArena *a, size_t size);
void *sa_garenaCAlloc(sa_GArena *a, size_t count, size_t size);
void *sa_garenaReAlloc(sa_GArena *a, const void *data, size_t size, size_t new_size);

void sa_garenaReset(sa_GArena *a);
void sa_garenaDestroy(sa_GArena *a);

#ifdef SAARENAH_IMPLEMENTATION
void *sa_arenaAlloc(sa_Arena *a, size_t size) {
    if(a == NULL)
        return NULL;
    size = saMemAlign(size);
    if(a->mem == NULL) {
        if(a->size != 0) {
            a->size = saMemAlign(a->size);
            a->mem = saMalloc(saMax(a->size, size));
            if(a->mem == NULL) 
                return NULL;
            a->offset = 0;
        } else 
            return NULL;
    }

    if(a->size < (a->offset + size))
        return NULL;
    
    void *result = a->mem + a->offset;
    a->offset += size;
    return result;
}
void *sa_arenaCAlloc(sa_Arena *a, size_t count, size_t size) {
    void *result = sa_arenaAlloc(a, count * size);
    if(result == NULL) 
        return NULL;
    memset(result, 0, count * size);
    return result;
}
void *sa_arenaReAlloc(sa_Arena *a, const void *data, size_t size, size_t new_size) {
    void *result = sa_arenaAlloc(a, new_size);
    if(result == NULL) 
        return NULL;
    memcpy(result, data, size);
    return result;
}

void sa_arenaReset(sa_Arena *a) {
    if(a == NULL)
        return;
    a->offset = 0;
}
void sa_arenaDestroy(sa_Arena *a) {
    if(a == NULL)
        return;
    a->offset = 0;
    a->size = 0;
    if(a->mem == NULL)
        return;
    saFree(a->mem);
    a->mem = NULL;
}

sa_GArena *sa_garenaCreate(size_t size) {
    sa_GArena *result = saMalloc(sizeof(sa_GArena));
    if(result == NULL)
        return NULL;
    
    result->offset = 0;
    result->next = NULL;
    
    result->size = saMemAlign(size);
    result->mem = saMalloc(saMemAlign(size));
    if(result->mem == NULL) {
        saFree(result);
        return NULL;
    }
    
    return result;
}
void *sa_garenaAlloc(sa_GArena *a, size_t size) {
    if(a == NULL)
        return NULL;
    size = saMemAlign(size);
    if(a->mem == NULL) 
        return NULL;
    
    while(a->size < (a->offset + size)) {
        if(a->next == NULL) {
            size_t new_size = saMax(a->size, size);
            a->next = sa_garenaCreate(new_size);
            if(a->next == NULL)
                return NULL;
            a = a->next;
            break;
        }
        a = a->next;
    }

    void *result = a->mem + a->offset;
    a->offset += size;
    return result;
}
void *sa_garenaCAlloc(sa_GArena *a, size_t count, size_t size)  {
    void *result = sa_garenaAlloc(a, count * size);
    if(result == NULL) 
        return NULL;
    memset(result, 0, count * size);
    return result;
}
void *sa_garenaReAlloc(sa_GArena *a, const void *data, size_t size, size_t new_size) {
    void *result = sa_garenaAlloc(a, new_size);
    if(result == NULL) 
        return NULL;
    memcpy(result, data, size);
    return result;
}

void sa_garenaReset(sa_GArena *a) {
    if(a == NULL)
        return;
    while(a) {
        a->offset = 0;
        a = a->next;
    }
}
void sa_garenaDestroy(sa_GArena *a) {
    if(a == NULL)
        return;
    sa_GArena *next;
    while(a) {
        a->offset = 0;
        a->size = 0;
        saFree(a->mem);
        next = a->next;
        saFree(a);
    }
}

#endif // SAARENAH_IMPLEMENTATION

#ifdef __cplusplus
}
#endif

#endif // libSAARENAH_HeaderFile