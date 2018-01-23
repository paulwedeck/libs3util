#ifndef S3UTIL_H
#define S3UTIL_H

#ifdef _WIN32
#include <windows.h>
#endif

#ifdef __linux__
#include <sys/mman.h>
#endif

#ifndef _WIN32
#include <endian.h>
#endif

#define S3DAT_ATTRIBUTE_INDEX 0x200
#define S3DAT_ATTRIBUTE_SEQ 0x201
#define S3DAT_ATTRIBUTE_SONG 0x20

#define S3DAT_EXCEPTION_WRONG_RESTYPE 0x109
#define S3DAT_EXCEPTION_IOERROR 0x101
#define S3DAT_EXCEPTION_HEADER 0x102
#define S3DAT_EXCEPTION_CONFLICTING_DATA 0x103
#define S3DAT_EXCEPTION_INDEXTYPE 0x104
#define S3DAT_EXCEPTION_OUT_OF_RANGE 0x105
#define S3DAT_EXCEPTION_ICONV_ERROR 0x106
#define S3DAT_EXCEPTION_OPEN 0x107
#define S3DAT_EXCEPTION_IOSET 0x108
#define S3DAT_EXCEPTION_WRONG_RESTYPE 0x109
#define S3DAT_EXCEPTION_OUT_OF_MEMORY 0x200


#define S3DAT_IOSET_NATIVEOS 0x400
#define S3DAT_IOSET_LINUX 0x401
#define S3DAT_IOSET_WIN32 0x402

#define S3DAT_IOSET_DEFAULT 0x500
#define S3DAT_IOSET_LIBC 0x501

//memory mapped file
#define S3DAT_IOSET_NATIVEOS_MMF 0x600
#define S3DAT_IOSET_LINUX_MMF 0x601
#define S3DAT_IOSET_WIN32_MMF 0x602
#define S3DAT_IOSET_LINUX_MMF_FD 0x603
#define S3DAT_IOSET_WIN32_MMF_HANDLE 0x604


#define S3DAT_SEEK_CUR 0x20
#define S3DAT_SEEK_SET 0x21

#define S3DAT_INTERNAL_OUT_OF_MEMORY(handle, throws) \
	s3dat_throw(handle, throws, S3DAT_EXCEPTION_OUT_OF_MEMORY, NULL, NULL, 0)

#define S3DAT_HANDLE_EXCEPTION(handle, throws, file, function, line)  \
	if(*throws != NULL) { \
		s3dat_add_to_stack(handle, throws, file, function, line); \
		return; \
	}

typedef struct s3dat_internal_attribute_t s3dat_internal_attribute_t;
typedef struct s3dat_internal_stack_t s3dat_internal_stack_t;
typedef struct s3dat_exception_t s3dat_exception_t;
typedef struct s3dat_memset_t s3dat_memset_t;
typedef struct s3dat_ioset_t s3dat_ioset_t;
typedef struct s3dat_mmf_t s3dat_mmf_t;






struct s3dat_internal_stack_t {
	uint8_t* file;
	const uint8_t* function;
	uint32_t line;

	s3dat_internal_stack_t* down;
};

struct s3dat_mmf_t {
	void* addr;
	uint32_t pos;
	uint32_t len;
	bool fork;

	void* additional_data; // win32 handles
};

struct s3dat_internal_attribute_t {
	uint32_t name;
	uint32_t value;

	s3dat_internal_attribute_t* next;
};

struct s3dat_exception_t {
	uint32_t type;
	s3dat_t* parent;

	s3dat_internal_stack_t* stack;
	s3dat_internal_attribute_t* attrs;
};

struct s3dat_ioset_t {
	bool (*read_func) (void*, void*, size_t);
	bool (*write_func) (void*, void*, size_t);
	size_t (*size_func) (void*);
	size_t (*pos_func) (void*);
	bool (*seek_func) (void*, uint32_t, int);
	void* (*open_func) (void*, bool);
	void (*close_func) (void*);
	void* (*fork_func) (void*);
	bool available;
};


void s3dat_free_func(s3dat_t* handle, void* data);
void* s3dat_alloc_func(s3dat_t* handle, size_t size, s3dat_exception_t** throws);


void s3dat_add_attr(s3dat_t* handle, s3dat_exception_t** throws, uint32_t name, uint32_t value);
void s3dat_add_to_stack(s3dat_t* handle, s3dat_exception_t** throws, uint8_t* file, const uint8_t* function, uint32_t line);
void s3dat_throw(s3dat_t* handle, s3dat_exception_t** throws, uint32_t exception, uint8_t* file, const uint8_t* function, uint32_t line);


uint32_t s3dat_le32(uint32_t le32_int);
uint16_t s3dat_le16(uint16_t le16_int);

uint32_t s3dat_le32p(uint32_t* le32_int);
uint16_t s3dat_le16p(uint16_t* le16_int);

#endif /*S3UTIL_H*/
