#ifndef S3UTIL_H
#define S3UTIL_H

#ifdef USE_ICONV
#include <iconv.h>
#endif

#ifdef _WIN32
#include <windows.h>
#endif

#ifdef __linux__
#include <sys/mman.h>
#endif

#ifndef _WIN32
#include <endian.h>
#endif

#include <sys/stat.h>
#include <sys/types.h>
#include <stddef.h>
#include <stdbool.h>
#include <stdlib.h>
#include <fcntl.h>
#include <stdint.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <time.h>

#define S3UTIL_ATTRIBUTE_INDEX 0x200
#define S3UTIL_ATTRIBUTE_SEQ 0x201
#define S3UTIL_ATTRIBUTE_SONG 0x20

#define S3UTIL_EXCEPTION_IOERROR 0x101
#define S3UTIL_EXCEPTION_HEADER 0x102
#define S3UTIL_EXCEPTION_CONFLICTING_DATA 0x103
#define S3UTIL_EXCEPTION_INDEXTYPE 0x104
#define S3UTIL_EXCEPTION_OUT_OF_RANGE 0x105
#define S3UTIL_EXCEPTION_ICONV_ERROR 0x106
#define S3UTIL_EXCEPTION_OPEN 0x107
#define S3UTIL_EXCEPTION_IOSET 0x108
#define S3UTIL_EXCEPTION_WRONG_RESTYPE 0x109
#define S3UTIL_EXCEPTION_NOICONV 0x110
#define S3UTIL_EXCEPTION_NOTFOUND 0x111
#define S3UTIL_EXCEPTION_OUT_OF_MEMORY 0x200


#define S3UTIL_IOSET_NATIVEOS 0x400
#define S3UTIL_IOSET_LINUX 0x401
#define S3UTIL_IOSET_WIN32 0x402

#define S3UTIL_IOSET_DEFAULT 0x500
#define S3UTIL_IOSET_LIBC 0x501

//memory mapped file
#define S3UTIL_IOSET_NATIVEOS_MMF 0x600
#define S3UTIL_IOSET_LINUX_MMF 0x601
#define S3UTIL_IOSET_WIN32_MMF 0x602
#define S3UTIL_IOSET_LINUX_MMF_FD 0x603
#define S3UTIL_IOSET_WIN32_MMF_HANDLE 0x604


#define S3UTIL_SEEK_CUR 0x20
#define S3UTIL_SEEK_SET 0x21

#define S3UTIL_INTERNAL_OUT_OF_MEMORY(memset, throws) \
	s3util_throw(memset, throws, S3UTIL_EXCEPTION_OUT_OF_MEMORY, NULL, NULL, 0)

#define S3UTIL_HANDLE_EXCEPTION(memset, throws, file, function, line) \
	if(*throws != NULL) { \
		s3util_add_to_stack(memset, throws, file, function, line); \
		return; \
	}



#define S3UTIL_CONCAT(a, b) a##b

#define S3UTIL_INTERNAL_READ(type, ioset, memset, throws) \
	S3UTIL_CONCAT(s3util_internal_read, type)(ioset, memset, throws);

#define S3UTIL_INTERNAL_WRITE(type, ioset, memset, to, throws) \
	S3UTIL_CONCAT(s3util_internal_write, type)(ioset, memset, to, throws);

typedef struct s3util_internal_attribute_t s3util_internal_attribute_t;
typedef struct s3util_internal_stack_t s3util_internal_stack_t;
typedef struct s3util_exception_t s3util_exception_t;
typedef struct s3util_monitor_t s3util_monitor_t;
typedef struct s3util_memset_t s3util_memset_t;
typedef struct s3util_ioset_t s3util_ioset_t;
typedef struct s3util_mmf_t s3util_mmf_t;

struct s3util_internal_stack_t {
	char* file;
	const char* function;
	uint32_t line;

	s3util_internal_stack_t* down;
};

struct s3util_mmf_t {
	void* addr;
	uint32_t pos;
	uint32_t len;
	bool fork;

	void* additional_data; // win32 handles
};

struct s3util_internal_attribute_t {
	uint32_t name;
	uint32_t value;

	s3util_internal_attribute_t* next;
};

struct s3util_exception_t {
	uint32_t type;
	s3util_memset_t* memset;

	s3util_internal_stack_t* stack;
	s3util_internal_attribute_t* attrs;
};

struct s3util_ioset_t {
	bool (*read_func) (void*, void*, size_t);
	bool (*write_func) (void*, void*, size_t);
	size_t (*size_func) (void*);
	size_t (*pos_func) (void*);
	bool (*seek_func) (void*, uint32_t, int);
	void* (*open_func) (void*, bool);
	void (*close_func) (void*);
	void* (*fork_func) (void*);
	bool available;
	void* arg;
};

struct s3util_memset_t {
	void* (*alloc_func) (void*, size_t);
	void (*free_func) (void*, void*);
	void* arg;
};

struct s3util_monitor_t {
	void* io_arg;
	bool close;
	s3util_ioset_t* ioset;

	void* mem_arg;
	void* (*alloc_func) (void*,size_t);
	void (*free_func) (void*,void*);

	uint32_t last_state;
};

void s3util_free_func(s3util_memset_t* memset, void* data);
void* s3util_alloc_func(s3util_memset_t* memset, size_t size, s3util_exception_t** throws);


void s3util_add_attr(s3util_memset_t* memset, s3util_exception_t** throws, uint32_t name, uint32_t value);
void s3util_add_to_stack(s3util_memset_t* memset, s3util_exception_t** throws, char* file, const char* function, uint32_t line);
void s3util_throw(s3util_memset_t* memset, s3util_exception_t** throws, uint32_t exception, char* file, const char* function, uint32_t line);
void s3util_print_exception(s3util_exception_t* ex);
void s3util_delete_exception(s3util_memset_t* memset, s3util_exception_t* ex);
bool s3util_catch_exception(s3util_exception_t** throws);


uint32_t s3util_internal_read32LE(s3util_ioset_t* ioset, s3util_memset_t* memset, s3util_exception_t** throws);
uint16_t s3util_internal_read16LE(s3util_ioset_t* ioset, s3util_memset_t* memset, s3util_exception_t** throws);
uint8_t s3util_internal_read8(s3util_ioset_t* ioset, s3util_memset_t* memset, s3util_exception_t** throws);

void s3util_internal_write32LE(s3util_ioset_t* ioset, s3util_memset_t* memset, uint32_t b32_int, s3util_exception_t** throws);
void s3util_internal_write16LE(s3util_ioset_t* ioset, s3util_memset_t* memset, uint16_t b16_int, s3util_exception_t** throws);
void s3util_internal_write8(s3util_ioset_t* ioset, s3util_memset_t* memset, uint8_t b8_int, s3util_exception_t** throws);

void s3util_internal_seek_func(s3util_ioset_t* ioset, s3util_memset_t* memset, uint32_t pos, int whence, s3util_exception_t** throws);

uint32_t s3util_le32(uint32_t le32_int);
uint16_t s3util_le16(uint16_t le16_int);

uint32_t s3util_le32p(uint32_t* le32_int);
uint16_t s3util_le16p(uint16_t* le16_int);

//text
void s3util_short(s3util_memset_t* memset, char** str);
void s3util_iso8859_to_utf8(s3util_memset_t* memset, char** str, uint32_t len, bool iso8859_2, s3util_exception_t** throws);
void s3util_iconv_dat_to_utf8(s3util_memset_t* memset, char* charset, char* cstr, char** utf8_str, s3util_exception_t** throws);

//ioset and memory functions
s3util_ioset_t* s3util_get_default_ioset(uint32_t type);

void* s3util_monitor_alloc_func(void* arg, size_t size);
void s3util_monitor_free_func(void* arg, void* mem);

void* s3util_default_alloc_func(void* arg, size_t size);
void s3util_default_free_func(void* arg, void* mem);

void s3util_monitor_print(s3util_monitor_t* monitor);

//linux
void* s3util_linux_open_func(void* arg, bool write);
void s3util_linux_close_func(void* arg);
bool s3util_linux_read_func(void* arg, void* bfr, size_t len);
bool s3util_linux_write_func(void* arg, void* bfr, size_t len);
bool s3util_linux_seek_func(void* arg, uint32_t pos, int whence);
size_t s3util_linux_pos_func(void* arg);
size_t s3util_linux_size_func(void* arg);

void* s3util_mmf_linux_fd_open_func(void* arg, bool write);
void* s3util_mmf_linux_name_open_func(void* arg, bool write);
void s3util_mmf_linux_close_func(void* arg);

//windows
void* s3util_win32_open_func(void* arg, bool write);
void s3util_win32_close_func(void* arg);
bool s3util_win32_read_func(void* arg, void* bfr, size_t len);
bool s3util_win32_write_func(void* arg, void* bfr, size_t len);
bool s3util_win32_seek_func(void* arg, uint32_t pos, int whence);
size_t s3util_win32_pos_func(void* arg);
size_t s3util_win32_size_func(void* arg);

void* s3util_mmf_win32_handle_open_func(void* arg, bool write);
void* s3util_mmf_win32_name_open_func(void* arg, bool write);
void s3util_mmf_win32_close_func(void* arg);

void* s3util_libc_open_func(void* arg, bool write);
void s3util_libc_close_func(void* arg);
bool s3util_libc_read_func(void* arg, void* bfr, size_t len);
bool s3util_libc_write_func(void* arg, void* bfr, size_t len);
bool s3util_libc_seek_func(void* arg, uint32_t pos, int whence);
size_t s3util_libc_pos_func(void* arg);
size_t s3util_libc_size_func(void* arg);

bool s3util_mmf_read_func(void* arg, void* bfr, size_t len);
bool s3util_mmf_write_func(void* arg, void* bfr, size_t len);
bool s3util_mmf_seek_func(void* arg, uint32_t pos, int whence);
size_t s3util_mmf_pos_func(void* arg);
size_t s3util_mmf_size_func(void* arg);
void* s3util_mmf_fork_func(void* arg);

#endif /*S3UTIL_H*/
