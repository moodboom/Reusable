#pragma once

// ------------------------------------------------------------------------------
// CONSTANTS GLOBALS STATICS
// ------------------------------------------------------------------------------
typedef enum
{
    // assert( HM_COUNT == 5 );
    HM_FIRST    = 0,
    HM_GET      = HM_FIRST,
    HM_PUT      ,
    HM_POST     ,
    HM_DELETE   ,
    HM_PATCH    ,

    HM_COUNT
} HTML_METHOD;

typedef enum
{
    // assert( HW_COUNT == 5 );
    HW_FIRST            = 0,
    HW_LINE_BEGIN       = HW_FIRST,
    HW_LINE_END         ,
    HW_METHOD           ,
    HW_PATH             ,
    HW_PARAM            ,

    HW_COUNT
} HTML_WRAPPERS_INDEX;

// This default set of API wrappers works well with bootstrap.
// Provide your own as needed, to present your API in any desired style.
// You can use the following variables and a substitution will be done: __API_url__, __API_method__, __API_token__
// assert( HW_COUNT == 5 );
const vector<string> c_default_wrappers =
{
    "<form class=\"api-form\" method=\"__API_method__\" action=\"__API_url__\"><div class=\"form-inline\">",        // HW_LINE_BEGIN
    "</div></form>",                                                                                                // HW_LINE_END
    "<button type=\"submit\" class=\"btn btn-__API_method__\">__API_method__</button><div class=\"form-group\">",   // HW_METHOD
    "<label>__API_token__</label>",                                                                                 // HW_PATH
    " <input type=\"text\" name=\"__API_token__\" class=\"form-control\" placeholder=\"__API_token__\"/> "          // HW_PARAM
};

const string cstr_HTML_HEADER1 = "HTTP/1.1 200 OK\r\nContent-Length: ";
const string cstr_HTML_HEADER2 = "\r\n\r\n";
// ------------------------------------------------------------------------------
