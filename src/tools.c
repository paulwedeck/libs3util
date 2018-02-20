#include "config.s3util.h"
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

void* s3util_default_alloc_func(void* arg, size_t size) {
	return calloc(size, 1);
}

void* s3util_monitor_alloc_func(void* arg, size_t size) {
	s3util_monitor_t* mon = arg;
	mon->last_state += size;

	s3util_monitor_print(mon);

	uint8_t* mem = mon->alloc_func(mon->mem_arg, size+4);
	if(!mem) return NULL;

	*((uint32_t*)mem) = size;

	return mem+4;
}


void s3util_default_free_func(void* arg, void* mem) {
	if(mem != NULL) free(mem);
}

void s3util_monitor_free_func(void* arg, void* mem) {
	s3util_monitor_t* mon = arg;
	mon->last_state -= *(((uint32_t*)mem)-1);

	s3util_monitor_print(mon);

	mon->free_func(mon->mem_arg, ((uint32_t*)mem)-1);

	if(mon->last_state == 0) {
		if(mon->close) mon->ioset->close_func(mon->io_arg);
		mon->free_func(mon->mem_arg, mon);
	}
}

void s3util_monitor_print(s3util_monitor_t* monitor) {
	char bfr[1024];

	snprintf(bfr, 1023, "%li %u\n", clock(), monitor->last_state);

	monitor->ioset->write_func(monitor->io_arg, bfr, strlen(bfr));
}

s3util_color_t s3util_native_to_8b(void *addr, s3util_color_type type) {
	s3util_color_t color = {0, 0, 0, 0xFF};
	if(type == s3util_alpha1) return color;


	double d58 = 255.0/31.0;
	double d68 = 255.0/63.0;

	if(type == s3util_gray5) {
		color.red = color.green = color.blue = (uint8_t)((*((uint8_t*)addr) & 0x1F)*d58);
		return color;
	}

	uint16_t raw = s3util_le16p(addr);

	if(type == s3util_rgb555) {
		color.red = (uint8_t)(((raw >> 10) & 0x1F)*d58);
		color.green = (uint8_t)(((raw >> 5) & 0x1F)*d58);
	} else {
		color.red = (uint8_t)(((raw >> 11)& 0x1F)*d58);
		color.green = (uint8_t)(((raw >> 5) & 0x3F)*d68);
	}
	color.blue = (uint8_t)((raw & 0x1F)*d58);

	return color;
}

void s3util_internal_8b_to_native(s3util_color_t *color, void *to, s3util_color_type type) {
	if(type == s3util_alpha1) return;

	uint8_t* ptr8 = to;
	uint16_t* ptr16 = to;

	double d85 = 31.0/255.0;
	double d86 = 63.0/255.0;

	if(type == s3util_gray5) {
		uint8_t gray5 = (uint8_t) (((color->red+color->green+color->blue)/3)*d85);
		*ptr8 = (uint8_t) (gray5 & 0x1F);
		return;
	}

	uint16_t red, green;

	uint16_t blue = (uint16_t)(color->blue*d85);

	if(type == s3util_rgb555) {
		red = (uint8_t) (color->red*d85);
		green = (uint8_t) (color->green*d85);

		red = (red) << 10;
		green = (green) << 5;
	} else {
		red = (uint8_t) (color->red*d85);
		green = (uint8_t) (color->green*d86);

		red = (red) << 11;
		green = (green) << 5;
	}

	*ptr16 = s3util_le16(red + green + blue);
}
