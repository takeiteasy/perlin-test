//
//  filesystem.c
//  sokol
//
//  Created by George Watson on 23/02/2023.
//

#include "filesystem.h"

void *VectorGrow(void *arr, int increment, int itemsize) {
    int dbl_cur = arr ? 2 * vector__sbm(arr) : 0;
    int min_needed = VectorCount(arr) + increment;
    int m = dbl_cur > min_needed ? dbl_cur : min_needed;
    int *p = realloc(arr ? vector__sbraw(arr) : 0, itemsize * m + sizeof(int) * 2);
    if (p) {
        if (!arr)
            p[1] = 0;
        p[0] = m;
        return p + 2;
    } else {
#ifdef VECTOR_OUT_OF_MEMORY
        VECTOR_OUT_OF_MEMORY;
#endif
        return (void *)(2 * sizeof(int)); // try to force a NULL pointer exception later
    }
}

const char* FileExt(const char *path) {
    const char *dot = strrchr(path, '.');
    return !dot || dot == path ? NULL : dot + 1;
}

const char** FindFiles(const char *ext) {
#if defined(PLATFORM_WINDOWS)
    //! TODO: FindFiles() Windows
    return NULL;
#else
    const char **result = NULL;
    unsigned long extLength = strlen(ext);
    static const char *path = "assets";
    DIR *dir = opendir(path);
    struct dirent *d;
    while ((d = readdir(dir))) {
        if (d->d_type == DT_REG) {
            const char *newExt = FileExt(d->d_name);
            if (newExt && !strncmp(newExt, ext, extLength))
                VectorAppend(result, strdup(d->d_name));
        }
    }
    closedir(dir);
    return result;
#endif
}
