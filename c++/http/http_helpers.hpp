#pragma once

#include <http/Simple-Web-Server/server_https.hpp>		// For Response

using namespace SimpleWeb;


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
// ------------------------------------------------------------------------------


static bool url_decode(const std::string& in, std::string& out)
{
  out.clear();
  out.reserve(in.size());
  for (std::size_t i = 0; i < in.size(); ++i)
  {
    if (in[i] == '%')
    {
      if (i + 3 <= in.size())
      {
        int value = 0;
        std::istringstream is(in.substr(i + 1, 2));
        if (is >> std::hex >> value)
        {
          out += static_cast<char>(value);
          i += 2;
        }
        else
        {
          return false;
        }
      }
      else
      {
        return false;
      }
    }
    else if (in[i] == '+')
    {
      out += ' ';
    }
    else
    {
      out += in[i];
    }
  }
  return true;
}


class API_call
{
public:
    API_call() {}
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

    // ----------------------
    // CALL HANDLER FUNCTIONS
    // ----------------------
    // Call handlers follow this pattern:
    //      1) on startup, read a static html skeleton into memory.
    //      2) on call, gather dynamic data and inject into the pre-loaded static html

    // For the first, initial work can be done by the base class.
    // Create a set of static html that mirrors the API structure and the base will read from them.
    // This also allows us to browse the static html directly, essential when working on the html/css/js skeleton.
    inline bool load_static_html();

    // Overload this to get the work done!
	// MDM SWS refactor TODO
	// virtual bool handle_call(const API_call& caller, reply& rep) { return false; }
	virtual bool handle_call(const API_call& caller, ServerBase<HTTPS>::Response& rep) { return false; }
    // ----------------------

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


    // TODO add JWT handling HERE in the base class
    // but we need to push AppUser class down here too!
    // And add all JWT handling.
    virtual bool setUserFromHeader() { return true; }

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
