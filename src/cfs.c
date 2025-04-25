#include "cfs.h"

#include "platform.h"

#include <errno.h>
#include <stdio.h>

#if defined(C_WIN)
	#include <Windows.h>
#else
	#include <dirent.h>
	#include <sys/stat.h>
	#include <unistd.h>
#endif

static char *str_cpy(char *dst, const char *src, size_t size)
{
	for (size_t i = 0; i < size; i++) {
		dst[i] = src[i];
		if (src[i] == '\0') {
			return &dst[i];
		}
	}

	return NULL;
}

cerr_t cfs_open(const char *path, const char *mode, void **file)
{
	if (path == NULL || mode == NULL || file == NULL) {
		return CERR_VAL;
	}

	errno = 0;
#if defined(C_WIN)
	switch (mode[0]) {
	case 'r':
	case 'w':
	case 'a': break;
	default: return CERR_VAL;
	}
	fopen_s((FILE **)file, path, mode);
#else
	*file = fopen(path, mode);
#endif

	cerr_t ret = CERR_UNKNOWN;

	switch (errno) {
#if defined(C_WIN)
	case 0: ret = CERR_OK; break;
	case EACCES: ret = cfs_isdir(path) ? CERR_TYPE : CERR_UNKNOWN; break;
	case EINVAL: ret = path[0] == '\0' ? CERR_NOT_FOUND : CERR_VAL; break;
#else
	case 0: ret = cfs_isfile(path) ? CERR_OK : (fclose(*file), CERR_TYPE); break;
	case ENOTDIR: ret = CERR_NOT_FOUND; break;
	case EISDIR: ret = CERR_TYPE; break;
	case EINVAL: ret = CERR_VAL; break;
#endif
	case ENOENT: ret = CERR_NOT_FOUND; break;
	}

	return ret;
}

cerr_t cfs_close(void *file)
{
	if (file == NULL) {
		return CERR_VAL;
	}

	fclose(file);
	return CERR_OK;
}

cerr_t cfs_write(void *file, const void *data, size_t size)
{
	if (file == NULL || data == NULL) {
		return CERR_VAL;
	}

	errno	   = 0;
	size_t cnt = fwrite(data, size, 1, file);
	if (cnt != 1) {
		cerr_t ret = CERR_UNKNOWN;
		switch (errno) {
		case EBADF: ret = CERR_DESC; break;
		}
		return ret;
	}

	return CERR_OK;
}

cerr_t cfs_read(void *file, void *data, size_t size)
{
	if (file == NULL || data == NULL) {
		return CERR_VAL;
	}

	size_t cnt;

	errno = 0;
#if defined(C_WIN)
	cnt = fread_s(data, size, size, 1, file);
#else
	cnt = fread(data, size, 1, file);
#endif
	if (size == 0 && cnt == 0 && errno == 0) {
		return CERR_OK;
	}

	if (cnt != 1) {
		cerr_t ret = CERR_UNKNOWN;
		switch (errno) {
#if defined(C_WIN)
		case 0: ret = CERR_DESC; break;
#else
		case EBADF: ret = CERR_DESC; break;
#endif
		}
		return ret;
	}

	return CERR_OK;
}

cerr_t cfs_du(void *file, size_t *size)
{
	if (file == NULL || size == NULL) {
		return CERR_VAL;
	}

	fseek(file, 0L, SEEK_END);
	*size = ftell(file);
	fseek(file, 0L, SEEK_SET);

	return CERR_OK;
}

int cfs_isdir(const char *path)
{
#if defined(C_WIN)
	int dwAttrib = GetFileAttributesA(path);
	return dwAttrib != INVALID_FILE_ATTRIBUTES && (dwAttrib & FILE_ATTRIBUTE_DIRECTORY);
#else
	struct stat buffer;
	if (stat(path, &buffer)) {
		return 0;
	} else {
		return S_ISDIR(buffer.st_mode);
	}
#endif
}

int cfs_isfile(const char *path)
{
#if defined(C_WIN)
	int dwAttrib = GetFileAttributesA(path);
	return dwAttrib != INVALID_FILE_ATTRIBUTES && !(dwAttrib & FILE_ATTRIBUTE_DIRECTORY);
#else
	struct stat buffer;
	if (path == NULL || stat(path, &buffer)) {
		return 0;
	} else {
		return S_ISREG(buffer.st_mode);
	}
#endif
}

cerr_t cfs_mkdir(const char *path)
{
#if defined(C_WIN)
	if (CreateDirectoryA(path, NULL) == 0) {
		cerr_t ret = CERR_UNKNOWN;
		DWORD err  = GetLastError();
		switch (err) {
		case ERROR_PATH_NOT_FOUND: ret = path == NULL ? CERR_VAL : CERR_NOT_FOUND; break;
		case ERROR_ALREADY_EXISTS: ret = CERR_EXIST; break;
		}
		return ret;
	}
#else
	errno = 0;
	if (mkdir(path, 0700)) {
		cerr_t ret = CERR_UNKNOWN;
		switch (errno) {
		case ENOENT: ret = CERR_NOT_FOUND; break;
		case EFAULT: ret = CERR_VAL; break;
		case EEXIST: ret = CERR_EXIST; break;
		case ENOTDIR: ret = CERR_NOT_FOUND; break;
		}
		return ret;
	}
#endif

	return CERR_OK;
}

cerr_t cfs_mkfile(const char *path)
{
#if defined(C_WIN)
	HANDLE h = CreateFileA(path, GENERIC_WRITE, 0, NULL, CREATE_NEW, FILE_ATTRIBUTE_NORMAL, NULL);
	if (h == INVALID_HANDLE_VALUE) {
		cerr_t ret = CERR_UNKNOWN;
		DWORD err  = GetLastError();
		switch (err) {
		case ERROR_PATH_NOT_FOUND: ret = path == NULL ? CERR_VAL : CERR_NOT_FOUND; break;
		case ERROR_ACCESS_DENIED: ret = cfs_isdir(path) ? CERR_TYPE : CERR_UNKNOWN; break;
		case ERROR_FILE_EXISTS: ret = CERR_EXIST; break;
		}
		return ret;
	}
	CloseHandle(h);
#else
	if (cfs_isfile(path)) {
		return CERR_EXIST;
	}
	void *file;
	cerr_t err = cfs_open(path, "w", &file);
	if (err) {
		return err;
	}
	cfs_close(file);
#endif

	return CERR_OK;
}

cerr_t cfs_rmdir(const char *path)
{
#if defined(C_WIN)
	if (RemoveDirectoryA(path) == 0) {
		cerr_t ret = CERR_UNKNOWN;
		DWORD err  = GetLastError();
		switch (err) {
		case ERROR_FILE_NOT_FOUND: ret = CERR_NOT_FOUND; break;
		case ERROR_PATH_NOT_FOUND: ret = path == NULL ? CERR_VAL : CERR_NOT_FOUND; break;
		case ERROR_DIR_NOT_EMPTY: ret = CERR_NOT_EMPTY; break;
		case ERROR_DIRECTORY: ret = CERR_NOT_FOUND; break;
		}
		return ret;
	}
#else
	if (cfs_isfile(path)) {
		return CERR_NOT_FOUND;
	}

	errno = 0;
	if (remove(path)) {
		cerr_t ret = CERR_UNKNOWN;
		switch (errno) {
		case ENOENT: ret = CERR_NOT_FOUND; break;
		case EFAULT: ret = CERR_VAL; break;
		case ENOTEMPTY: ret = CERR_NOT_EMPTY; break;
		}
		return ret;
	}
#endif

	return CERR_OK;
}

cerr_t cfs_rmfile(const char *path)
{
#if defined(C_WIN)
	if (DeleteFileA(path) == 0) {
		cerr_t ret = CERR_UNKNOWN;
		DWORD err  = GetLastError();
		switch (err) {
		case ERROR_FILE_NOT_FOUND: ret = CERR_NOT_FOUND; break;
		case ERROR_PATH_NOT_FOUND: ret = path == NULL ? CERR_VAL : CERR_NOT_FOUND; break;
		case ERROR_ACCESS_DENIED: ret = cfs_isdir(path) ? CERR_NOT_FOUND : CERR_UNKNOWN; break;
		}
		return ret;
	}
#else
	if (cfs_isdir(path)) {
		return CERR_NOT_FOUND;
	}

	errno = 0;
	if (remove(path)) {
		cerr_t ret = CERR_UNKNOWN;
		switch (errno) {
		case ENOENT: ret = CERR_NOT_FOUND; break;
		case EFAULT: ret = CERR_VAL; break;
		}
		return ret;
	}
#endif
	return CERR_OK;
}

cerr_t cfs_getcwd(char *path, size_t size)
{
	if (path == NULL) {
		return CERR_VAL;
	}

#if defined(C_WIN)
	if (size == 0) {
		return CERR_VAL;
	}

	size_t cnt = GetCurrentDirectory(0, NULL);
	if (cnt > size) {
		return CERR_MEM;
	}

	GetCurrentDirectory((DWORD)size, path);
#else
	errno = 0;
	if (getcwd(path, size) == NULL) {
		cerr_t ret = CERR_UNKNOWN;
		switch (errno) {
		case EINVAL: ret = CERR_VAL; break;
		case ERANGE: ret = CERR_MEM; break;
		}
		return ret;
	}
#endif
	return CERR_OK;
}

cerr_t cfs_lsdir_begin(const char *path, void **it, char *name, size_t size)
{
	if (path == NULL || it == NULL || name == NULL) {
		return CERR_VAL;
	}

#if defined(C_WIN)
	if (path[0] == '\0') {
		return CERR_NOT_FOUND;
	}

	char child_path[C_MAX_PATH] = {0};

	char *path_end = str_cpy(child_path, path, sizeof(child_path));
	str_cpy(path_end, "\\*.*", sizeof(child_path) - (path_end - child_path));

	WIN32_FIND_DATA data = {0};
	if ((*(HANDLE *)it = FindFirstFileA(child_path, (LPWIN32_FIND_DATAA)&data)) == INVALID_HANDLE_VALUE) {
		cerr_t ret = CERR_UNKNOWN;
		DWORD err  = GetLastError();
		switch (err) {
		case ERROR_FILE_NOT_FOUND: ret = CERR_NOT_FOUND; break;
		case ERROR_PATH_NOT_FOUND: ret = CERR_NOT_FOUND; break;
		case ERROR_DIRECTORY: ret = CERR_TYPE; break;
		}
		return ret;
	}

	const char *n = (char *)data.cFileName;
	if ((n[0] == '.' && n[1] == '\0') || (n[0] == '.' && n[1] == '.' && n[2] == '\0') ||
	    !(data.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)) {
		return cfs_lsdir_next(it, name, size);
	}

	if (str_cpy(name, n, size) == NULL) {
		return CERR_MEM;
	}

	return CERR_OK;
#else
	if ((*(DIR **)it = opendir(path)) == NULL) {
		cerr_t ret = CERR_UNKNOWN;
		switch (errno) {
		case ENOENT: ret = CERR_NOT_FOUND; break;
		case ENOTDIR: ret = CERR_TYPE; break;
		}
		return ret;
	}

	return cfs_lsdir_next(it, name, size);
#endif
}

cerr_t cfs_lsdir_next(void **it, char *name, size_t size)
{
	if (it == NULL || name == NULL) {
		return CERR_VAL;
	}

#if defined(C_WIN)
	WIN32_FIND_DATA data = {0};
	while (FindNextFileA(*(HANDLE *)it, (LPWIN32_FIND_DATAA)&data)) {
		const char *n = (char *)data.cFileName;
#else
	struct dirent *dp;
	while ((dp = readdir(*(DIR **)it))) {
		const char *n = dp->d_name;
#endif
		if ((n[0] == '.' && n[1] == '\0') || (n[0] == '.' && n[1] == '.' && n[2] == '\0')) {
			continue;
		}
#if defined(C_WIN)
		if (!(data.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY))
#else
		if (dp->d_type != 4)
#endif
		{
			continue;
		}

		if (str_cpy(name, n, size) == NULL) {
			return CERR_MEM;
		}

		return CERR_OK;
	}
	return CERR_END;
}

int cfs_lsdir_end(void **it)
{
	if (it == NULL) {
		return CERR_VAL;
	}

#if defined(C_WIN)
	FindClose(*(HANDLE *)it);
#else
	closedir(*(DIR **)it);
#endif

	return CERR_OK;
}

cerr_t cfs_lsfile_begin(const char *path, void **it, char *name, size_t size)
{
	if (path == NULL || it == NULL || name == NULL) {
		return CERR_VAL;
	}

#if defined(C_WIN)
	if (path[0] == '\0') {
		return CERR_NOT_FOUND;
	}

	char child_path[C_MAX_PATH] = {0};

	char *path_end = str_cpy(child_path, path, sizeof(child_path));
	str_cpy(path_end, "\\*.*", sizeof(child_path) - (path_end - child_path));

	WIN32_FIND_DATA data = {0};
	if ((*(HANDLE *)it = FindFirstFileA(child_path, (LPWIN32_FIND_DATAA)&data)) == INVALID_HANDLE_VALUE) {
		cerr_t ret = CERR_UNKNOWN;
		DWORD err  = GetLastError();
		switch (err) {
		case ERROR_FILE_NOT_FOUND: ret = CERR_NOT_FOUND; break;
		case ERROR_PATH_NOT_FOUND: ret = CERR_NOT_FOUND; break;
		case ERROR_DIRECTORY: ret = CERR_TYPE; break;
		}
		return ret;
	}

	const char *n = (char *)data.cFileName;
	if ((n[0] == '.' && n[1] == '\0') || (n[0] == '.' && n[1] == '.' && n[2] == '\0') ||
	    (data.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)) {
		return cfs_lsfile_next(it, name, size);
	}

	if (str_cpy(name, n, size) == NULL) {
		return CERR_MEM;
	}

	return CERR_OK;
#else
	if ((*(DIR **)it = opendir(path)) == NULL) {
		cerr_t ret = CERR_UNKNOWN;
		switch (errno) {
		case ENOENT: ret = CERR_NOT_FOUND; break;
		case ENOTDIR: ret = CERR_TYPE; break;
		}
		return ret;
	}

	return cfs_lsfile_next(it, name, size);
#endif
}

cerr_t cfs_lsfile_next(void **it, char *name, size_t size)
{
	if (it == NULL || name == NULL) {
		return CERR_VAL;
	}

#if defined(C_WIN)
	WIN32_FIND_DATA data = {0};
	while (FindNextFileA(*(HANDLE *)it, (LPWIN32_FIND_DATAA)&data)) {
		const char *n = (char *)data.cFileName;
#else
	struct dirent *dp;
	while ((dp = readdir(*(DIR **)it))) {
		const char *n = dp->d_name;
#endif
		if ((n[0] == '.' && n[1] == '\0') || (n[0] == '.' && n[1] == '.' && n[2] == '\0')) {
			continue;
		}
#if defined(C_WIN)
		if (data.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
#else
		if (dp->d_type != 8)
#endif
		{
			continue;
		}

		if (str_cpy(name, n, size) == NULL) {
			return CERR_MEM;
		}

		return CERR_OK;
	}
	return CERR_END;
}

int cfs_lsfile_end(void **it)
{
	if (it == NULL) {
		return CERR_VAL;
	}

#if defined(C_WIN)
	FindClose(*(HANDLE *)it);
#else
	closedir(*(DIR **)it);
#endif

	return CERR_OK;
}
