#define NOB_IMPL 0
#include "nobuild.h"
#include <assert.h>
#include <stdio.h>

int main(void)
{
    compiler_t *cc;
    INIT_CC(cc);
    SET_CC(cc, "gcc");

    flag_t *flag_list = NULL;

    ADD_FLAG(&flag_list, FLAG("-Wall"));
    assert(strcmp(flag_list->name, "-Wall") == 0);
    
    ADD_FLAG(&flag_list, FLAG("-Wextra"));
    assert(strcmp((flag_list->__next)->name, "-Wextra") == 0);

    object_t target = OBJECT("test/main.c");

    object_t *dependencies = NULL;

    ADD_OBJECT(&dependencies, OBJECT("test/foo.c"));
    assert(strcmp(dependencies->name, "test/foo.c") == 0);
    
    ADD_OBJECT(&dependencies, OBJECT("test/bar.c"));
    assert(strcmp((dependencies->__next)->name, "test/bar.c") == 0);

    build_rule_t *rule = NULL;
    INIT_RULE(rule);
    MAKE_RULE(rule, cc, flag_list, &target, dependencies, "out");
    assert(rule != NULL);

    BUILD(rule);

    cleanup(rule);
    return 0;
}