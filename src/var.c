#include "var.h"
#include "betree.h"
#include "error.h"
#include "tree.h"
#include "utils.h"

bool get_variable(betree_var_t var, const struct betree_variable** preds, struct value* value)
{
    const struct betree_variable* pred = preds[var];
    if(pred != NULL) {
        *value = pred->value;
        return true;
    }
    return false;
}

bool get_float_var(betree_var_t var, const struct betree_variable** preds, double* ret)
{
    const struct betree_variable* pred = preds[var];
    if(pred != NULL) {
        *ret = pred->value.float_value;
        return true;
    }
    return false;
}

bool get_string_var(
    betree_var_t var, const struct betree_variable** preds, struct string_value* ret)
{
    const struct betree_variable* pred = preds[var];
    if(pred != NULL) {
        *ret = pred->value.string_value;
        return true;
    }
    return false;
}

bool get_integer_enum_var(
    betree_var_t var, const struct betree_variable** preds, struct integer_enum_value* ret)
{
    const struct betree_variable* pred = preds[var];
    if(pred != NULL) {
        *ret = pred->value.integer_enum_value;
        return true;
    }
    return false;
}

bool get_integer_var(betree_var_t var, const struct betree_variable** preds, int64_t* ret)
{
    const struct betree_variable* pred = preds[var];
    if(pred != NULL) {
        *ret = pred->value.integer_value;
        return true;
    }
    return false;
}

bool get_bool_var(betree_var_t var, const struct betree_variable** preds, bool* ret)
{
    const struct betree_variable* pred = preds[var];
    if(pred != NULL) {
        *ret = pred->value.boolean_value;
        return true;
    }
    return false;
}

bool get_integer_list_var(
    betree_var_t var, const struct betree_variable** preds, struct betree_integer_list** ret)
{
    const struct betree_variable* pred = preds[var];
    if(pred != NULL) {
        *ret = pred->value.integer_list_value;
        return true;
    }
    return false;
}

bool get_string_list_var(
    betree_var_t var, const struct betree_variable** preds, struct betree_string_list** ret)
{
    const struct betree_variable* pred = preds[var];
    if(pred != NULL) {
        *ret = pred->value.string_list_value;
        return true;
    }
    return false;
}

bool get_segments_var(
    betree_var_t var, const struct betree_variable** preds, struct betree_segments** ret)
{
    const struct betree_variable* pred = preds[var];
    if(pred != NULL) {
        *ret = pred->value.segments_value;
        return true;
    }
    return false;
}

bool get_frequency_var(
    betree_var_t var, const struct betree_variable** preds, struct betree_frequency_caps** ret)
{
    const struct betree_variable* pred = preds[var];
    if(pred != NULL) {
        *ret = pred->value.frequency_caps_value;
        return true;
    }
    return false;
}

bool is_empty_list(struct value value)
{
    return (value.value_type == BETREE_INTEGER_LIST || value.value_type == BETREE_STRING_LIST
               || value.value_type == BETREE_SEGMENTS || value.value_type == BETREE_FREQUENCY_CAPS)
        && value.integer_list_value->count == 0 && value.string_list_value->count == 0 && value.segments_value->size == 0
        && value.frequency_caps_value->size == 0;
}
