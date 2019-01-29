%{
    #include <stdint.h>
    #include <stdbool.h>
    #include <stdio.h>
    #include <string.h>
    #include "alloc.h"
    #include "ast.h"
    #include "betree.h"
    #include "event_parser.h"
    #include "tree.h"
    #include "value.h"
    struct betree_event *root;
    extern int zzlex();
    void zzerror(void *scanner, const char *s) { (void)scanner; printf("ERROR: %s\n", s); }
#if defined(__GNUC__)
    #pragma GCC diagnostic push
    #pragma GCC diagnostic ignored "-Wswitch-default"
    #pragma GCC diagnostic ignored "-Wshadow"
#endif
%}

// %debug
%pure-parser
%lex-param {void *scanner}
%parse-param {void *scanner}
%define api.prefix {zz}

%{
    int event_parse(const char *text, struct betree_event **event);
%}

%union {
    int token;
    char *string;

    bool boolean_value;
    int64_t integer_value;
    double float_value;
    struct string_value string_value;
    struct betree_integer_list* integer_list_value;
    struct betree_string_list* string_list_value;
    struct betree_segments* segments_list_value;
    struct betree_segment* segment_value;
    struct betree_frequency_caps* frequencies_value;
    struct betree_frequency_cap* frequency_value;

    struct value value;
    struct betree_variable* variable;

    struct betree_event* event;
}

%token<token> EVENT_LCURLY EVENT_RCURLY
%token<token> EVENT_LSQUARE EVENT_RSQUARE
%token<token> EVENT_COMMA
%token<token> EVENT_COLON
%token<token> EVENT_MINUS
%token<token> EVENT_NULL

%token<boolean_value> EVENT_TRUE EVENT_FALSE
%type<boolean_value> boolean
%token<integer_value> EVENT_INTEGER 
%type<integer_value> integer
%token<float_value> EVENT_FLOAT 
%type<float_value> float
%token<string> EVENT_STRING
%type<string_value> string
%type<integer_list_value> empty_list_value
%type<integer_list_value> integer_list_value integer_list_loop
%type<string_list_value> string_list_value string_list_loop
%type<segments_list_value> segments_value segments_loop
%type<segment_value> segment_value
%type<frequencies_value> frequencies_value frequencies_loop
%type<frequency_value> frequency_value;

%type<value> value
%type<variable> variable
%type<event> variable_loop

%start program


%printer { fprintf(yyoutput, "%lld", $$); } <integer_value>
%printer { fprintf(yyoutput, "%.2f", $$); } <float_value>
%printer { fprintf(yyoutput, "%s", $$); } <string>
%printer { fprintf(yyoutput, "%s", $$.string); } <string_value>
%printer { fprintf(yyoutput, "%zu integers", $$.count); } <integer_list_value>
%printer { fprintf(yyoutput, "%zu strings", $$.count); } <string_list_value>
%printer { fprintf(yyoutput, "%zu segments", $$.size); } <segments_list_value>
%printer { fprintf(yyoutput, "%zu caps", $$.size); } <frequencies_value>

%%

program             : EVENT_LCURLY variable_loop EVENT_RCURLY   { root = $2; }

variable_loop       : variable                              { $$ = make_empty_event(); add_variable($1, $$); }
                    | variable_loop EVENT_COMMA variable    { add_variable($3, $1); $$ = $1; }
;       

variable            : EVENT_STRING EVENT_COLON value        { $$ = make_pred($1, INVALID_VAR, $3); bfree($1); }
                    | EVENT_STRING EVENT_COLON EVENT_NULL   { $$ = NULL; bfree($1); }
;

value               : boolean                               { $$.value_type = BETREE_BOOLEAN; $$.boolean_value = $1; }
                    | integer                               { $$.value_type = BETREE_INTEGER; $$.integer_value = $1; }
                    | float                                 { $$.value_type = BETREE_FLOAT; $$.float_value = $1; }
                    | string                                { $$.value_type = BETREE_STRING; $$.string_value = $1; }
                    | empty_list_value                      { $$.value_type = BETREE_INTEGER_LIST; $$.integer_list_value = $1; }
                    | integer_list_value                    { $$.value_type = BETREE_INTEGER_LIST; $$.integer_list_value = $1; }
                    | string_list_value                     { $$.value_type = BETREE_STRING_LIST; $$.string_list_value = $1; }
                    | segments_value                        { $$.value_type = BETREE_SEGMENTS; $$.segments_value = $1; }
                    | frequencies_value                     { $$.value_type = BETREE_FREQUENCY_CAPS; $$.frequency_caps_value = $1; }

boolean             : EVENT_TRUE                            { $$ = true; }
                    | EVENT_FALSE                           { $$ = false; }
;                       

integer             : EVENT_INTEGER                         { $$ = $1; }
                    | EVENT_MINUS EVENT_INTEGER             { $$ = - $2; }
;                       

float               : EVENT_FLOAT                           { $$ = $1; }
                    | EVENT_MINUS EVENT_FLOAT               { $$ = - $2; }
;       

string              : EVENT_STRING                          { $$.string = bstrdup($1); $$.str = INVALID_STR; bfree($1); }

empty_list_value    : EVENT_LSQUARE EVENT_RSQUARE           { $$ = make_integer_list(); }

integer_list_value  : EVENT_LSQUARE integer_list_loop EVENT_RSQUARE       
                                                            { $$ = $2; }

integer_list_loop   : integer                               { $$ = make_integer_list(); add_integer_list_value($1, $$); }
                    | integer_list_loop EVENT_COMMA integer { add_integer_list_value($3, $1); $$ = $1; }
;               

string_list_value   : EVENT_LSQUARE string_list_loop EVENT_RSQUARE        
                                                            { $$ = $2; }

string_list_loop    : string                                { $$ = make_string_list(); add_string_list_value($1, $$); }
                    | string_list_loop EVENT_COMMA string   { add_string_list_value($3, $1); $$ = $1; }
;       

segments_value      : EVENT_LSQUARE segments_loop EVENT_RSQUARE           
                                                            { $$ = $2; }

segments_loop       : segment_value                         { $$ = make_segments(); add_segment($1, $$); }
                    | segments_loop EVENT_COMMA segment_value        
                                                            { add_segment($3, $1); $$ = $1; }
;

segment_value       : EVENT_LSQUARE integer EVENT_COMMA integer EVENT_RSQUARE  
                                                            { $$ = make_segment($2, $4); }

frequencies_value   : EVENT_LSQUARE frequencies_loop EVENT_RSQUARE
                                                            { $$ = $2; }

frequencies_loop    : frequency_value                       { $$ = make_frequency_caps(); add_frequency($1, $$); }
                    | frequencies_loop EVENT_COMMA frequency_value  
                                                            { add_frequency($3, $1); $$ = $1; }
;

frequency_value     : EVENT_LSQUARE EVENT_STRING EVENT_COMMA integer EVENT_COMMA string EVENT_COMMA integer EVENT_COMMA integer EVENT_RSQUARE
                                                            { $$ = make_frequency_cap($2, $4, $6, true, $10, $8); bfree($2); }
                    | EVENT_LSQUARE EVENT_LSQUARE EVENT_STRING EVENT_COMMA integer EVENT_COMMA string EVENT_RSQUARE EVENT_COMMA integer EVENT_COMMA integer EVENT_RSQUARE
                                                            { $$ = make_frequency_cap($3, $5, $7, true, $12, $10); bfree($3); }
;

%%

#if defined(__GNUC__)
#pragma GCC diagnostic pop
#endif

#include "event_lexer.h"

int event_parse(const char *text, struct betree_event** event)
{
    // zzdebug = 1;
    
    // Parse using Bison.
    yyscan_t scanner;
    zzlex_init(&scanner);
    YY_BUFFER_STATE buffer = zz_scan_string(text, scanner);
    int rc = zzparse(scanner);
    zz_delete_buffer(buffer, scanner);
    zzlex_destroy(scanner);
    
    if(rc == 0) {
        *event = root;
        return 0;
    }
    else {
        return -1;
    }
}
