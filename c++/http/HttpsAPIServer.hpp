#pragma once

#include "HttpsServer.hpp"
#include <utilities.hpp>
#include "HttpsAPIServerConstants.hpp"


// ------------------
// HttpsAPIServer
// ------------------
// Extends HttpsServer to add support for easy creation of RESTful APIS.
// This server expects a well-defined RESTful API.
// ------------------
class HttpsAPIServer : public HttpsServer
{
    typedef HttpsServer inherited;

public:
    
    HttpsAPIServer(
            
        // required params
        const vector<string>& includes,
        // TODO
        // const vector<API_call*>& vpAPI,
        const string& title,

        // required base params
        unsigned short port, 
        size_t num_threads, 
        const std::string& cert_file, 
        const std::string& private_key_file,

        // defaulted params
        const vector<string>& wrappers = c_default_wrappers,
        int max_body_size = 100000,

        // defaulted base params
        long timeout_request=5, 
        long timeout_content=300,
        const std::string& verify_file=std::string()

    ) :
        // Call base class
        inherited(port,num_threads,cert_file,private_key_file,timeout_request,timeout_content,verify_file),
        
        // Init vars
        // TODO
        // vpAPI_(vpAPI),
        title_(title),
        wrappers_(wrappers),
        max_body_size_(max_body_size)
        
    {}

    virtual void startServer() {
        createFaviconHandler();
        createAPIDocumentationHandler();
        createAPIHandlers();
        createBadRequestHandler();

        inherited::start();   
    }
    
protected:

    // Override in your derived class to provide your API handlers.    
    virtual void createAPIHandlers() {}

private:
    
    // Do not use this in your derived class, use createAPIHandlers() instead.
    // This version provides the following default response handling for you:
    //
    //      /favicon.ico    -> returns this file (loaded and cached on startup)
    //      /               -> returns the API definition
    //      (your handlers) -> your response
    //      (bad request)   -> redirect to /
    //
    
    void createFaviconHandler();
    void createAPIDocumentationHandler();
    void createBadRequestHandler();

    // TODO
    // const vector<API_call> vAPI_;
    
    std::size_t max_body_size_;
    vector<pair<string,string>> includes_;
    const string& title_;
    const vector<string>& wrappers_;
    string favicon_;
    string index_;
};


inline void HttpsAPIServer::createFaviconHandler() {

    try
    {
        favicon_ = read_file("htdocs/favicon.ico");
    }
    catch(...)
    {
        log(LV_WARNING,"WARNING: No favicon.ico file was found in [htdocs/].");
    }

    resource[".*favicon.ico"]["GET"]=[this](std::shared_ptr<HttpsServer::Response> response, std::shared_ptr<HttpsServer::Request> request) {
        log(LV_ALWAYS,string("Received GET request: ") + request->path);

        string content = favicon_;

        request->header.insert(std::make_pair(string("Content-Type"),string("image/x-icon")));
        
        *response << cstr_HTML_HEADER1 << content.length() << cstr_HTML_HEADER2 << content;
    };
}


inline void HttpsAPIServer::createAPIDocumentationHandler() {
    resource["^/"]["GET"]=[this](std::shared_ptr<HttpsServer::Response> response, std::shared_ptr<HttpsServer::Request> request) {
        log(LV_ALWAYS,string("Received GET request for API docs"));

        string content = "TODO: API DOCS";

        request->header.insert(std::make_pair(string("Content-Type"),string("text/html")));
        
        *response << cstr_HTML_HEADER1 << content.length() << cstr_HTML_HEADER2 << content;
    };
}


inline void HttpsAPIServer::createBadRequestHandler() {

    default_resource["GET"]=[this](std::shared_ptr<HttpsServer::Response> response, std::shared_ptr<HttpsServer::Request> request) {

        // We respond to all bad requests with a redirect to the index.
        // Note that this is no place to prevent DDOS - DDOS'ing to legit API calls is trivial.
        // Help the user out.

        string msg = string("Received unrecognized GET request: ") + request->path;
        log(LV_ERROR,msg);

        request->header.insert(std::make_pair(string("Content-Type"),string("text/html")));
        
        string html = "<html><head>";
        html += "<meta http-equiv=\"refresh\" content=\"2; URL='/'\" />";
        html += "</head><body>";
        html += msg;
        html += "</body></html>";
        
        *response << cstr_HTML_HEADER1 << html.length() << cstr_HTML_HEADER2 << html;
    };
}


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







// This functor creates the server reply for the given request.
// The caller will provide custom server replies via the vpAPI parameter.
class server_handler
  : private boost::noncopyable
{
public:

    explicit server_handler(
        const vector<string>& includes,
        const vector<API_call*>& vpAPI,
        const string& title,
        const vector<string>& wrappers = c_default_wrappers,
        int max_body_size = 100000
    ) :
        // init vars
        vpAPI_(vpAPI),
        title_(title),
        wrappers_(wrappers),
        max_body_size_(max_body_size)
    {
        assert(wrappers_.size() == HW_COUNT);
        set_html_includes(includes);
        load_favicon();
        load_index();
        for (auto& pAPI : vpAPI_)
        {
            if (pAPI->load_static_html())
                inject_includes(*pAPI);
        }
    }

    virtual ~server_handler()
    {
        for (auto& pcall : vpAPI_)
        {
            delete pcall;
        }
    }

    virtual void operator() (const ServerBase<HTTPS>::Request& req, ServerBase<HTTPS>::Response& rep)
    {
        // Let's see what type of request we got.
        // Slice up the URL and switch on the pieces.
        // Our tokenizer will validate that the url was in the correct format.

        std::string protocol, host, action, type;
        std::vector<std::string> path_tokens;
        vector<pair<string,string>> pair_tokens;


        // TODO revamp from "http example 3" to SWS
        /*


        // Mark the request bad until we validate.
        rep.status = reply::bad_request;

        // Tokenize, and ignore malformed requests.
        // That means we will have at least one path token and one action (or just the hostname).
        API_call ac;
        ac.set_method(req.method);
        if (tokenize_API_url(req.uri,protocol,host,ac))
        {
            if (process_API_call(ac,rep))
            {
                rep.status = reply::ok;
                if (!ac.types_.empty())
                    type = ac.types_[0];
            }
        }

        if (rep.status == reply::bad_request)
        {
            // There is one hardcoded request that we handle by hand, the annoying favicon.ico.
            if (b_string_ends_in(req.uri,"favicon.ico"))
            {
                rep.content = favicon_;
                type = "ico";
                rep.status = reply::ok;

            } else if (b_string_ends_in(req.uri,"/index.html"))
            {
                rep.content = index_;
                type = "html";
                rep.status = reply::ok;

            } else
            {
                string msg = string("Received unrecognized request: ") + req.method + " " + req.uri;
                log(LV_ERROR,msg);

                // We respond to all bad requests with a redirect to the index.
                // Note that this is no place to prevent DDOS - DDOS'ing to legit API calls is trivial.
                // Help the user out.
                string html = "<html><head>";
                html += "<meta http-equiv=\"refresh\" content=\"2; URL='/index.html'\" />";
                html += "</head><body>";
                html += msg;
                html += "</body></html>";
                rep.content = html;
                type = "html";
                rep.status = reply::ok;
            }
        }

        // Build the headers.
        rep.headers.resize(2);
        rep.headers[0].name = "Content-Length";
        rep.headers[0].value = boost::lexical_cast<std::string>(rep.content.size());
        rep.headers[1].name = "Content-Type";
        rep.headers[1].value = mime_types::extension_to_type(type);

        if (rep.status == reply::bad_request)
        {
        }


        */


    }

    std::size_t max_body_size_;

protected:

    // Called on startup
    inline void load_favicon()
    {
        try
        {
            favicon_ = read_file("htdocs/favicon.ico");
        }
        catch(...)
        {
            log(LV_WARNING,"WARNING: No favicon.ico file was found in [htdocs/].");

            // TODO add an encoded default
        }
    }

    // This function performs API self-documentation.
    // We need an index.html file as a basis.
    // Then we automatically inject the API documentation.
    inline void load_index()
    {
        try
        {

            // Use a temporary ac to inject includes.
            API_call ac;
            ac.static_html_ = read_file("htdocs/index.html");
            inject_includes(ac);
            index_ = ac.static_html_;

            // We have a couple special S&R for the title.
            replace(index_,"<title>Title</title>",string("<title>") + title_ + "</title>");
            replace(index_,"<h1>Title</h1>",string("<h1>") + title_ + "</h1>");

            // Remove test data, replace with actual API data.
            replace_once_with_regex(index_,"<!-- BEGIN SAMPLE DATA[\\s\\S]*END SAMPLE DATA -->",get_API_html());
        }
        catch(...)
        {
            log(LV_WARNING,"WARNING: No index.html file was found in [htdocs/].");
            log(LV_WARNING,"This file is required for the REST self-documentation.");
        }
    }

    // Called on startup using a constructor parameter
    inline void set_html_includes(const vector<string>& includes)
    {
        // We receive a series of filenames.
        // We take the filename and preload it from the local file system.
        // We also save the search string that should be replaced by the preloaded data in load_static_html().
        // KISS: We support css and js, that's it for now.

        try
        {
            for (auto& filename : includes)
            {
                if (filename.substr(filename.size()-4,4) == ".css")
                {
                    includes_.push_back(
                        pair<string,string>(
                            string("<link href=\"") + filename + "\" rel=\"stylesheet\">",
                            string("<style type=\"text/css\">") + read_file(string("htdocs/") + filename) + "</style>"
                        )
                    );

                } else if (filename.substr(filename.size()-3,3) == ".js")
                {
                    includes_.push_back(
                        pair<string,string>(
                            string("<script src=\"") + filename + "\"></script>",
                            string("<script type=\"text/javascript\">") + read_file(string("htdocs/") + filename) + "</script>"
                        )
                    );

                } else
                {
                    // A file was requested to be inline-included but the type is not understood yet.
                    // Add another handler as needed!
                    assert(false);
                }
            }
        }
        catch(...)
        {
            log(LV_ERROR,"ERROR: Some HTML includes were not found.");
        }
    }

    // Called on startup for each preloaded html file.
    // Search for any include files, and replace with the full script or style.
    void inject_includes(API_call& ac)
    {
        // First, clean up test data.
        // We provide a generic way to add data to the html that
        // will automatically be removed on load (often important for live editing).
        replace_with_regex(ac.static_html_,"<!-- DISCARDED BY SERVER BEGIN[\\s\\S]*?DISCARDED BY SERVER END -->","");

        // We may need to build the relative path back to the root, based on the depth of the ac path.
        string relative_path;
        int steps = ac.path_tokens_.size() - 1;
        for (int n=0; n < steps; ++n)
            relative_path += "../";

        for (auto& include : includes_)
        {
            // Update any path to include the proper relative path.
            // 1) build the target url replacement string
            string target = include.first;
            replace(target,"href=\"",string("href=\"") + relative_path);
            replace(target,"src=\"",string("src=\"") + relative_path);
            // 2) replace it in the static html
            replace(ac.static_html_, target, include.second);
        }
    }

    bool b_API_call_matches(const API_call& acDefinition, const API_call& ac, bool bIncludeParamPairs = false)
    {
        if (acDefinition.method_ != ac.method_)
            return false;

        if (acDefinition.path_tokens_.size() != ac.path_tokens_.size())
            return false;

        for (int n = 0; n < ac.path_tokens_.size(); ++n)
        {
            if (acDefinition.path_tokens_[n][0] != ':')
            {
                if (!strings_are_equal(acDefinition.path_tokens_[n],ac.path_tokens_[n]))
                    return false;
            }
        }

        if (acDefinition.b_param_pairs_are_mandatory_)
        {
            if (acDefinition.pair_tokens_.size() != ac.pair_tokens_.size())
                return false;

            for (int n = 0; n < ac.pair_tokens_.size(); ++n)
            {
                if (!strings_are_equal(acDefinition.pair_tokens_[n].first,ac.pair_tokens_[n].first))
                    return false;
            }
        }

        return true;
    }


    bool process_API_call(API_call& ac, ServerBase<HTTPS>::Response& rep)
    {
        for (auto& acloop : vpAPI_)
        {
            if (b_API_call_matches(*acloop,ac))
            {
                // Authenticate using the header.
                if (acloop->setUserFromHeader())
                {
                    return acloop->handle_call(ac,rep);
                }
                else
                {
                    // TODO provide "unauthorized" feedback?
                    return false;
                }
            }
        }
        return false;
    }


    string get_API_html()
    {
        string html;
        for (auto& ac : vpAPI_)
        {
            for (auto& type : ac->types_)
            {
                // The method becomes a button that is pressed to execute the call.
                // We need to build a form.

                // ----------------
                // We need to substitute for these strings:
                //
                //  __API_url__, __API_method__, __API_token__
                //
                // Create a copy of the wrappers and S&R in them.
                string url = ac->url();
                if (!type.empty())
                {
                    url += ".";
                    url += type;
                }
                string method = ac->method();
                vector<string> wrappers = wrappers_;
                for (auto& wr : wrappers)
                {
                    replace(wr,"__API_url__",url);
                    replace(wr,"__API_method__",method);
                }
                // ----------------

                html += wrappers[HW_LINE_BEGIN];
                html += wrappers[HW_METHOD] + " /";
                for (int n = 0; n < ac->path_tokens_.size(); ++n)
                {
                    const string& path = ac->path_tokens_[n];
                    if (!path.empty() && path[0] == ':')
                    {
                        html += build_param(path.substr(1));

                    } else
                    {
                        html += build_path(path);
                    }
                    if (n < ac->path_tokens_.size() - 1)
                    {
                        html += "/";
                    }
                }
                if (!type.empty())
                {
                    html += build_path(string(".")+type);
                }
                for (int n = 0; n < ac->pair_tokens_.size(); ++n)
                {
                    // We wrap the bit of non-param html in path tokens.
                    string nonparam;
                    const pair<string,string>& tokenpair = ac->pair_tokens_[n];
                    if (n==0) nonparam += " ? ";
                    else      nonparam += " & ";
                    nonparam += tokenpair.first + "=";
                    html += build_path(nonparam);

                    // Now the actual param.
                    html += build_param(tokenpair.second);
                }
                html += wrappers[HW_LINE_END];
            }
        }

        return html;
    }

    // -------
    // Helpers
    // -------

    bool tokenize_API_url(const std::string& url, std::string& protocol, std::string& host, API_call& ac)
    {
        // This is not the ultimate URL parser (there are libraries for that when you need it - google-url, StrTk, etc.).
        // It only handles the formats we expect for our RESTful API.
        //
        // These include the following type of urls:
        //
        //      http(s)://server.com/version/action.type?param1=value1&param2=value2
        //      server.com/version/action.type?param1=value1&param2=value2
        //      /version/action.type?param1=value1&param2=value2
        //
        // If we do not find such a format, it does not fit our RESTful API model and we return false.
        // Specifically, we REQUIRE version and action.

        // We must at least have /version/action.  No buffer overflow attempts please.
        if (url.length() < 4 || url.length() > 700)
            return false;

        protocol.clear();
        host.clear();
        ac.path_tokens_.clear();
        ac.pair_tokens_.clear();

        // ===================================
        // protocol
        size_t walk1 = url.find("://",0);
        size_t walk2;
        if (walk1 == std::string::npos)
        {
            walk1 = 0;

        } else
        {
            protocol = url.substr(0,walk1);
            walk1 = walk1 + 3;
        }

        // If all we got was the protocol, that's not enough.
        if (url.length() <= walk1)
            return false;
        // ===================================

        // ===================================
        // host
        if (url[walk1] != '/')
        {
            walk2 = url.find_first_of('/',walk1);
            host = url.substr(walk1,walk2-walk1);

            // If all we got was the host, that's not enough.
            // Actually, we will allow it now, so we can provide a self-documenting API.
            // But we still need to return here.
            if (walk2 == std::string::npos || walk2 == url.length())
                return true;

            walk1 = walk2;
        }
        // ===================================

        // ===================================
        // paths

        // We should always be at the first '/' here, skip over it.
        ++walk1;

        while (walk1 < url.length())
        {
            walk2 = url.find_first_of('/',walk1);
            if (walk2 == std::string::npos)
                break;

            ac.path_tokens_.push_back(url.substr(walk1,walk2-walk1));
            walk1 = walk2 + 1;
        }

        // If we have anything beyond the hostname,
        // we need at least one path token for the API version,
        // and we also need an action.
        if (ac.path_tokens_.size() < 1 || url.length() < walk1 + 1)
            return false;
        // ===================================

        // ===================================
        // action
        // ===================================

        // We need to look for both "." and "?".
        walk2 = url.find_first_of(".?",walk1);
        ac.path_tokens_.push_back(url.substr(walk1,walk2-walk1));

        // It's ok if we stopped at action.
        if (walk2 == std::string::npos)
            return true;
        // ===================================

        // ===================================
        // type

        // We may be at a "." or a "?".  We only have a type if we're at ".".
        walk1 = walk2;
        if (url[walk1] == '.')
        {
            walk2 = url.find_first_of('?',walk1);
            ac.types_.push_back(url.substr(walk1+1,walk2-walk1-1));

            // If we DID have a ".", we should have a type.
            if (ac.types_[0].empty())
                return false;

            // It's ok to finish with no name-value pairs.
            if (walk2 == std::string::npos)
                return true;

            walk1 = walk2;
        }
        // ===================================

        // ===================================
        // pairs
        // break apart "param1=value1&param2=value2" name-value pairs

        // We should always be at the "?" here, skip over it.
        ++walk1;

        while (walk1 < url.length())
        {
            walk2 = url.find_first_of('=',walk1);

            // No good if we didn't have a full pair.
            if (walk2 == std::string::npos)
                return false;

            size_t walk3 = url.find_first_of('&',walk2);
            ac.pair_tokens_.push_back(
                std::pair<string,string>(
                    url.substr(walk1,walk2-walk1),
                    url.substr(walk2+1,walk3-walk2-1)
                )
            );

            // All done if we ran past the end.
            if (walk3 == std::string::npos)
                return true;

            walk1 = walk3 + 1;
        }
        // ===================================

        // We actually shouldn't hit this.
        return false;
    }

    string build_path(const string& param)
    {
        // Substitute var as needed.
        string wr = wrappers_[HW_PATH];
        replace(wr,"__API_token__",param);
        return wr;
    }

    string build_param(const string& param)
    {
        // Substitute var as needed.
        string wr = wrappers_[HW_PARAM];
        replace(wr,"__API_token__",param);
        return wr;
    }

    const vector<API_call*>& vpAPI_;
    vector<pair<string,string>> includes_;
    const string& title_;
    const vector<string>& wrappers_;
    string favicon_;
    string index_;
};
