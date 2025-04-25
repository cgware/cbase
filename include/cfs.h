#ifndef CFS_H
#define CFS_H

#include "cerr.h"

#include <stddef.h>

cerr_t cfs_open(const char *path, const char *mode, void **file);
cerr_t cfs_close(void *file);

cerr_t cfs_write(void *file, const void *data, size_t size);
cerr_t cfs_read(void *file, void *data, size_t size);

cerr_t cfs_du(void *file, size_t *size);

int cfs_isdir(const char *path);
int cfs_isfile(const char *path);

cerr_t cfs_mkdir(const char *path);
cerr_t cfs_mkfile(const char *path);

cerr_t cfs_rmdir(const char *path);
cerr_t cfs_rmfile(const char *path);

cerr_t cfs_getcwd(char *path, size_t size);

cerr_t cfs_lsdir_begin(const char *path, void **it, char *name, size_t size);
cerr_t cfs_lsdir_next(void **it, char *name, size_t size);
int cfs_lsdir_end(void **it);

cerr_t cfs_lsfile_begin(const char *path, void **it, char *name, size_t size);
cerr_t cfs_lsfile_next(void **it, char *name, size_t size);
int cfs_lsfile_end(void **it);

#define cfs_lsdir_foreach(_i, _dir, _it, _name, _size)                                                                                     \
	for (_i = cfs_lsdir_begin(_dir, _it, _name, _size); _i == CERR_OK ? 1 : (cfs_lsdir_end(&it), 0);                                   \
	     _i = cfs_lsdir_next(_it, _name, _size))

#define cfs_lsfile_foreach(_i, _dir, _it, _name, _size)                                                                                    \
	for (_i = cfs_lsfile_begin(_dir, _it, _name, _size); _i == CERR_OK ? 1 : (cfs_lsfile_end(&it), 0);                                 \
	     _i = cfs_lsfile_next(_it, _name, _size))

#endif
