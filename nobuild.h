/**
 * @file
 * @brief The nobuild system.
 *
 * Defines the fundamental types used by nobuild to represent build targets,
 * dependencies, compiler configurations, and associated metadata.
 *
 * @details
 * The nobuild philosophy is based on minimalism ("less is more"), aiming to 
 * keep the build system simple, lightweight, and C/C++ compatible. 
 * 
 * Fields prefixed with '__' are considered private implementation details 
 * and are not part of the public API. These fields may change in future 
 * versions without notice. Users should avoid relying on them directly.
 *
 * All structures are designed to be lightweight and extensible, relying 
 * heavily on linked lists for dynamic collections of (build) objects.
 *
 * @author 
 * Federico Casu
 * 
 * @version
 * 0.1 (Initial version)
 * 
 * @date
 * 04/27/2025
 */

#ifndef __NOBUILD_H__
#define __NOBUILD_H__

#include <stdlib.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <unistd.h>

// The nobuild philosophy follows the idea that "less is more". Still, sometimes
// "cutting too much" is  not the best approach.  That's why we aim  to keep at
// least basic compatibility with both C and C++ compilers, so nobuild can work
// with either one.
#ifdef __cplusplus
#include <cstdio>
#include <cstring>
#define LIKELY      [[likely]]
#define UNLIKELY    [[unlikely]]
#define LINKAGE     extern "C"
#else
#include <stdio.h>
#include <string.h>
#define LINKAGE
#define LIKELY
#define UNLIKELY
#endif  // __cplusplus

#define DEFAULT_CAPACITY (128 - 8)
#define FLAG_CAPACITY   DEFAULT_CAPACITY
#define OBJECT_CAPACITY DEFAULT_CAPACITY
#define CC_CAPACITY     DEFAULT_CAPACITY
#define OUT_CAPACITY    DEFAULT_CAPACITY

/**
 * @struct flag_t
 * @brief Represents a single compiler flag.
 *
 * @note Fields starting with '__' are private implementation
 * details and may change in future versions.
 */
typedef struct __flag {
    char name[FLAG_CAPACITY];   /** The flag text (e.g., "-Wall", "-O2", ...). */
    struct __flag *__next;      /** Pointer to the next flag in the linked list (private). */
} flag_t;

/**
 * @struct compiler_t
 * @brief Represents a compiler command and its associated flags.
 */
typedef struct __compiler {
    char cmd[CC_CAPACITY];      /** The compiler executable name (e.g., "gcc" or "clang"). */
    struct __flag *flags;       /** Linked list of compiler flags. */
} compiler_t;

/**
 * @struct object_t
 * @brief Represents a build object (e.g., a source file or object file).
 *
 * @note Fields starting with '__' are private implementation
 * details and may change in future versions.
 * 
 * Field '__next' is used to build chains of object dependencies.
 */
typedef struct __object {
    char name[OBJECT_CAPACITY];     /** The object name or file path. */
    struct __object *__next;        /** Pointer to the next object in the linked list (private). */
} object_t;

/**
 * @struct build_rule_t
 * @brief Represents a build rule linking targets, dependencies
 *        and compiler configuration.
 */
typedef struct __build_rule {
    struct __object* target;        /** The object being built (target). */
    struct __object* dependencies;  /** Linked list of objects whose 'target' depends on. */
    struct __compiler* cc;          /** Compiler configuration to use. */
    char output[OUT_CAPACITY];      /** The name or path of the generated output file. */
} build_rule_t;



/************************************************************
 * Macros for Object Creation
 ************************************************************/

/**
 * @brief Allocates an anonymous flag_t object.
 */
#define FLAG(__FLAG__)      ((flag_t) { (__FLAG__), NULL })

#define COMPILER(__NAME__)  ((compiler_t) { (__NAME__), NULL })
#define OBJECT(__NAME__)    ((object_t)   { (__NAME__), NULL })



/************************************************************
 * Initialiser Macros
 ************************************************************/

/**
 * @brief Allocates (heap) and initialises a compiler_t object.
 */
#define INIT_CC(__CC_PTR__)                                         \
    (__CC_PTR__) = (compiler_t*) malloc(sizeof(compiler_t));        \
    (__CC_PTR__)->flags = NULL;                                     \
    memset((__CC_PTR__)->cmd, '\0', sizeof(*((__CC_PTR__)->cmd)));

/**
 * @brief Allocates (heap) and initialises a build_rule_t object.
 */
#define INIT_RULE(__RULE_PTR__)                                             \
    (__RULE_PTR__) = (build_rule_t*) malloc(sizeof(build_rule_t));          \
    (__RULE_PTR__)->target = NULL;                                          \
    (__RULE_PTR__)->dependencies = NULL;                                    \
    (__RULE_PTR__)->cc = NULL;                                              \
    memset((__RULE_PTR__)->output, '\0', sizeof(*((__RULE_PTR__)->output)));



/************************************************************
 * Setter Macros
 ************************************************************/

/**
 * @brief Sets the compiler command string in a compiler_t.
 *
 * @param __CC_PTR__ Pointer to the compiler_t object.
 * @param __CC_STR__ Source string to copy into the command field.
 */
#define SET_CC(__CC_PTR__, __CC_STR__)                      \
    strncpy((__CC_PTR__)->cmd, (__CC_STR__),  CC_CAPACITY);

/**
 * @brief Sets the output file path in a build_rule_t.
 *
 * @param __RULE_PTR__  Pointer to the build_rule_t object.
 * @param __OUT_STR__   Output string to copy.
 */
#define SET_OUT(__RULE_PTR__, __OUT_STR__)                      \
    strncpy((__RULE_PTR__)->output, (__OUT_STR__), CC_CAPACITY);




/************************************************************
 * API Functions
 ************************************************************/

/**
 * @brief Adds a flag to a list of flags.
 *
 * @param __flags   Pointer to the head of the flag list.
 * @param __flag    The flag to add.
 */
LINKAGE void ADD_FLAG(flag_t **__flags, const flag_t __flag);

/**
 * @brief Adds an object to a list of objects.
 *
 * @param __objects Pointer to the head of the object list.
 * @param __object  The object to add.
 */
LINKAGE void ADD_OBJECT(object_t **__objects, const object_t __object);

/**
 * @brief Creates a build rule with the given parameters.
 *
 * @param __rule        Pointer to the build rule to populate.
 * @param __compiler    Compiler configuration to use.
 * @param __flags       List of compiler flags.
 * @param __target      Target object to build.
 * @param __dependencies List of dependencies.
 * @param output        Output file path.
 */
LINKAGE void MAKE_RULE(build_rule_t *__rule, compiler_t *__compiler, flag_t *__flags, object_t* __target, object_t* __dependencies, const char* output);

/**
 * @brief Executes the build process according to a build rule.
 *
 * @param rule Pointer to the build rule to execute.
 * @return 0 on success, non-zero on failure.
 */
LINKAGE short BUILD(build_rule_t* rule);

/**
 * @brief Cleans up resources associated with a build rule.
 *
 * @param rule Pointer to the build rule to clean up.
 */
LINKAGE void cleanup(build_rule_t* rule);

short __build(build_rule_t* rule);


#ifdef NOB_IMPL
#define DEFINE_ALLOC_FN(TYPE, __CAPACITY__)             \
static TYPE* __##TYPE##_alloc(const TYPE copy) {        \
    TYPE* allocated = (TYPE*) malloc(sizeof(TYPE));     \
    if (!allocated) LIKELY {                            \
        return NULL;                                    \
    }                                                   \
                                                        \
    size_t len = strlen(copy.name);                     \
    if (len > (__CAPACITY__) - 1) {                     \
        free(allocated);                                \
        return NULL;                                    \
    }                                                   \
    memcpy(allocated->name, copy.name, len);            \
    allocated->name[len] = '\0';                        \
    allocated->__next = NULL;                           \
                                                        \
    return allocated;                                   \
}

DEFINE_ALLOC_FN(flag_t, FLAG_CAPACITY)
DEFINE_ALLOC_FN(object_t, OBJECT_CAPACITY)


#define LIST_PUSH_BACK(TYPE, HEAD, OBJECT)              \
do {                                                    \
    TYPE* ptr = HEAD;                                   \
    TYPE* prev = NULL;                                  \
    for (; ptr != NULL; ptr = ptr->__next) LIKELY {     \
        prev = ptr;                                     \
    }                                                   \
    if (!prev) UNLIKELY {                               \
        HEAD = __##TYPE##_alloc(OBJECT);                \
    } else LIKELY {                                     \
        prev->__next = __##TYPE##_alloc(OBJECT);        \
    }                                                   \
} while(0);


void ADD_FLAG(flag_t **__flags, const flag_t __flag)
{
    LIST_PUSH_BACK(flag_t, *__flags, __flag)
}

void ADD_OBJECT(object_t **__objects, const object_t __object)
{
    LIST_PUSH_BACK(object_t, *__objects, __object)
}

void MAKE_RULE(build_rule_t *__rule, compiler_t *__compiler, flag_t *__flags, object_t* __target, object_t* __dependencies, const char* output)
{
    if (!__rule || !__compiler || !__dependencies) UNLIKELY {
        return;
    }
    
    LIKELY {
        __rule->cc           = __compiler;
        __rule->cc->flags    = __flags;
        SET_OUT(__rule, output);
        __rule->target       = __target;
        __rule->dependencies = __dependencies;
    }
}

void cleanup(build_rule_t* rule)
{
    if (!rule) {
        return;
    }

    object_t* deps = rule->dependencies;
    while (deps) {
        object_t* next = deps->__next;
        free(deps);
        deps = next;
    }

    flag_t* flags = rule->cc->flags;
    while (flags) {
        flag_t* next = flags->__next;
        free(flags);
        flags = next;
    }

    if (rule->cc) {
        free(rule->cc);
    }

    free(rule);
}

short BUILD(build_rule_t* rule)
{
    if (!rule || !rule->cc || !rule->target || !rule->dependencies) {
        return -1;
    }

 #ifdef DEBUG
    char command[1024] = {0};

    // Start with the compiler command
    snprintf(command, sizeof(command), "%s ", rule->cc->cmd);

    // Add flags
    for (flag_t* f = rule->cc->flags; f != NULL; f = f->__next) {
        strncat(command, f->name, sizeof(command) - strlen(command) - 1);
        strncat(command, " ", sizeof(command) - strlen(command) - 1);
    }

    // Add output
    strncat(command, "-o ", sizeof(command) - strlen(command) - 1);
    strncat(command, rule->output, sizeof(command) - strlen(command) - 1);
    strncat(command, " ", sizeof(command) - strlen(command) - 1);

    // Add target
    strncat(command, rule->target->name, sizeof(command) - strlen(command) - 1);

    // Add dependencies
    for (object_t* d = rule->dependencies; d != NULL; d = d->__next) {
        strncat(command, d->name, sizeof(command) - strlen(command) - 1);
        strncat(command, " ", sizeof(command) - strlen(command) - 1);
    }

    printf("%s\n", command);
#endif

    return __build(rule);
}

short __build(build_rule_t* rule)
{
    if (!rule || !rule->cc || !rule->target || !rule->dependencies) UNLIKELY {
        return -1;
    }

    // First, count how many args we need
    int argc = 0;
    for (flag_t* f = rule->cc->flags; f; f = f->__next) LIKELY {
        argc++;
    }

    for (object_t* o = rule->dependencies; o; o = o->__next) LIKELY {
        argc++;
    }

    //   += (compiler + "-o" + output + target + NULL)
    argc += (1        + 1    + 1      + 1      + 1); 

    // Allocate argv array
    char** argv = (char**) calloc(argc, sizeof(char*));
    if (!argv) UNLIKELY {
        perror("calloc");
        return -1;
    }

    int i = 0;
    argv[i++] = rule->cc->cmd; // compiler, e.g., "gcc"

    // Add all flags
    for (flag_t* f = rule->cc->flags; f; f = f->__next) LIKELY {
        argv[i++] = f->name;
    }

    // Output option
    argv[i++] = "-o";
    argv[i++] = rule->output;

    argv[i++] = rule->target->name;

    // Dependencies
    for (object_t* o = rule->dependencies; o; o = o->__next) LIKELY {
        argv[i++] = o->name;
    }

    argv[i] = NULL; // execvp() needs NULL-terminated array

    // Fork process
    pid_t pid = fork();
    if (pid < 0) {
        perror("fork");
        free(argv);
        return -1;
    }

    if (pid == 0) {
        // Child process
        execvp(argv[0], argv);
        perror("execvp"); // Only if execvp fails
        exit(EXIT_FAILURE);
    }

    // Parent process
    int status = 0;
    waitpid(pid, &status, 0);

    free(argv);

    if (WIFEXITED(status) && WEXITSTATUS(status) == 0) {
        return 0; // success
    } else {
        return -1; // failure
    }
}
#endif  // NOB_IMPL

#endif  // __NOBUILD_H__