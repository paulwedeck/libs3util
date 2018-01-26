#include "s3util.h"
#ifdef PRIVATE_FILENAME
#line __LINE__ "ioset.c"
#endif

#ifdef __linux__
void* s3util_linux_open_func(void* arg, bool write) {
	int fd = open(arg, write ? O_WRONLY : O_RDONLY);

	if(fd == -1) {
		return NULL;
	}

	int* fd_p = calloc(1, sizeof(int));
	if(fd_p != NULL) *fd_p = fd;
		else close(fd);

	return fd_p;
}

void s3util_linux_close_func(void* arg) {
	if(!arg) return;

	close(*(int*)(arg));
	free(arg);
}

bool s3util_linux_read_func(void* arg, void* bfr, size_t len) {
	return read(*((int*)arg), bfr, len) == len;
}

bool s3util_linux_write_func(void* arg, void* bfr, size_t len) {
	return write(*((int*)arg), bfr, len) == len;
}

size_t s3util_linux_size_func(void* arg) {
	struct stat file_stat;
	fstat(*((int*)arg), &file_stat);
	return file_stat.st_size;
}

size_t s3util_linux_pos_func(void* arg) {
	return lseek(*((int*)arg), 0, SEEK_CUR);
}

bool s3util_linux_seek_func(void* arg, uint32_t pos, int whence) {
	int seek_whence = whence == S3UTIL_SEEK_CUR ? SEEK_CUR : SEEK_SET;
	return lseek(*((int*)arg), pos, seek_whence) != (off_t)-1;
}
#else
void* s3util_linux_open_func(void* arg, bool write) {return NULL;}
void s3util_linux_close_func(void* arg) {}
bool s3util_linux_read_func(void* arg, void* bfr, size_t len) {return 0;}
bool s3util_linux_write_func(void* arg, void* bfr, size_t len) {return 0;}
bool s3util_linux_seek_func(void* arg, uint32_t pos, int whence) {return 0;}
size_t s3util_linux_pos_func(void* arg) {return 0;}
size_t s3util_linux_size_func(void* arg) {return 0;}
#endif

#ifdef _WIN32
void* s3util_win32_open_func(void* arg, bool write) {
	HANDLE file_handle = CreateFile(arg,
		write ? GENERIC_WRITE : GENERIC_READ, FILE_SHARE_READ | FILE_SHARE_WRITE,
		NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);

	if(file_handle == INVALID_HANDLE_VALUE) return NULL;
	else return file_handle;
}

void s3util_win32_close_func(void* arg) {
	if(arg) CloseHandle(arg);
}

bool s3util_win32_read_func(void* arg, void* bfr, size_t len) {
	return ReadFile(arg, bfr, len, NULL, NULL);
}

bool s3util_win32_write_func(void* arg, void* bfr, size_t len) {
	return WriteFile(arg, bfr, len, NULL, NULL);
}

bool s3util_win32_seek_func(void* arg, uint32_t pos, int whence) {
	DWORD seek_whence = whence == S3UTIL_SEEK_CUR ? FILE_CURRENT : FILE_BEGIN;
	return SetFilePointer(arg, pos, NULL, seek_whence) != INVALID_SET_FILE_POINTER;
}

size_t s3util_win32_pos_func(void* arg) {
	return SetFilePointer(arg, 0, NULL, FILE_CURRENT);
}

size_t s3util_win32_size_func(void* arg) {
	DWORD pos = SetFilePointer(arg, 0, NULL, FILE_CURRENT);

	size_t size = SetFilePointer(arg, 0, NULL, FILE_END); // TODO make this portable
	SetFilePointer(arg, pos, NULL, FILE_BEGIN);

	return size;
}
#else
void* s3util_win32_open_func(void* arg, bool write) {return NULL;}
void s3util_win32_close_func(void* arg) {}
bool s3util_win32_read_func(void* arg, void* bfr, size_t len) {return 0;}
bool s3util_win32_write_func(void* arg, void* bfr, size_t len) {return 0;}
bool s3util_win32_seek_func(void* arg, uint32_t pos, int whence) {return 0;}
size_t s3util_win32_pos_func(void* arg) {return 0;}
size_t s3util_win32_size_func(void* arg) {return 0;}
#endif

void* s3util_libc_open_func(void* arg, bool write) {
	return fopen(arg, write ? "wb" : "rb");
}

void s3util_libc_close_func(void* arg) {
	if(arg) fclose(arg);
}

bool s3util_libc_read_func(void* arg, void* bfr, size_t len) {
	return fread(bfr, 1, len, arg) == len;
}

bool s3util_libc_write_func(void* arg, void* bfr, size_t len) {
	return fwrite(bfr, 1, len, arg) == len;
}

bool s3util_libc_seek_func(void* arg, uint32_t pos, int whence) {
	int seek_whence = whence == S3UTIL_SEEK_CUR ? SEEK_CUR : SEEK_SET;
	return fseek(arg, pos, seek_whence) != -1;
}

size_t s3util_libc_pos_func(void* arg) {
	return ftell(arg);
}

size_t s3util_libc_size_func(void* arg) {
	size_t pos = ftell(arg);

	fseek(arg, 0, SEEK_END); // TODO make this portable
	size_t size = ftell(arg);
	fseek(arg, pos, SEEK_SET);

	return size;
}

#ifdef __linux__
void* s3util_mmf_linux_fd_open_func(void* arg, bool write) {
	if(write) return NULL; // we can not calculate the file mapping size

	s3util_mmf_t* mmf = calloc(1, sizeof(s3util_mmf_t));
	mmf->len = s3util_linux_size_func(arg);
	mmf->addr = mmap(NULL, mmf->len, write ? PROT_WRITE : PROT_READ, MAP_PRIVATE, *(int*)arg, 0);

	return mmf;
}

void* s3util_mmf_linux_name_open_func(void* arg, bool write) {
	int fd = open(arg, O_RDONLY);
	if(fd == -1) return NULL;

	s3util_mmf_t* mmf = s3util_mmf_linux_fd_open_func(&fd, write);
	close(fd);

	return mmf;
}

void s3util_mmf_linux_close_func(void* arg) {
	if(!arg) return;

	s3util_mmf_t* mmf = arg;

	if(!mmf->fork) munmap(mmf->addr, mmf->len);
	free(mmf);
}
#else
void* s3util_mmf_linux_fd_open_func(void* arg, bool write) {return NULL;}
void* s3util_mmf_linux_name_open_func(void* arg, bool write) {return NULL;}
void s3util_mmf_linux_close_func(void* arg) {}
#endif

#ifdef _WIN32
typedef struct {
	HANDLE win32fm;
	HANDLE file;
} s3util_win32_add_t;

void* s3util_mmf_win32_handle_open_func(void* arg, bool write) {
	if(write) return NULL; // we can not calculate the file mapping size

	HANDLE win32fm = CreateFileMapping(arg, NULL, write ? PAGE_READWRITE : PAGE_READONLY, 0, 0, NULL);

	s3util_mmf_t* mmf = calloc(1, sizeof(s3util_mmf_t));

	mmf->addr = MapViewOfFile(win32fm, write ? FILE_MAP_WRITE : FILE_MAP_READ, 0, 0, 0);
	mmf->len = s3util_win32_size_func(arg);
	mmf->additional_data = calloc(1, sizeof(s3util_win32_add_t));
	((s3util_win32_add_t*)mmf->additional_data)->win32fm = win32fm;
	((s3util_win32_add_t*)mmf->additional_data)->file = NULL;

	return mmf;
}

void* s3util_mmf_win32_name_open_func(void* arg, bool write) {
	HANDLE file = s3util_win32_open_func(arg, write);
	if(file == NULL) return NULL;

	s3util_mmf_t* mmf = s3util_mmf_win32_handle_open_func(file, write);
	if(mmf == NULL) return NULL;

	((s3util_win32_add_t*)mmf->additional_data)->file = file;
	return mmf;
}

void s3util_mmf_win32_close_func(void* arg) {
	if(!arg) return;

	s3util_mmf_t* mmf = arg;

	if(!mmf->fork) {
		UnmapViewOfFile(mmf->addr);

		CloseHandle(((s3util_win32_add_t*)mmf->additional_data)->win32fm);
		if(((s3util_win32_add_t*)mmf->additional_data)->file) CloseHandle(((s3util_win32_add_t*)mmf->additional_data)->file);

		free(mmf->additional_data);
	}
	free(mmf);
}
#else
void* s3util_mmf_win32_handle_open_func(void* arg, bool write) {return NULL;}
void* s3util_mmf_win32_name_open_func(void* arg, bool write) {return NULL;}
void s3util_mmf_win32_close_func(void* arg) {}
#endif

bool s3util_mmf_read_func(void* arg, void* bfr, size_t len) {
	s3util_mmf_t* mmf = arg;

	if(mmf->len < mmf->pos+len) return false;

	memcpy(bfr, mmf->addr+mmf->pos, len);

	mmf->pos += len;

	return true;
}

bool s3util_mmf_write_func(void* arg, void* bfr, size_t len) {
	s3util_mmf_t* mmf = arg;

	if(mmf->len < mmf->pos+len) return false;

	memcpy(mmf->addr+mmf->pos, bfr, len);

	mmf->pos += len;

	return true;
}


bool s3util_mmf_seek_func(void* arg, uint32_t pos, int whence) {
	s3util_mmf_t* mmf = arg;

	uint32_t from = whence == S3UTIL_SEEK_SET ? 0 : mmf->pos;

	if(mmf->len < (pos+from)) return false;
	mmf->pos = pos+from;

	return true;
}

size_t s3util_mmf_pos_func(void* arg) {
	s3util_mmf_t* mmf = arg;
	return mmf->pos;
}

size_t s3util_mmf_size_func(void* arg) {
	s3util_mmf_t* mmf = arg;
	return mmf->len;
}

void* s3util_mmf_fork_func(void* arg) {
	s3util_mmf_t* mmf = arg;

	s3util_mmf_t* fork = calloc(1, sizeof(s3util_mmf_t));
	memcpy(fork, mmf, sizeof(s3util_mmf_t));

	fork->fork = true;

	return fork;
}

s3util_ioset_t s3util_internal_linux_ioset = {
	s3util_linux_read_func,
	s3util_linux_write_func,
	s3util_linux_size_func,
	s3util_linux_pos_func,
	s3util_linux_seek_func,
	s3util_linux_open_func,
	s3util_linux_close_func,
	NULL,
	#ifdef __linux__
	true
	#else
	false
	#endif
};

s3util_ioset_t s3util_internal_mmf_linux_name_ioset = {
	s3util_mmf_read_func,
	s3util_mmf_write_func,
	s3util_mmf_size_func,
	s3util_mmf_pos_func,
	s3util_mmf_seek_func,
	s3util_mmf_linux_name_open_func,
	s3util_mmf_linux_close_func,
	s3util_mmf_fork_func,
	#ifdef __linux__
	true
	#else
	false
	#endif
};

s3util_ioset_t s3util_internal_mmf_linux_fd_ioset = {
	s3util_mmf_read_func,
	s3util_mmf_write_func,
	s3util_mmf_size_func,
	s3util_mmf_pos_func,
	s3util_mmf_seek_func,
	s3util_mmf_linux_fd_open_func,
	s3util_mmf_linux_close_func,
	s3util_mmf_fork_func,
	#ifdef __linux__
	true
	#else
	false
	#endif
};


s3util_ioset_t s3util_internal_win32_ioset = {
	s3util_win32_read_func,
	s3util_win32_write_func,
	s3util_win32_size_func,
	s3util_win32_pos_func,
	s3util_win32_seek_func,
	s3util_win32_open_func,
	s3util_win32_close_func,
	NULL,
	#ifdef _WIN32
	true
	#else
	false
	#endif
};

s3util_ioset_t s3util_internal_mmf_win32_name_ioset = {
	s3util_mmf_read_func,
	s3util_mmf_write_func,
	s3util_mmf_size_func,
	s3util_mmf_pos_func,
	s3util_mmf_seek_func,
	s3util_mmf_win32_name_open_func,
	s3util_mmf_win32_close_func,
	s3util_mmf_fork_func,
	#ifdef _WIN32
	true
	#else
	false
	#endif
};

s3util_ioset_t s3util_internal_mmf_win32_handle_ioset = {
	s3util_mmf_read_func,
	s3util_mmf_write_func,
	s3util_mmf_size_func,
	s3util_mmf_pos_func,
	s3util_mmf_seek_func,
	s3util_mmf_win32_handle_open_func,
	s3util_mmf_win32_close_func,
	s3util_mmf_fork_func,
	#ifdef _WIN32
	true
	#else
	false
	#endif
};


s3util_ioset_t s3util_internal_libc_ioset = {
	s3util_libc_read_func,
	s3util_libc_write_func,
	s3util_libc_size_func,
	s3util_libc_pos_func,
	s3util_libc_seek_func,
	s3util_libc_open_func,
	s3util_libc_close_func,
	NULL,
	true
};

s3util_ioset_t s3util_internal_null_ioset = {
	NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, false
};

s3util_ioset_t* s3util_get_default_ioset(uint32_t type) {
	if(type == S3UTIL_IOSET_NATIVEOS) {
		#ifdef _WIN32
		return &s3util_internal_win32_ioset;
		#elif (defined __linux__)
		return &s3util_internal_linux_ioset;
		#endif
	} else if(type == S3UTIL_IOSET_LINUX) {
		return &s3util_internal_linux_ioset;
	} else if(type == S3UTIL_IOSET_WIN32) {
		return &s3util_internal_win32_ioset;
	} else if(type == S3UTIL_IOSET_LIBC || type == S3UTIL_IOSET_DEFAULT) {
		return &s3util_internal_libc_ioset;
	} else if(type == S3UTIL_IOSET_NATIVEOS_MMF) {
		#ifdef _WIN32
		return &s3util_internal_mmf_win32_name_ioset;
		#elif (defined __linux__)
		return &s3util_internal_mmf_linux_name_ioset;
		#endif
	} else if(type == S3UTIL_IOSET_LINUX_MMF) {
		return &s3util_internal_mmf_linux_name_ioset;
	} else if(type == S3UTIL_IOSET_WIN32_MMF) {
		return &s3util_internal_mmf_win32_name_ioset;
	} else if(type == S3UTIL_IOSET_LINUX_MMF_FD) {
		return &s3util_internal_mmf_linux_fd_ioset;
	} else if(type == S3UTIL_IOSET_WIN32_MMF_HANDLE) {
		return &s3util_internal_mmf_win32_handle_ioset;
	}

	return &s3util_internal_null_ioset;
}
