#include "s3util.h"
#ifdef PRIVATE_FILENAME
#line __LINE__ "iotools.c"
#endif

uint32_t s3util_le32(uint32_t le32_int) {
	#ifdef _WIN32
		#ifdef IS_BE
		return ((le32_int & 0xFF) << 24) | ((le32_int & 0xFF00) << 8) |
		((le32_int & 0xFF0000) >> 8) | ((le32_int & 0xFF000000) >> 24);
		#else
		return le32_int;
		#endif
	#else
	return le32toh(le32_int);
	#endif
}

uint32_t s3util_le32p(uint32_t* le32_int) {
	return s3util_le32(*((uint32_t*)le32_int));
}


uint16_t s3util_le16(uint16_t le16_int) {
	#ifdef _WIN32
		#ifdef IS_BE
		return ((le16_int & 0xFF) << 24) | ((le16_int & 0xFF00) << 8);
		#else
		return le16_int;
		#endif
	#else
	return le16toh(le16_int);
	#endif
}

uint16_t s3util_le16p(uint16_t* le16_int) {
	return s3util_le16(*((uint16_t*)le16_int));
}

void s3util_internal_seek_func(s3util_ioset_t* ioset, s3util_memset_t* memset, uint32_t pos, int whence, s3util_exception_t** throws) {
	if(!ioset->seek_func(ioset->arg, pos, whence)) {
		s3util_throw(memset, throws, S3UTIL_EXCEPTION_IOERROR, __FILE__, __func__, __LINE__);
	}
}

uint32_t s3util_internal_read32LE(s3util_ioset_t* ioset, s3util_memset_t* memset, s3util_exception_t** throws) {
	uint32_t dat;
	if(!ioset->read_func(ioset->arg, &dat, 4)) s3util_throw(memset, throws, S3UTIL_EXCEPTION_IOERROR, __FILE__, __func__, __LINE__);
	return s3util_le32(dat);

}

void s3util_internal_write32LE(s3util_ioset_t* ioset, s3util_memset_t* memset, uint32_t b32_int, s3util_exception_t** throws) {
	uint32_t le32_int = s3util_le32(b32_int);
	if(!ioset->write_func(ioset->arg, &le32_int, 4)) s3util_throw(memset, throws, S3UTIL_EXCEPTION_IOERROR, __FILE__, __func__, __LINE__);
}

uint16_t s3util_internal_read16LE(s3util_ioset_t* ioset, s3util_memset_t* memset, s3util_exception_t** throws) {
	uint16_t dat;
	if(!ioset->read_func(ioset->arg, &dat, 2)) s3util_throw(memset, throws, S3UTIL_EXCEPTION_IOERROR, __FILE__, __func__, __LINE__);
	return s3util_le16(dat);
}

void s3util_internal_write16LE(s3util_ioset_t* ioset, s3util_memset_t* memset, uint16_t b16_int, s3util_exception_t** throws) {
	uint16_t le16_int = s3util_le16(b16_int);
	if(!ioset->write_func(ioset->arg, &le16_int, 2)) s3util_throw(memset, throws, S3UTIL_EXCEPTION_IOERROR, __FILE__, __func__, __LINE__);
}

uint8_t s3util_internal_read8(s3util_ioset_t* ioset, s3util_memset_t* memset, s3util_exception_t** throws) {
	uint8_t dat;
	if(!ioset->read_func(ioset->arg, &dat, 1)) s3util_throw(memset, throws, S3UTIL_EXCEPTION_IOERROR, __FILE__, __func__, __LINE__);
	return dat;
}

void s3util_internal_write8(s3util_ioset_t* ioset, s3util_memset_t* memset, uint8_t b8_int, s3util_exception_t** throws) {
	if(!ioset->write_func(ioset->arg, &b8_int, 1)) s3util_throw(memset, throws, S3UTIL_EXCEPTION_IOERROR, __FILE__, __func__, __LINE__);
}


void s3util_free_func(s3util_memset_t* memset, void* data) {
	memset->free_func(memset->arg, data);
}

void* s3util_alloc_func(s3util_memset_t* memset, size_t size, s3util_exception_t** throws) {
	void* new_block = memset->alloc_func(memset->arg, size);

	if(new_block == NULL && throws != NULL) {
		S3UTIL_INTERNAL_OUT_OF_MEMORY(memset, throws);
	}

	return new_block;
}