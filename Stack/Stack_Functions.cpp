#include "Stack_Functions.h"

void StackCtor (Stack_t * stack, int64_t capacity, size_t element_size
               DEBUG_ON(,const void * const poison_val,const char * name,
                         const char * file, const char * func, int line))
{
    if(!stack) {

        fprintf(stderr, "ERROR: Stack_Null_Pointer\n");
        return;
    }
    if(capacity < 0) {

        fprintf(stderr, "Negative capacity, can't create stack");
        return;
    }

    #ifndef NO_DEBUG
    stack->name = name;
    stack->file = file;
    stack->func = func;
    stack->line = line;
    memcpy(&(stack->poison_val), &poison_val, sizeof(void*));
    #endif

    stack->capacity = capacity;
    stack->mem_size_aligned = capacity*element_size +
                            ((capacity*element_size) % sizeof(uint64_t)) + 2*sizeof(uint64_t);
    stack->element_size = element_size;
    stack->free_data = (char*) calloc(stack->mem_size_aligned, sizeof(char));
    stack->data = (stack->free_data + sizeof(uint64_t));

    assert(stack->data);
    stack->Stack_Status = true;
    #ifndef NO_CANARY_PROTECTION
    // strict aliasing
        memcpy((((char*)(stack->data)) - sizeof(uint64_t)),
               &Data_left_protector,
               sizeof(uint64_t));
        memcpy((((char*)(stack->free_data)) + stack->mem_size_aligned - sizeof(uint64_t)),
               &Data_right_protector,
               sizeof(uint64_t));
        stack->left_struct_canary = Struct_left_protector;
        stack->right_struct_canary = Struct_right_protector;
    #endif

    #ifndef NO_HASH_PROTECTION
        stack->struct_hash_sum = Hash_sum(stack, ((char*)&(stack->struct_hash_sum))-1);
        stack->data_hash_sum = Hash_sum(stack->free_data, stack->free_data +
                                                          stack->mem_size_aligned - 1);
    #endif
}

void StackPush(Stack_t * stack, void * value) {

    if(!stack) {

        STACK_ASSERT(stack);
        return;
    }

    STACK_ASSERT(stack);
    if(stack->Stack_Status) {
        if(stack->size >= stack->capacity-1)
            if(StackResize(stack, "+"))
                return;

        memcpy((char*)stack->data+(stack->size++)*stack->element_size, value, stack->element_size);

        #ifndef NO_HASH_PROTECTION
            stack->struct_hash_sum = Hash_sum(stack, ((char*)&(stack->struct_hash_sum))-1);
            stack->data_hash_sum = Hash_sum(stack->free_data, stack->free_data +
                                                              stack->mem_size_aligned - 1);
        #endif

        STACK_ASSERT(stack);
    }
}

void StackPop(Stack_t * stack, void * return_var) {

    if(!stack) {

        STACK_ASSERT(stack);
        return;
    }
    if(stack->Stack_Status) {
        if(stack->size <= 0) {

            fprintf(stderr, "ERROR: Capacity = 0, can't do StackPop()\n");
            return;
        }

        STACK_ASSERT(stack);

        uint64_t tmp = 0;
        memcpy(&tmp, (char*)stack->data + (stack->size-1)*stack->element_size,
               stack->element_size);

        #ifndef NO_DEBUG
            memcpy((char*)stack->data + (stack->size - 1)*stack->element_size,
                   stack->poison_val,
                   stack->element_size);
        #endif

        stack->size--;


        if(stack->size < (stack->capacity/4))
            if(!StackResize(stack, "-"))
                return;

        #ifndef NO_HASH_PROTECTION
            stack->struct_hash_sum = Hash_sum(stack, ((char*)&(stack->struct_hash_sum))-1);
            stack->data_hash_sum = Hash_sum(stack->free_data, stack->free_data +
                                                              stack->mem_size_aligned - 1);
        #endif

        STACK_ASSERT(stack);

        memcpy(return_var, &tmp, stack->element_size);
    }
}

static bool StackResize(Stack_t * stack, const char * option) {

    size_t prev_mem_size_aligned = stack->mem_size_aligned;

    if(!strcmp(option, "incr") || !strcmp(option, "Incr") ||
       !strcmp(option, "increase") || !strcmp(option, "Increase") ||
       !strcmp(option, "+"))
    {

        stack->capacity *= 2;

        if(stack->capacity == 0)
            stack->capacity = 1;

        DEBUG_ON(printf("\nCapacity was increased\n\n");)

        stack->mem_size_aligned = stack->capacity*stack->element_size +
                               ((stack->capacity*stack->element_size) % sizeof(uint64_t)) +
                                 2*sizeof(uint64_t);
        char* tmp_ptr = NULL;
        tmp_ptr = (char*)realloc(stack->free_data, stack->mem_size_aligned);
        if(tmp_ptr) {

            stack->free_data = tmp_ptr;
            tmp_ptr = NULL;
        }
        else {

            fprintf(stderr, "ERROR: memory was not reallocated\n");
            return false;
        }
        stack->data = (stack->free_data + sizeof(uint64_t));

        #ifndef NO_DEBUG
            uint64_t tmp = 0;
            memcpy(&tmp, stack->poison_val, stack->element_size);
            memset((((char*)(stack->free_data)) + prev_mem_size_aligned - sizeof(uint64_t)),
                    tmp, stack->mem_size_aligned - prev_mem_size_aligned);
        #endif

        #ifndef NO_CANARY_PROTECTION
            memcpy((((char*)(stack->free_data)) +
                         stack->mem_size_aligned -
                         sizeof(uint64_t)),
                         &Data_right_protector,
                         sizeof(uint64_t));
            //*(uint64_t*)(((char*)(stack->free_data)) + stack->mem_size_aligned - sizeof(uint64_t)) = Data_right_protector;
        #endif

        #ifndef NO_HASH_PROTECTION
            stack->struct_hash_sum = Hash_sum(stack, ((char*)&(stack->struct_hash_sum))-1);
            stack->data_hash_sum = Hash_sum(stack->free_data, stack->free_data +
                                                              stack->mem_size_aligned - 1);
        #endif

        STACK_ASSERT(stack);

        return true;
    }
    else {

        fprintf(stderr, "Incorrect option format\n");
        return false;
    }

    if(!strcmp(option, "decr") || !strcmp(option, "Decr") ||
       !strcmp(option, "decrease") || !strcmp(option, "Decrease") ||
       !strcmp(option, "-"))
    {

        stack->capacity /= 2;

        DEBUG_ON(printf("\nCapacity was decreased\n\n");)

        stack->mem_size_aligned = stack->capacity*stack->element_size +
                               ((stack->capacity*stack->element_size) % sizeof(uint64_t)) +
                                 2*sizeof(uint64_t);

        char* tmp_ptr = NULL;
        tmp_ptr = (char*)realloc(stack->free_data, stack->mem_size_aligned);
        if(tmp_ptr) {

            stack->free_data = tmp_ptr;
            tmp_ptr = NULL;
        }
        else {

            fprintf(stderr, "ERROR: memory was not reallocated\n");
            return false;
        }

        stack->data = (stack->free_data + sizeof(uint64_t));

        #ifndef NO_CANARY_PROTECTION
        memcpy((((char*)(stack->free_data)) +
                         stack->mem_size_aligned -
                         sizeof(uint64_t)),
                         &Data_right_protector,
                         sizeof(uint64_t));
        #endif

        #ifndef NO_HASH_PROTECTION
        stack->struct_hash_sum = Hash_sum(stack, ((char*)&(stack->struct_hash_sum))-1);
        stack->data_hash_sum = Hash_sum(stack->free_data, stack->free_data +
                                                          stack->mem_size_aligned - 1);
        #endif
        STACK_ASSERT(stack);

        return true;
    }
    else {

        fprintf(stderr, "Incorrect option format\n");
        return false;
    }
}

void StackDtor (Stack_t * stack) {

    if(!stack) {

        fprintf(stderr, "Stack_t * stack = NULL");
        return;
    }
    if(!stack->data) {

        fprintf(stderr, "StackElem_t * stack->data = NULL");
        return;
    }
    if(stack->Stack_Status)
        free(stack->data);

}
