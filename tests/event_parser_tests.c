#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "ast.h"
#include "minunit.h"
#include "tree.h"
#include "utils.h"
#include "var.h"

int event_parse(const char* text, struct betree_event** event);

bool test_bool_pred(const char* attr, bool value, const struct betree_event* event, size_t index)
{
    const struct betree_variable* pred = event->variables[index];
    return strcmp(pred->attr_var.attr, attr) == 0 && pred->value.value_type == BETREE_BOOLEAN
        && pred->value.boolean_value == value;
}

bool test_integer_pred(const char* attr, int64_t value, const struct betree_event* event, size_t index)
{
    const struct betree_variable* pred = event->variables[index];
    return strcmp(pred->attr_var.attr, attr) == 0 && pred->value.value_type == BETREE_INTEGER
        && pred->value.integer_value == value;
}

bool test_float_pred(const char* attr, double value, const struct betree_event* event, size_t index)
{
    const struct betree_variable* pred = event->variables[index];
    return strcmp(pred->attr_var.attr, attr) == 0 && pred->value.value_type == BETREE_FLOAT
        && feq(pred->value.float_value, value);
}

bool test_string_pred(const char* attr, const char* value, const struct betree_event* event, size_t index)
{
    const struct betree_variable* pred = event->variables[index];
    return strcmp(pred->attr_var.attr, attr) == 0 && pred->value.value_type == BETREE_STRING
        && strcmp(pred->value.string_value.string, value) == 0;
}

bool test_empty_list(const struct betree_variable* pred)
{
    return is_empty_list(pred->value);
}

bool test_integer_list_pred(
    const char* attr, struct betree_integer_list* list, const struct betree_event* event, size_t index)
{
    const struct betree_variable* pred = event->variables[index];
    if(strcmp(pred->attr_var.attr, attr) == 0
        && (test_empty_list(pred) || pred->value.value_type == BETREE_INTEGER_LIST)) {
        if(list->count == pred->value.integer_list_value->count) {
            for(size_t i = 0; i < list->count; i++) {
                if(list->integers[i] != pred->value.integer_list_value->integers[i]) {
                    return false;
                }
            }
            return true;
        }
    }
    return false;
}

bool test_integer_list_pred0(const char* attr, const struct betree_event* event, size_t index)
{
    struct betree_integer_list* list = make_integer_list();
    bool result = test_integer_list_pred(attr, list, event, index);
    free_integer_list(list);
    return result;
}

bool test_integer_list_pred1(const char* attr, int64_t i1, const struct betree_event* event, size_t index)
{
    struct betree_integer_list* list = make_integer_list();
    add_integer_list_value(i1, list);
    bool result = test_integer_list_pred(attr, list, event, index);
    free_integer_list(list);
    return result;
}

bool test_integer_list_pred2(
    const char* attr, int64_t i1, int64_t i2, const struct betree_event* event, size_t index)
{
    struct betree_integer_list* list = make_integer_list();
    add_integer_list_value(i1, list);
    add_integer_list_value(i2, list);
    bool result = test_integer_list_pred(attr, list, event, index);
    free_integer_list(list);
    return result;
}

bool test_string_list_pred(
    const char* attr, struct betree_string_list* list, const struct betree_event* event, size_t index)
{
    const struct betree_variable* pred = event->variables[index];
    if(strcmp(pred->attr_var.attr, attr) == 0
        && (test_empty_list(pred) || pred->value.value_type == BETREE_STRING_LIST)) {
        if(list->count == pred->value.string_list_value->count) {
            for(size_t i = 0; i < list->count; i++) {
                if(strcmp(list->strings[i].string, pred->value.string_list_value->strings[i].string) != 0) {
                    return false;
                }
            }
            return true;
        }
    }
    return false;
}

bool test_string_list_pred0(const char* attr, const struct betree_event* event, size_t index)
{
    struct betree_string_list* list = make_string_list();
    bool result = test_string_list_pred(attr, list, event, index);
    free_string_list(list);
    return result;
}

bool test_string_list_pred1(
    const char* attr, const char* s1, const struct betree_event* event, size_t index)
{
    struct betree_string_list* list = make_string_list();
    struct string_value sv1 = { .string = strdup(s1) };
    add_string_list_value(sv1, list);
    bool result = test_string_list_pred(attr, list, event, index);
    free_string_list(list);
    return result;
}

bool test_string_list_pred2(
    const char* attr, const char* s1, const char* s2, const struct betree_event* event, size_t index)
{
    struct betree_string_list* list = make_string_list();
    struct string_value sv1 = { .string = strdup(s1) };
    struct string_value sv2 = { .string = strdup(s2) };
    add_string_list_value(sv1, list);
    add_string_list_value(sv2, list);
    bool result = test_string_list_pred(attr, list, event, index);
    free_string_list(list);
    return result;
}

bool test_segment_list_pred(
    const char* attr, struct betree_segments* list, const struct betree_event* event, size_t index)
{
    const struct betree_variable* pred = event->variables[index];
    if(strcmp(pred->attr_var.attr, attr) == 0
        && (test_empty_list(pred) || pred->value.value_type == BETREE_SEGMENTS)) {
        if(list->size == pred->value.segments_value->size) {
            for(size_t i = 0; i < list->size; i++) {
                struct betree_segment* target = list->content[i];
                struct betree_segment* value = pred->value.segments_value->content[i];
                if(target->id != value->id || target->timestamp != value->timestamp) {
                    return false;
                }
            }
            return true;
        }
    }
    return false;
}

bool test_segment_list_pred0(const char* attr, const struct betree_event* event, size_t index)
{
    struct betree_segments* list = make_segments();
    bool result = test_segment_list_pred(attr, list, event, index);
    free_segments(list);
    return result;
}

bool test_segment_list_pred1(
    const char* attr, int64_t id1, int64_t timestamp1, const struct betree_event* event, size_t index)
{
    struct betree_segments* list = make_segments();
    struct betree_segment* s1 = make_segment(id1, timestamp1);
    add_segment(s1, list);
    bool result = test_segment_list_pred(attr, list, event, index);
    free_segments(list);
    return result;
}

bool test_segment_list_pred2(const char* attr,
    int64_t id1,
    int64_t timestamp1,
    int64_t id2,
    int64_t timestamp2,
    const struct betree_event* event,
    size_t index)
{
    struct betree_segments* list = make_segments();
    struct betree_segment* s1 = make_segment(id1, timestamp1);
    struct betree_segment* s2 = make_segment(id2, timestamp2);
    add_segment(s1, list);
    add_segment(s2, list);
    bool result = test_segment_list_pred(attr, list, event, index);
    free_segments(list);
    return result;
}

bool test_frequency_list_pred(
    const char* attr, struct betree_frequency_caps* list, const struct betree_event* event, size_t index)
{
    const struct betree_variable* pred = event->variables[index];
    if(strcmp(pred->attr_var.attr, attr) == 0
        && (test_empty_list(pred) || pred->value.value_type == BETREE_FREQUENCY_CAPS)) {
        if(list->size == pred->value.frequency_caps_value->size) {
            for(size_t i = 0; i < list->size; i++) {
                struct betree_frequency_cap* target = list->content[i];
                struct betree_frequency_cap* value = pred->value.frequency_caps_value->content[i];
                if(target->id != value->id || target->timestamp != value->timestamp
                    || target->timestamp_defined != value->timestamp_defined
                    || target->type != value->type || target->value != value->value
                    || strcmp(target->namespace.string, value->namespace.string) != 0) {
                    return false;
                }
            }
            return true;
        }
    }
    return false;
}

bool test_frequency_list_pred0(const char* attr, const struct betree_event* event, size_t index)
{
    struct betree_frequency_caps* list = make_frequency_caps();
    bool result = test_frequency_list_pred(attr, list, event, index);
    free_frequency_caps(list);
    return result;
}

bool test_frequency_list_pred1(const char* attr,
    const char* type1,
    int64_t id1,
    const char* namespace1,
    int64_t timestamp1,
    int64_t value1,
    const struct betree_event* event,
    size_t index)
{
    struct betree_frequency_caps* list = make_frequency_caps();
    struct string_value ns1 = { .string = strdup(namespace1) };
    struct betree_frequency_cap* s1 = make_frequency_cap(type1, id1, ns1, true, timestamp1, value1);
    add_frequency(s1, list);
    bool result = test_frequency_list_pred(attr, list, event, index);
    free_frequency_caps(list);
    return result;
}

bool test_frequency_list_pred2(const char* attr,
    const char* type1,
    int64_t id1,
    const char* namespace1,
    int64_t timestamp1,
    int64_t value1,
    const char* type2,
    int64_t id2,
    const char* namespace2,
    int64_t timestamp2,
    int64_t value2,
    const struct betree_event* event,
    size_t index)
{
    struct betree_frequency_caps* list = make_frequency_caps();
    struct string_value ns1 = { .string = strdup(namespace1) };
    struct betree_frequency_cap* s1 = make_frequency_cap(type1, id1, ns1, true, timestamp1, value1);
    struct string_value ns2 = { .string = strdup(namespace2) };
    struct betree_frequency_cap* s2 = make_frequency_cap(type2, id2, ns2, true, timestamp2, value2);
    add_frequency(s1, list);
    add_frequency(s2, list);
    bool result = test_frequency_list_pred(attr, list, event, index);
    free_frequency_caps(list);
    return result;
}

int test_bool()
{
    struct betree_event* event;
    event_parse("{\"true\": true, \"false\": false}", &event);
    mu_assert(event->variable_count == 2 && test_bool_pred("true", true, event, 0)
            && test_bool_pred("false", false, event, 1),
        "true and false");
    free_event(event);
    return 0;
}

int test_integer()
{
    struct betree_event* event;
    event_parse("{\"positive\": 1, \"negative\": -1}", &event);
    mu_assert(event->variable_count == 2 && test_integer_pred("positive", 1, event, 0)
            && test_integer_pred("negative", -1, event, 1),
        "positive and negative");
    free_event(event);
    return 0;
}

int test_float()
{
    struct betree_event* event;
    event_parse("{\"no decimal\": 1., \"decimal\": 1.2, \"negative\": -1.}", &event);
    mu_assert(event->variable_count == 3 && test_float_pred("no decimal", 1., event, 0)
            && test_float_pred("decimal", 1.2, event, 1)
            && test_float_pred("negative", -1., event, 2),
        "all cases");
    free_event(event);
    return 0;
}

int test_string()
{
    struct betree_event* event;
    event_parse("{\"normal\": \"normal\", \"empty\": \"\"}", &event);
    mu_assert(event->variable_count == 2 && test_string_pred("normal", "normal", event, 0)
            && test_string_pred("empty", "", event, 1),
        "empty or not");
    free_event(event);
    return 0;
}

int test_integer_list()
{
    struct betree_event* event;
    event_parse("{\"single\":[1], \"two\":[1,2], \"empty\":[]}", &event);
    mu_assert(event->variable_count == 3 && test_integer_list_pred1("single", 1, event, 0)
            && test_integer_list_pred2("two", 1, 2, event, 1)
            && test_integer_list_pred0("empty", event, 2),
        "all cases");
    free_event(event);
    return 0;
}

int test_string_list()
{
    struct betree_event* event;
    event_parse("{\"single\":[\"1\"], \"two\":[\"1\",\"2\"], \"empty\":[]}", &event);
    mu_assert(event->variable_count == 3 && test_string_list_pred1("single", "1", event, 0)
            && test_string_list_pred2("two", "1", "2", event, 1)
            && test_string_list_pred0("empty", event, 2),
        "all cases");
    free_event(event);
    return 0;
}

int test_segment()
{
    struct betree_event* event;
    event_parse("{\"single\":[[1,2]], \"two\": [[1,2],[3,4]], \"empty\":[]}", &event);
    mu_assert(event->variable_count == 3 && test_segment_list_pred1("single", 1, 2, event, 0)
            && test_segment_list_pred2("two", 1, 2, 3, 4, event, 1)
            && test_segment_list_pred0("empty", event, 2),
        "all cases");
    free_event(event);
    return 0;
}

int test_frequency()
{
    struct betree_event* event;
    event_parse("{\"single\":[[\"flight\",1,\"ns\",2,3]], \"two\": "
                "[[\"flight\",1,\"ns1\",2,3],[\"flight\",4,\"ns2\",5,6]], \"empty\":[]}",
        &event);
    mu_assert(event->variable_count == 3
            && test_frequency_list_pred1("single", "flight", 1, "ns", 3, 2, event, 0)
            && test_frequency_list_pred2("two", "flight", 1, "ns1", 3, 2, "flight", 4, "ns2", 6, 5, event, 1)
            && test_frequency_list_pred0("empty", event, 2),
        "all cases");
    free_event(event);
    event_parse("{\"advertiser\": [[\"advertiser\",0,\"\",0,0]],"
                "\"advertiser:ip\": [[\"advertiser:ip\",0,\"\",0,0]],"
                "\"campaign\": [[\"campaign\",0,\"\",0,0]],"
                "\"campaign:ip\": [[\"campaign:ip\",0,\"\",0,0]],"
                "\"flight\": [[\"flight\",0,\"\",0,0]],"
                "\"flight:ip\": [[\"flight:ip\",0,\"\",0,0]],"
                "\"product\": [[\"product\",0,\"\",0,0]],"
                "\"product:ip\": [[\"product:ip\",0,\"\",0,0]]}",
        &event);
    mu_assert(event->variable_count == 8
            && test_frequency_list_pred1("advertiser", "advertiser", 0, "", 0, 0, event, 0)
            && test_frequency_list_pred1("advertiser:ip", "advertiser:ip", 0, "", 0, 0, event, 1)
            && test_frequency_list_pred1("campaign", "campaign", 0, "", 0, 0, event, 2)
            && test_frequency_list_pred1("campaign:ip", "campaign:ip", 0, "", 0, 0, event, 3)
            && test_frequency_list_pred1("flight", "flight", 0, "", 0, 0, event, 4)
            && test_frequency_list_pred1("flight:ip", "flight:ip", 0, "", 0, 0, event, 5)
            && test_frequency_list_pred1("product", "product", 0, "", 0, 0, event, 6)
            && test_frequency_list_pred1("product:ip", "product:ip", 0, "", 0, 0, event, 7),
        "all types");
    free_event(event);
    event_parse("{\"new_format\": [[[\"flight\", 1, \"ns\"], 2, 3]]}", &event);
    mu_assert(event->variable_count == 1
            && test_frequency_list_pred1("new_format", "flight", 1, "ns", 3, 2, event, 0),
        "new format");
    free_event(event);
    return 0;
}

int test_null()
{
    struct betree_event* event;
    event_parse("{\"single\": null}", &event);
    mu_assert(event->variable_count == 0, "single null");
    free_event(event);
    event_parse("{\"first\": null, \"second\": 2, \"third\": 3}", &event);
    mu_assert(event->variable_count == 2 && test_integer_pred("second", 2, event, 0)
            && test_integer_pred("third", 3, event, 1),
        "first");
    free_event(event);
    event_parse("{\"first\": 1, \"second\": null, \"third\": 3}", &event);
    mu_assert(event->variable_count == 2 && test_integer_pred("first", 1, event, 0)
            && test_integer_pred("third", 3, event, 1),
        "second");
    free_event(event);
    event_parse("{\"first\": 1, \"second\": 2, \"third\": null}", &event);
    mu_assert(event->variable_count == 2 && test_integer_pred("first", 1, event, 0)
            && test_integer_pred("second", 2, event, 1),
        "third");
    free_event(event);
    return 0;
}

int all_tests()
{
    mu_run_test(test_bool);
    mu_run_test(test_integer);
    mu_run_test(test_float);
    mu_run_test(test_string);
    mu_run_test(test_integer_list);
    mu_run_test(test_string_list);
    mu_run_test(test_segment);
    mu_run_test(test_frequency);
    mu_run_test(test_null);
    return 0;
}

RUN_TESTS()
