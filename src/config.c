#include <float.h>
#include <stdio.h>
#include <string.h>

#include "alloc.h"
#include "config.h"
#include "error.h"
#include "hashmap.h"
#include "memoize.h"
#include "utils.h"

struct config* make_config(uint8_t lnode_max_cap, uint8_t partition_min_size)
{
    struct config* config = bcalloc(sizeof(*config));
    if(config == NULL) {
        fprintf(stderr, "%s bcalloc failed\n", __func__);
        abort();
    }
    config->attr_domain_count = 0;
    config->attr_domains = NULL;
    config->lnode_max_cap = lnode_max_cap;
    config->partition_min_size = partition_min_size;
    config->max_domain_for_split = 1000;
    config->string_map_count = 0;
    config->string_maps = NULL;
    config->pred_map = make_pred_map();
    return config;
}

struct config* make_default_config()
{
    return make_config(3, 0);
}

void free_config(struct config* config)
{
    if(config == NULL) {
        return;
    }
    if(config->attr_domains != NULL) {
        for(size_t i = 0; i < config->attr_domain_count; i++) {
            bfree((char*)config->attr_domains[i]->attr_var.attr);
            bfree(config->attr_domains[i]);
        }
        bfree(config->attr_domains);
        config->attr_domains = NULL;
    }
    if(config->integer_maps != NULL) {
        for(size_t i = 0; i < config->integer_map_count; i++) {
            bfree((char*)config->integer_maps[i].attr_var.attr);
            bfree(config->integer_maps[i].integer_values);
        }
        bfree(config->integer_maps);
        config->integer_maps = NULL;
    }
    if(config->string_maps != NULL) {
        for(size_t i = 0; i < config->string_map_count; i++) {
            bfree((char*)config->string_maps[i].attr_var.attr);
            map_deinit(&config->string_maps[i].m);
        }
        bfree(config->string_maps);
        config->string_maps = NULL;
    }
    if(config->pred_map != NULL) {
        free_pred_map(config->pred_map);
        config->pred_map = NULL;
    }
    bfree(config);
}

static struct attr_domain* make_attr_domain(
    const char* attr, betree_var_t variable_id, struct value_bound bound, bool allow_undefined)
{
    struct attr_domain* attr_domain = bcalloc(sizeof(*attr_domain));
    if(attr_domain == NULL) {
        fprintf(stderr, "%s bcalloc faild\n", __func__);
        abort();
    }
    attr_domain->attr_var.attr = bstrdup(attr);
    attr_domain->attr_var.var = variable_id;
    attr_domain->bound = bound;
    attr_domain->allow_undefined = allow_undefined;
    return attr_domain;
}

static void add_attr_domain(
    struct config* config, const char* attr, struct value_bound bound, bool allow_undefined)
{
    betree_var_t variable_id = config->attr_domain_count;
    struct attr_domain* attr_domain = make_attr_domain(attr, variable_id, bound, allow_undefined);
    if(config->attr_domain_count == 0) {
        config->attr_domains = bcalloc(sizeof(*config->attr_domains));
        if(config->attr_domains == NULL) {
            fprintf(stderr, "%s bcalloc failed\n", __func__);
            abort();
        }
    }
    else {
        struct attr_domain** attr_domains = brealloc(
            config->attr_domains, sizeof(*attr_domains) * (config->attr_domain_count + 1));
        if(attr_domains == NULL) {
            fprintf(stderr, "%s brealloc failed\n", __func__);
            abort();
        }
        config->attr_domains = attr_domains;
    }
    config->attr_domains[config->attr_domain_count] = attr_domain;
    config->attr_domain_count++;
}

void add_attr_domain_bounded_i(
    struct config* config, const char* attr, bool allow_undefined, int64_t min, int64_t max)
{
    struct value_bound bound = { .value_type = BETREE_INTEGER, .imin = min, .imax = max };
    add_attr_domain(config, attr, bound, allow_undefined);
}

void add_attr_domain_i(struct config* config, const char* attr, bool allow_undefined)
{
    add_attr_domain_bounded_i(config, attr, allow_undefined, INT64_MIN, INT64_MAX);
}

void add_attr_domain_ie(struct config* config, const char* attr, bool allow_undefined)
{
    add_attr_domain_bounded_ie(config, attr, allow_undefined, SIZE_MAX);
}

void add_attr_domain_bounded_f(
    struct config* config, const char* attr, bool allow_undefined, double min, double max)
{
    struct value_bound bound = { .value_type = BETREE_FLOAT, .fmin = min, .fmax = max };
    add_attr_domain(config, attr, bound, allow_undefined);
}

void add_attr_domain_f(struct config* config, const char* attr, bool allow_undefined)
{
    add_attr_domain_bounded_f(config, attr, allow_undefined, -DBL_MAX, DBL_MAX);
}

void add_attr_domain_b(struct config* config, const char* attr, bool allow_undefined)
{
    struct value_bound bound = { .value_type = BETREE_BOOLEAN, .bmin = false, .bmax = true };
    add_attr_domain(config, attr, bound, allow_undefined);
}

void add_attr_domain_s(struct config* config, const char* attr, bool allow_undefined)
{
    add_attr_domain_bounded_s(config, attr, allow_undefined, SIZE_MAX);
}

void add_attr_domain_bounded_s(
    struct config* config, const char* attr, bool allow_undefined, size_t max)
{
    struct value_bound bound = { .value_type = BETREE_STRING, .smin = 0, .smax = max - 1 };
    add_attr_domain(config, attr, bound, allow_undefined);
}

void add_attr_domain_bounded_ie(
    struct config* config, const char* attr, bool allow_undefined, size_t max)
{
    struct value_bound bound = { .value_type = BETREE_INTEGER_ENUM, .smin = 0, .smax = max - 1 };
    add_attr_domain(config, attr, bound, allow_undefined);
}

void add_attr_domain_il(struct config* config, const char* attr, bool allow_undefined)
{
    add_attr_domain_bounded_il(config, attr, allow_undefined, INT64_MIN, INT64_MAX);
}

void add_attr_domain_bounded_il(
    struct config* config, const char* attr, bool allow_undefined, int64_t min, int64_t max)
{
    struct value_bound bound = { .value_type = BETREE_INTEGER_LIST, .imin = min, .imax = max };
    add_attr_domain(config, attr, bound, allow_undefined);
}

void add_attr_domain_sl(struct config* config, const char* attr, bool allow_undefined)
{
    add_attr_domain_bounded_sl(config, attr, allow_undefined, SIZE_MAX);
}

void add_attr_domain_bounded_sl(
    struct config* config, const char* attr, bool allow_undefined, size_t max)
{
    struct value_bound bound = { .value_type = BETREE_STRING_LIST, .smin = 0, .smax = max - 1 };
    add_attr_domain(config, attr, bound, allow_undefined);
}

void add_attr_domain_segments(struct config* config, const char* attr, bool allow_undefined)
{
    struct value_bound bound = { .value_type = BETREE_SEGMENTS };
    add_attr_domain(config, attr, bound, allow_undefined);
}

void add_attr_domain_frequency(struct config* config, const char* attr, bool allow_undefined)
{
    struct value_bound bound = { .value_type = BETREE_FREQUENCY_CAPS };
    add_attr_domain(config, attr, bound, allow_undefined);
}

const struct attr_domain* get_attr_domain(
    const struct attr_domain** attr_domains, betree_var_t variable_id)
{
    return attr_domains[variable_id];
}

static void add_integer_map(struct attr_var attr_var, struct config* config)
{
    if(config->integer_map_count == 0) {
        config->integer_maps = bcalloc(sizeof(*config->integer_maps));
        if(config->integer_maps == NULL) {
            fprintf(stderr, "%s bcalloc failed\n", __func__);
            abort();
        }
    }
    else {
        struct integer_map* integer_maps
            = brealloc(config->integer_maps, sizeof(*integer_maps) * (config->integer_map_count + 1));
        if(integer_maps == NULL) {
            fprintf(stderr, "%s brealloc failed\n", __func__);
            abort();
        }
        config->integer_maps = integer_maps;
    }
    config->integer_maps[config->integer_map_count].attr_var.attr = bstrdup(attr_var.attr);
    config->integer_maps[config->integer_map_count].attr_var.var = attr_var.var;
    config->integer_maps[config->integer_map_count].integer_value_count = 0;
    config->integer_maps[config->integer_map_count].integer_values = 0;
    config->integer_map_count++;
}

static void add_string_map(struct attr_var attr_var, struct config* config)
{
    if(config->string_map_count == 0) {
        config->string_maps = bcalloc(sizeof(*config->string_maps));
        if(config->string_maps == NULL) {
            fprintf(stderr, "%s bcalloc failed\n", __func__);
            abort();
        }
    }
    else {
        struct string_map* string_maps
            = brealloc(config->string_maps, sizeof(*string_maps) * (config->string_map_count + 1));
        if(string_maps == NULL) {
            fprintf(stderr, "%s brealloc failed\n", __func__);
            abort();
        }
        config->string_maps = string_maps;
    }
    config->string_maps[config->string_map_count].attr_var.attr = bstrdup(attr_var.attr);
    config->string_maps[config->string_map_count].attr_var.var = attr_var.var;
    config->string_maps[config->string_map_count].string_value_count = 0;
    config->string_map_count++;
}

static void add_to_integer_map(struct integer_map* integer_map, int64_t integer)
{
    if(integer_map->integer_value_count == 0) {
        integer_map->integer_values = bcalloc(sizeof(*integer_map->integer_values));
        if(integer_map->integer_values == NULL) {
            fprintf(stderr, "%s bcalloc failed\n", __func__);
            abort();
        }
    }
    else {
        int64_t* integer_values = brealloc(integer_map->integer_values,
            sizeof(*integer_values) * (integer_map->integer_value_count + 1));
        if(integer_values == NULL) {
            fprintf(stderr, "%s brealloc failed\n", __func__);
            abort();
        }
        integer_map->integer_values = integer_values;
    }
    integer_map->integer_values[integer_map->integer_value_count] = integer;
    integer_map->integer_value_count++;
}

static void add_to_string_map(struct string_map* string_map, const char* string)
{
    if(string_map->string_value_count == 0) {
        map_init(&string_map->m);
    }
    map_set(&string_map->m, string, string_map->string_value_count);
    string_map->string_value_count++;
}

betree_ienum_t try_get_id_for_ienum(
    const struct config* config, struct attr_var attr_var, int64_t integer)
{
    for(size_t i = 0; i < config->integer_map_count; i++) {
        if(config->integer_maps[i].attr_var.var == attr_var.var) {
            struct integer_map* integer_map = integer_map = &config->integer_maps[i];
            for(size_t j = 0; j < integer_map->integer_value_count; j++) {
                if(integer_map->integer_values[j] == integer) {
                    return j;
                }
            }
            break;
        }
    }
    return INVALID_IENUM;
}

betree_str_t try_get_id_for_string(
    const struct config* config, struct attr_var attr_var, const char* string)
{
    for(size_t i = 0; i < config->string_map_count; i++) {
        if(config->string_maps[i].attr_var.var == attr_var.var) {
            struct string_map* string_map = string_map = &config->string_maps[i];
            betree_str_t* str = map_get(&string_map->m, string);
            if(str != NULL) {
                return *str;
            }
            break;
        }
    }
    return INVALID_STR;
}

betree_ienum_t get_id_for_ienum(struct config* config, struct attr_var attr_var, int64_t integer, bool always_assign)
{
    struct integer_map* integer_map = NULL;
    for(size_t i = 0; i < config->integer_map_count; i++) {
        if(config->integer_maps[i].attr_var.var == attr_var.var) {
            integer_map = &config->integer_maps[i];
            for(size_t j = 0; j < integer_map->integer_value_count; j++) {
                if(integer_map->integer_values[j] == integer) {
                    return j;
                }
            }
            break;
        }
    }
    if(integer_map == NULL) {
        add_integer_map(attr_var, config);
        integer_map = &config->integer_maps[config->integer_map_count - 1];
    }
    const struct attr_domain* attr_domain
        = get_attr_domain((const struct attr_domain**)config->attr_domains, attr_var.var);
    if(!always_assign && attr_domain->bound.smax + 1 == integer_map->integer_value_count) {
        return INVALID_IENUM;
    }
    add_to_integer_map(integer_map, integer);
    return integer_map->integer_value_count - 1;
}

betree_str_t get_id_for_string(struct config* config, struct attr_var attr_var, const char* string, bool always_assign)
{
    struct string_map* string_map = NULL;
    for(size_t i = 0; i < config->string_map_count; i++) {
        if(config->string_maps[i].attr_var.var == attr_var.var) {
            string_map = &config->string_maps[i];
            betree_str_t* str = map_get(&string_map->m, string);
            if(str != NULL) {
                return *str;
            }
            break;
        }
    }
    if(string_map == NULL) {
        add_string_map(attr_var, config);
        string_map = &config->string_maps[config->string_map_count - 1];
    }
    const struct attr_domain* attr_domain
        = get_attr_domain((const struct attr_domain**)config->attr_domains, attr_var.var);
    if(!always_assign && attr_domain->bound.smax + 1 == string_map->string_value_count) {
        return INVALID_STR;
    }
    add_to_string_map(string_map, string);
    return string_map->string_value_count - 1;
}

bool is_variable_allow_undefined(const struct config* config, const betree_var_t variable_id)
{
    return config->attr_domains[variable_id]->allow_undefined;
}

