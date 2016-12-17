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



// API_call
// This class wraps everything that defines a RESTful API call.  It has tons of helpers to manage the call, too.
class API_call
{
public:
    
    API_call(
        HTML_METHOD method,
        vector<string> path_tokens,
        vector<string> types,
        vector<pair<string,string>> pair_tokens = vector<pair<string,string>>(),
        bool b_param_pairs_are_mandatory = true
    ) :
        method_(method),
        path_tokens_(path_tokens),
        types_(types),
        pair_tokens_(pair_tokens),
        b_param_pairs_are_mandatory_(b_param_pairs_are_mandatory)
    {}

    // This is used to create keys etc, but never for full API call objects.
    API_call() 
    {}

    // Call handlers should follow this pattern:
    // 
    //      1) on startup, read a static html skeleton into memory.
    //      2) on call, gather dynamic data and inject into the pre-loaded static html
    //
    // This base class can do (1) for you in this function.
    // It creates a set of static html that mirrors the API structure and the base will read from them.
    // This also allows us to browse the static html directly, essential when working on the html/css/js skeleton.
    inline bool load_static_html();

    string url() const
    {
        string url;

        for (auto& token : path_tokens_)
        {
            url += "/";
            url += token;
        }
        return url;
    }

    string regex() const
    {
        // result regex eg: 
        //  source  /car/:id/:brand/pic.png
        //  regex   "^/car/[0-9].*/[s]/pic.png"
        
        string regex("^");
        for (auto& token: path_tokens_)
        {
            regex += string("/");
            if (token[0] == ':')
            {
                // Special meanings: 
                //  /:int/      only numerals
                //  /:double/   only numerals and [.]
                //  default     any alphanumeric BUT NOTHING ELSE (eg SLASH :-) ).
                //              Reommended to use /:string/ for consistency
                string id = token.substr(1,token.length()-1);
                     if (strings_are_equal(id,"int"))    regex += "[[:digit:]]*";
                else if (strings_are_equal(id,"double")) regex += "[[:digit:].]*";
                else                                     regex += "[[:alnum:].]*";

            } else {
                regex += token;
            }
        }
        regex += string("[.](");
        bool bFirst = true;
        for (auto& type: types_)
        {
            if (bFirst) { bFirst = false; } else { regex += "|"; }
            regex += type;
        }
        regex += ")";
        
        if (!pair_tokens_.empty())
        {
            if (b_param_pairs_are_mandatory_) {
                for (auto& pair : pair_tokens_)
                {
                    regex += string("(?|&)")+pair.first+"=.*";
                }
            } else {
                regex += ".*";
            }
        }
        
        return regex;
    }

    string method() const
    {
        switch (method_)
        {
            case HM_GET      : return "GET"     ;
            case HM_PUT      : return "PUT"     ;
            case HM_POST     : return "POST"    ;
            case HM_DELETE   : return "DELETE"  ;
            case HM_PATCH    : return "PATCH"   ;
            default          : return "Unsupported method";
        }
    }
    void set_method(string m)
    {
             if (m == "GET"    ) method_ = HM_GET      ;
        else if (m == "PUT"    ) method_ = HM_PUT      ;
        else if (m == "POST"   ) method_ = HM_POST     ;
        else if (m == "DELETE" ) method_ = HM_DELETE   ;
        else if (m == "PATCH"  ) method_ = HM_PATCH    ;
        else method_ = HM_COUNT ;
    }

    bool b_has_type(string search_type) const
    {
        for (auto& t : types_)
            if (strings_are_equal(t,search_type))
                return true;

        return false;
    }

    // These test the *first* type, which should be set for valid API calls.
    bool b_no_type() const { return types_.empty(); }
    bool b_type(const string& test_type) const { return !b_no_type() && strings_are_equal(types_[0],test_type); }


    HTML_METHOD method_;
    vector<string>  path_tokens_;
    vector<string>  types_;
    vector<pair<string,string>> pair_tokens_;
    bool b_param_pairs_are_mandatory_;

    string static_html_;
};


bool API_call::load_static_html()
{
    if (types_.empty() || types_[0] != "html")
        return false;

    string filename = "htdocs";

    // NOTE that we do NOT want the actual token here: [auto& token].
    // We want to work with a copy.  So here, we use [auto token].
    // THIS IS AN EXCEPTION TO THE GENERAL RULE.  Be C++11 smart, my friend.  :-)
    for (auto token : path_tokens_)
    {
        assert(token.length() > 0);

        // REST api is documented with a leading colon for caller-provided id-type fields.
        // Remove these from the pathname.
        if (token[0] == ':')
            token = token.substr(1,token.length()-1);

        filename += string("/") + token;
    }
    filename += string(".") + types_[0];

    try
    {
        static_html_ = read_file(filename);
    }
    catch (...)
    {
        log(LV_ERROR,string("Unable to load static html:") + filename);
        return false;
    }
    return true;
}


