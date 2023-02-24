//
//  vector.h
//  sokol
//
//  Created by George Watson on 23/02/2023.
//

#ifndef vector_h
#define vector_h

#define vector__sbraw(a) ((int *)(void *)(a)-2)
#define vector__sbm(a) vector__sbraw(a)[0]
#define vector__sbn(a) vector__sbraw(a)[1]

#define vector__sbneedgrow(a, n) ((a) == 0 || vector__sbn(a) + (n) >= vector__sbm(a))
#define vector__sbmaybegrow(a, n) (vector__sbneedgrow(a, (n)) ? vector__sbgrow(a, n) : 0)
#define vector__sbgrow(a, n) (*((void **)&(a)) = VectorGrow((a), (n), sizeof(*(a))))

#define DestroyVector(a) ((a) ? free(vector__sbraw(a)), 0 : 0)
#define VectorAppend(a, v) (vector__sbmaybegrow(a, 1), (a)[vector__sbn(a)++] = (v))
#define VectorCount(a) ((a) ? vector__sbn(a) : 0)
#define VectorLast(a) ((a)[vector__sbn(a)-1])
#define VectorRemove(a, idx)      \
    do                            \
    {                             \
        (a)[idx] = VectorLast(a); \
        --vector__sbn(a);         \
    } while (0)

void *VectorGrow(void *arr, int increment, int itemsize);

#endif /* vector_h */
