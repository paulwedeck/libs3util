#include "s3util.h"
#ifdef PRIVATE_FILENAME
#line __LINE__ "exception.c"
#endif

s3util_internal_stack_t s3util_internal_out_of_memory_stack = {NULL, NULL, 0, NULL};
s3util_exception_t s3util_internal_out_of_memory = {S3UTIL_EXCEPTION_OUT_OF_MEMORY, NULL, NULL, NULL};

void s3util_add_to_stack(s3util_memset_t* memset, s3util_exception_t** throws, char* file, const char* func, uint32_t line) {
	if(*throws == NULL) return;
	s3util_internal_stack_t* now;

	if((*throws)->type == S3UTIL_EXCEPTION_OUT_OF_MEMORY) {
		if((*throws)->stack != &s3util_internal_out_of_memory_stack) {
			now = &s3util_internal_out_of_memory_stack;
		} else {
			return; // only one stack member is supported
		}
	} else {
		now = s3util_alloc_func(memset, sizeof(s3util_internal_stack_t), NULL);
		if(now == NULL) return;
	}

	now->file = file;
	now->function = func;
	now->line = line;
	now->down = (*throws)->stack;
	(*throws)->stack = now;
}

void s3util_add_attr(s3util_memset_t* memset, s3util_exception_t** throws, uint32_t name, uint32_t value) {
	if(*throws == NULL || (*throws)->type == S3UTIL_EXCEPTION_OUT_OF_MEMORY) return;

	s3util_internal_attribute_t* attr = s3util_alloc_func(memset, sizeof(s3util_internal_attribute_t), NULL);
	if(attr == NULL) return;

	attr->name = name;
	attr->value = value;
	attr->next = (*throws)->attrs;
	(*throws)->attrs = attr;
}

void s3util_throw(s3util_memset_t* memset, s3util_exception_t** throws, uint32_t type, char* file, const char* func, uint32_t line) {
	if(type == S3UTIL_EXCEPTION_OUT_OF_MEMORY) {
		*throws = &s3util_internal_out_of_memory;
	} else {
		*throws = s3util_alloc_func(memset, sizeof(s3util_exception_t), NULL);
		if(*throws == NULL) {
			s3util_throw(memset, throws, S3UTIL_EXCEPTION_OUT_OF_MEMORY, NULL, NULL, 0); // out_of_memory has priority
		}
		s3util_add_to_stack(memset, throws, file, func, line);
	}
	(*throws)->memset = memset;
	(*throws)->type = type;
}

void s3util_delete_exception(s3util_memset_t* memset, s3util_exception_t* ex) {
	if(ex->type == S3UTIL_EXCEPTION_OUT_OF_MEMORY) return;

	s3util_internal_stack_t* stack1;

	stack1 = ex->stack;

	while(stack1 != NULL) {
		s3util_internal_stack_t* stack2 = stack1;
		stack1 = stack1->down;
		s3util_free_func(memset, stack2);
	}

	s3util_internal_attribute_t* attr1;

	attr1 = ex->attrs;

	while(attr1 != NULL) {
		s3util_internal_attribute_t* attr2 = attr1;
		attr1 = attr1->next;
		s3util_free_func(memset, attr2);
	}

	s3util_free_func(memset, ex);
}

typedef struct {
	uint32_t type;
	char* name;
} s3util_internal_map_entry_t;

s3util_internal_map_entry_t exception_map[] = {
	{S3UTIL_EXCEPTION_IOERROR, "IOError"},
	{S3UTIL_EXCEPTION_HEADER, "WrongHeaderError"},
	{S3UTIL_EXCEPTION_CONFLICTING_DATA, "ConflictingDataError"},
	{S3UTIL_EXCEPTION_INDEXTYPE, "IndexTypeError"},
	{S3UTIL_EXCEPTION_OUT_OF_RANGE, "OutOfRangeError"},
	{S3UTIL_EXCEPTION_ICONV_ERROR, "IconvError"},
	{S3UTIL_EXCEPTION_OPEN, "OpenError"},
	{S3UTIL_EXCEPTION_IOSET, "NullIOSetError"},
	{S3UTIL_EXCEPTION_OUT_OF_MEMORY, "OutOfMemoryError"},
	{S3UTIL_EXCEPTION_NOICONV, "NoIconvSupportError"},
	{S3UTIL_EXCEPTION_NOTFOUND, "NotFoundError"},
	{0, NULL}
};

s3util_internal_map_entry_t attr_map[] = {
	{S3UTIL_ATTRIBUTE_INDEX, "index"},
	{S3UTIL_ATTRIBUTE_SEQ, "sequence"},
	{S3UTIL_ATTRIBUTE_SONG, "song"},
	{0, NULL}
};

char* s3util_internal_find_entry(s3util_internal_map_entry_t* map, uint32_t type) {
	char* re_value = NULL;
	uint32_t index = 0;
	while(map[index].name != NULL) {
		if(map[index].type == type) re_value = map[index].name;
		index++;
	}

	return re_value;
}

void s3util_print_exception(s3util_exception_t* ex) {
	char* name = s3util_internal_find_entry(exception_map, ex->type);

	if(name != NULL) printf("%s caught\n", name);
				else printf("exception caught 0x%x\n", ex->type);

	s3util_internal_attribute_t* attr = ex->attrs;
	while(attr != NULL) {
		printf("at ");
		char* attr_name = s3util_internal_find_entry(attr_map, attr->name);
		if(attr_name != NULL) {
			printf("%s", attr_name);
		} else {
			printf("%u", attr->name);
		}
		printf(": %u\n", attr->value);
		attr = attr->next;
	}

	s3util_internal_stack_t* stack = ex->stack;
	while(stack != NULL) {
		printf(" at %s(%s:%u)\n", stack->function, stack->file, stack->line);
		stack = stack->down;
	}
}

bool s3util_catch_exception(s3util_exception_t** throws) {
	if(*throws != NULL) {
		s3util_print_exception(*throws);
		s3util_delete_exception((*throws)->memset, *throws);
		*throws = NULL;
		return false;
	}
	return true;
}

