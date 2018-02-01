#include "s3util.h"
#ifdef PRIVATE_FILENAME
#line __LINE__ "text.c"
#endif

void s3util_short(s3util_memset_t* memset, char** str) {
	char* bfr = *str;

	char* bfr2 = s3util_alloc_func(memset, strlen(bfr)+1, NULL);
	if(bfr2 == NULL) return;

	strcpy(bfr2, bfr);
	s3util_free_func(memset, bfr);

	*str = bfr2;
}

uint16_t s3util_iso8859_2_to_utf8_map[96] = {0xA0, 0x104, 0x2D8, 0x141, 0xA4, 0x13D, 0x15A, 0xA7, 0xA8, 0x160, 0x15E, 0x164, 0x179, 0xAD, 0x17D, 0x17B, 0xB0, 0x105, 0x2DB, 0x142, 0xB4, 0x13E, 0x15B, 0x2C7, 0xB8, 0x161, 0x15F, 0x165, 0x17A, 0x2DD, 0x17E, 0x17C, 0x154, 0xC1, 0xC2, 0x102, 0xC4, 0x139, 0x106, 0xC7, 0x10C, 0xC9, 0x118, 0xCB, 0x11A, 0xCD, 0xCE, 0x10E, 0x110, 0x143, 0x147, 0xD3, 0xD4, 0x150, 0xD6, 0xD7, 0x158, 0x16E, 0xDA, 0x170, 0xDC, 0xDD, 0x162, 0xDF, 0x155, 0xE1, 0xE2, 0x103, 0xE4, 0x13A, 0x107, 0xE7, 0x10D, 0xE9, 0x119, 0xEB, 0x11B, 0xED, 0xEE, 0x10F, 0x111, 0x144, 0x148, 0xF3, 0xF4, 0x151, 0xF6, 0xF7, 0x159, 0x16F, 0xFA, 0x171, 0xFC, 0xFD, 0x163, 0x2D9};

void s3util_iso8859_to_utf8(s3util_memset_t* memset, char** str, uint32_t len, bool iso8859_2, s3util_exception_t** throws) {

	unsigned char* bfr = (unsigned char*) *str;

	uint32_t real_len = 0;
	for(uint32_t i = 0;i != len;i++) {
		if(bfr[i] == '\\' && i+1 != len && bfr[i+1] == 'n') { // \n is only one character
		} else if(iso8859_2 && bfr[i] > 126 && bfr[i] < 160) {
			real_len += 3; // unknown chars like some not iso8859-2 polish characters
		} else if(bfr[i] >= 128){
			real_len += 2;
		} else {
			real_len++;
		}
	}

	unsigned char* bfr2 = s3util_alloc_func(memset, real_len, throws);
	S3UTIL_HANDLE_EXCEPTION(memset, throws, __FILE__, __func__, __LINE__);

	uint32_t bfr2_ptr = 0;
	for(uint32_t bfr_ptr = 0;bfr_ptr != len;bfr_ptr++) {
		if(bfr[bfr_ptr] == '\\' && bfr_ptr+1 != len && bfr[bfr_ptr+1] == 'n') {
			bfr2[bfr2_ptr] = '\n';
			bfr_ptr++;
		} else if(iso8859_2 && bfr[bfr_ptr] > 126 && bfr[bfr_ptr] < 160) {
			bfr2[bfr2_ptr] = 0xEF;
			bfr2[bfr2_ptr+1] = 0xBF;
			bfr2[bfr2_ptr+2] = 0xBD;
			bfr2_ptr += 2;
		} else if((iso8859_2 && bfr[bfr_ptr] > 0xA0) || (!iso8859_2 && bfr[bfr_ptr] >= 128)) {
			uint16_t character = iso8859_2 ? s3util_iso8859_2_to_utf8_map[bfr[bfr_ptr]-0xA0] : bfr[bfr_ptr];
			bfr2[bfr2_ptr+1] = 0x80 | (character & 0x3F);
			bfr2[bfr2_ptr] = 0xC0 | ((character & 0x7C0) >> 6);
			bfr2_ptr++;
		} else {
			bfr2[bfr2_ptr] = bfr[bfr_ptr];
		}

		bfr2_ptr++;
	}

	s3util_free_func(memset, bfr);
	bfr2[real_len-1] = '\0';

	*str = (char*) bfr2;
}

#ifdef USE_ICONV
void s3util_iconv_dat_to_utf8(s3util_memset_t* memset, char* charset, char* cstr, char** utf8_str, s3util_exception_t** throws) {
	iconv_t iconv_s = iconv_open("UTF8", charset);

	if(iconv_s == (iconv_t)-1) {
		s3util_throw(memset, throws, S3UTIL_EXCEPTION_ICONV_ERROR, __FILE__, __func__, __LINE__);
		return;
	}

	size_t inlen = strlen(cstr);
	size_t outlen = inlen*4+4;
	char* utf8s = s3util_alloc_func(memset, outlen, throws);
	if(*throws != NULL) {
		s3util_add_to_stack(memset, throws, __FILE__, __func__, __LINE__);
		return;
	}

	*utf8_str = utf8s;
	char* instr = cstr;

	if(iconv(iconv_s, &instr, &inlen, &utf8s, &outlen) == -1) {
		s3util_free_func(memset, *utf8_str);
		s3util_throw(memset, throws, S3UTIL_EXCEPTION_ICONV_ERROR, __FILE__, __func__, __LINE__);
	}
	iconv_close(iconv_s);
}
#else
void s3util_iconv_dat_to_utf8(s3util_memset_t* memset, char* charset, char* cstr, char** utf8_str, s3util_exception_t** throws) {
	s3util_throw(memset, throws, S3UTIL_EXCEPTION_NOICONV, __FILE__, __func__, __LINE__);
}
#endif

