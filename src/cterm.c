#include "cterm.h"

#include "platform.h"

#if defined(C_WIN)
	#include <io.h>
	#include <windows.h>
#else
	#include <unistd.h>
#endif

int c_term_color(FILE *stream)
{
	if (stream != stdout && stream != stderr) {
		return 0;
	}

#if defined(C_WIN)
	int fd = _fileno(stream);
	if (fd < 0 || !_isatty(fd)) {
		return 0;
	}
	HANDLE handle = GetStdHandle(stream == stdout ? STD_OUTPUT_HANDLE : STD_ERROR_HANDLE);
	DWORD mode;
	return handle != INVALID_HANDLE_VALUE && GetConsoleMode(handle, &mode) &&
	       ((mode & ENABLE_VIRTUAL_TERMINAL_PROCESSING) != 0 || SetConsoleMode(handle, mode | ENABLE_VIRTUAL_TERMINAL_PROCESSING));
#else
	return isatty(stream == stdout ? STDOUT_FILENO : STDERR_FILENO) != 0;
#endif
}
