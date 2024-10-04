#include "Stack_Debug_Functions.h"

void Stack_Dump(const Stack_t * stack, void (* Printf_Format_Func)(const void * value),
                const char * file, const char* func, int line) {

    if(stack->Stack_Status) {
        printf("Stack_t [%p]\n", stack);
        printf("    called from %s:%d (%s)\n", file, line, func);
        DEBUG_ON(printf("    name \"%s\" born at %s:%d (%s) {\n",
                        stack->name, stack->file, stack->line, stack->func);)

        #ifndef NO_CANARY_PROTECTION
        printf("        left_struct_canary = %#llx\n", stack->left_struct_canary);
        printf("        right_struct_canary = %#llx\n", stack->right_struct_canary);
        #endif

        printf("        capacity = %lld\n", stack->capacity);
        printf("        size = %lld\n", stack->size);
        printf("        mem_size_aligned = %zd\n", stack->mem_size_aligned);

        #ifndef NO_HASH_PROTECTION
        printf("        struct_hash_sum = %zd\n", stack->struct_hash_sum);
        printf("        data_hash_sum = %zd\n", stack->data_hash_sum);
        #endif

        printf("        data = [%p] {   * - element belongs to stack\n", stack->data);

        #ifndef NO_CANARY_PROTECTION
        uint64_t tmp = 0;
        memcpy(&tmp, (((char*)(stack->data)) - sizeof(uint64_t)), sizeof(uint64_t));
        printf("            left_data_canary (%p) = ", ((uint64_t*)(((char*)(stack->data)) -
                                                        sizeof(uint64_t))));
        printf("%#llx\n", tmp);
        #endif

        for(uint64_t i = 0; i < stack->size; i++) {

        printf("            *[%d] = ", i);
        double tmp = 0;
        memcpy(&tmp, ((char*)stack->data + i*(stack->element_size)), stack->element_size);
        Printf_Format_Func(&tmp);
        printf("\n");
        }
        for(uint64_t i = stack->size; i < stack->capacity; i++){

        printf("             [%d] = ", i);
        double tmp = 0;
        memcpy(&tmp, ((char*)stack->data + i*(stack->element_size)), stack->element_size);
        Printf_Format_Func(&tmp);
        printf("\n");
        }
        #ifndef NO_CANARY_PROTECTION
        memcpy(&tmp, (uint64_t*)(((char*)(stack->free_data)) +
                                     stack->mem_size_aligned -
                                     sizeof(uint64_t)),
                                     sizeof(uint64_t));
        printf("            right_data_canary (%p) = ", (uint64_t*)(((char*)(stack->free_data)) +
                                                        stack->mem_size_aligned -
                                                        sizeof(uint64_t)));
        printf("%#llx\n", tmp);
        #endif

        printf("        }\n    }\n");
    }

}

#ifndef NO_DEBUG

int StackAssert (Stack_t * stack, const char * file, const char* func, int line) {
    if(stack->Stack_Status) {
        int error_code = Stack_error(stack);
        if(error_code) {
            if(!stack) {

                DEBUG_ON(assert(error_code != Stack_Null_Pointer);)
                return error_code;
            }
            Error_output(error_code, stack->name);
            //Stack_Dump(stack, Printf_Format_Func, file, func, line);
            DEBUG_ON(StackDtor(stack);)
            fprintf(stderr, "Free() is used in 'StackAssert' in %s", file);
            ABORT_SKIP(abort();)
        }
        return error_code;
    }
}

int Stack_error(const Stack_t * stack) {

    int error_code = 0;
    if(!stack) {
        error_code += Stack_Null_Pointer;
        return error_code;
    }
    if(!stack->data && stack->capacity != 0)
        error_code += Data_Null_Pointer;
    if(stack->capacity < 0)
        error_code += Negative_Capacity;
    if(stack->size < 0)
        error_code += Negative_Size;
    if(stack->capacity < stack->size)
        error_code += Capacity_Less_Size;

    #ifndef NO_CANARY_PROTECTION
        if(stack->left_struct_canary != Struct_left_protector)
            error_code += Left_Struct_Canary_Is_Dead;
        if(stack->right_struct_canary != Struct_right_protector)
            error_code += Right_Struct_Canary_Is_Dead;
        uint64_t tmp = 0;
        memcpy(&tmp, (((char*)(stack->data)) - sizeof(uint64_t)), sizeof(uint64_t));
        if(tmp !=  Data_left_protector)
            error_code += Left_Data_Canary_Is_Dead;
        memcpy(&tmp, (((char*)(stack->free_data)) +
                          stack->mem_size_aligned -
                          sizeof(uint64_t)),
                          sizeof(uint64_t));
        if(tmp !=  Data_right_protector)
            error_code += Right_Data_Canary_Is_Dead;
    #endif

    #ifndef NO_HASH_PROTECTION
        if(stack->struct_hash_sum != Hash_sum(stack, ((char*)&(stack->struct_hash_sum))-1))
            error_code += Wrong_Struct_Hash;
        if(stack->data_hash_sum != Hash_sum(stack->free_data,
                                            stack->free_data +
                                            stack->mem_size_aligned - 1))
            error_code += Wrong_Data_Hash;
    #endif
    return error_code;
}

void Error_output (int error_code,const char * name) {

    ERROR_CHECK(error_code, Stack_Null_Pointer);
    ERROR_CHECK(error_code, Data_Null_Pointer);
    ERROR_CHECK(error_code, Negative_Capacity);
    ERROR_CHECK(error_code, Negative_Size);
    ERROR_CHECK(error_code, Capacity_Less_Size);

    #ifndef NO_CANARY_PROTECTION
    ERROR_CHECK(error_code, Left_Struct_Canary_Is_Dead);
    ERROR_CHECK(error_code, Right_Struct_Canary_Is_Dead);
    ERROR_CHECK(error_code, Left_Data_Canary_Is_Dead);
    ERROR_CHECK(error_code, Right_Data_Canary_Is_Dead);
    #endif

    #ifndef NO_HASH_PROTECTION
    ERROR_CHECK(error_code, Wrong_Struct_Hash);
    ERROR_CHECK(error_code, Wrong_Data_Hash);
    #endif
}


size_t Hash_sum(const void * buffer_start, const void * buffer_end) {

    const char* byte_buffer = (const char*) buffer_start;
    size_t len = (const char*)buffer_end - byte_buffer+1;
    size_t h_sum = +7952812;
    for(int i = 0; i < len; i++){

        h_sum = (h_sum*10 + h_sum % 10 + i) ^ (byte_buffer[i]);
    }
    return h_sum;
}

#endif
