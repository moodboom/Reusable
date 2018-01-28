#pragma once

#include "HttpServer.hpp"
#include <utilities.hpp>
#include "HttpsAPIServerConstants.hpp"


// ------------------
// HttpAPIServer
// ------------------
// Extends HttpServer to add support for easy creation of RESTful APIS.
// This server expects a RESTful API defined by an array of API_call objects.
// 
// QUICKSTART:
//
//      HttpAPIServer api_server(...);
//      api_server.vAPI_push_back(API_call( HM_GET, {"v1","login"                      }, {"html"       } )); GetLogin(vAPI_.back());
//      api_server.vAPI_push_back(API_call( HM_GET, {"v1","accounts",":id","analyze"   }, {"html"       } )); Analyze(vAPI_.back());
//      api_server.startServer();
//
//      Define lambdas in the handler functions, see examples for further details.
//
// OR for better SoC:
//
//      1) Create a derived class.
//      2) Provide port, includes, title, shared HTML strings, etc., all within that one class.
//      3) Override startServer() to initialize vAPI_ before calling base.
//
// ------------------


class HttpAPIServer : public HttpServer
{
    typedef HttpServer inherited;

public:
    
    HttpAPIServer(
            
        // required params
        const vector<string>& includes,
        const vector<string>& static_files,
        const string& title,

        // required base params
        unsigned short port, 
        size_t num_threads, 

        // (no https params)

        // defaulted params
        const vector<string>& wrappers = c_default_wrappers,
        bool bServeRootOnError = false,
        bool bServeAPIAtRoot = false,
        int max_body_size = 100000,

        // defaulted base params
        long timeout_request=5, 
        long timeout_content=300,
        const std::string& verify_file=std::string()

    ) :
        // Call base class
        inherited(port,num_threads,timeout_request,timeout_content),
        
        // Init vars
        title_(title),
        wrappers_(wrappers),
        bServeRootOnError_(bServeRootOnError),
        bServeAPIAtRoot_(bServeAPIAtRoot),
        max_body_size_(max_body_size)        
    {
        assert(wrappers_.size() == HW_COUNT);
        set_html_includes(includes);
        add_static_file_handlers(static_files);
    }
    virtual ~HttpAPIServer() {}

    // THE RESTful API
    // Make sure you define this before calling startServer().
    // See top of page for examples.
    vector<API_call> vAPI_;

    // This class provides the following response handling for you:
    //
    //      /favicon.ico    -> returns this file (loaded and cached on startup)
    //      /               -> returns the API definition in a nice html page
    //      (bad request)   -> redirect to /
    //
    // Your derived class should register your handlers before calling this base class to finish up.
    virtual void startServer() {
        
        load_index();
        for (auto& API : vAPI_)
        {
            if (API.load_static_html())
                inject_includes(API);
        }

        createFaviconHandler();
        
        if (bServeAPIAtRoot_)
          createAPIDocumentationHandler();
          
        if (bServeRootOnError_)
          createBadRequestHandler();

        // DEBUG
        /*
        // Write out the available web routes
        log(LV_ALWAYS,string("=================="));
        log(LV_ALWAYS,string("    ROUTES:"));
        log(LV_ALWAYS,string("=================="));
        log(LV_ALWAYS,routes());
        log(LV_ALWAYS,string("=================="));
        */

        inherited::startServer();
    }
    
protected:
    
    // -------
    // Helpers
    // -------
    virtual bool url_upgrade_any_old_semver(string& url)
    {
      // Perform a regex to update the embedded version if needed.
      // We have to see if (1) we have a semver and (2) it is not the current semver.
      // Only then do we take action.
      boost::regex regex("/([v0-9.]+?)/(.*)");
      boost::smatch sm_res;
      if (boost::regex_match(url,sm_res,regex,boost::match_default))
      {
        string incoming_semver(sm_res[1].first,sm_res[1].second);
        if (incoming_semver != semanticVersion())
        {
          url = string("/")+semanticVersion()+"/"+string(sm_res[2].first,sm_res[2].second);
          return true;
        }      
      }
      return false;
    }
    bool tokenize_API_url(const std::string& url, std::string& protocol, std::string& host, API_call& ac);
    bool tokenize_API_querystring(const std::string& querystring, API_call& ac);
    void badCall(HRes &response, const string msg, int delay_secs = 1);
    string requestError(const Request &request, const string msg);

private:
    
    void createFaviconHandler();
    void createAPIDocumentationHandler();
    void createBadRequestHandler();

    // This function performs API self-documentation.
    // We need an index.html file as a basis.
    // Then we automatically inject the API documentation.
    void load_index();
    
    // Called on startup using a constructor parameter
    void set_html_includes(const vector<string>& includes);

    // Static includes are any files that are served up as-is by the webserver.
    void add_static_file_handlers(const vector<string>& static_files);
    
    // Called on startup for each preloaded html file.
    // Search for any include files, and replace with the full script or style.
    void inject_includes(API_call& ac);

    string get_API_html();
    
    string build_path(const string& param);
    string build_param(const string& param);

    std::size_t max_body_size_;
    vector<pair<string,string>> includes_;
    map<const string,const string> static_files_;
    const string title_;
    const vector<string>& wrappers_;
    string favicon_;
    string index_;
    
    bool bServeRootOnError_;
    bool bServeAPIAtRoot_;
};


inline void HttpAPIServer::createFaviconHandler() {

    try
    {
        favicon_ = read_file("htdocs/favicon.ico");
    }
    catch(...)
    {
        log(LV_WARNING,"WARNING: No favicon.ico file was found in [htdocs/].");
    }

    resource["[./]*favicon.ico"]["GET"]=[this](HRes response, HReq request) {
        string content = favicon_;
        string content_type = "\r\nContent-Type: image/x-icon";
        *response << cstr_HTML_HEADER1 << content.length() << content_type << cstr_HTML_HEADER2 << content;
    };
}


inline void HttpAPIServer::add_static_file_handlers(const vector<string>& static_files) {

    // NOTE that these are prefixed with [htdocs] when reading from file system,
    // and expected to be requested with the exact url from clients.
    
    for (auto& file : static_files)
    {
        string body;
        try
        {
            body = read_file(string("htdocs")+file);
            
            // Clean up any static data.
            // Important so during development, js (etc) can be made directly observable in browser.
            replace_with_regex(body,"<!-- DISCARDED BY SERVER BEGIN[\\s\\S]*?DISCARDED BY SERVER END -->","");
        
        }
        catch(...)
        {
            log(LV_ERROR,"*************************************************************");
            log(LV_ERROR,string("ERROR: ")+file+" was not found and cannot be served.");
            log(LV_ERROR,"*************************************************************");

            // Don't allow this!  A file we are expecting got lost, STOP NOW.
            assert(false);            
            // continue;
        }

        // Fix version string in url.  v1 is used as our placeholder.
        string newfile = file;
        replace(newfile,"v1",semanticVersion());
    
        // We keep them all in memory and serve them up like lightning.
        static_files_.insert(make_pair(newfile,body));

        resource[newfile]["GET"]=[this](HRes response, HReq request) {
            const string& body = static_files_[request->path];
            
            // We provide mime types for extensions: css, jss, png, jpg, x-icon
            // Expand as needed.
            
            // NOTE that injecting header data into our header constants requires
            // a CRLF at the START of the header.
            
            string content_type = "\r\nContent-Type: ";
                 if (strings_are_equal(request->path.substr(request->path.length()-4),".css"))
              content_type += "text/css";
            else if (strings_are_equal(request->path.substr(request->path.length()-3),".js"))
              content_type += "text/javascript";
            else if (strings_are_equal(request->path.substr(request->path.length()-4),".png"))
              content_type += "image/png";
            else if (strings_are_equal(request->path.substr(request->path.length()-4),".jpg"))
              content_type += "image/jpeg";
            else if (strings_are_equal(request->path.substr(request->path.length()-4),".svg"))
              content_type += "image/svg+xml";
            else if (strings_are_equal(request->path.substr(request->path.length()-4),".ico"))
              content_type += "image/x-icon";
            else
              content_type += "text/plain";

            // ALWAYS add caching to all static resources.
            // If these resources are dynamic, add an API handler, or use versioning in the url (recommended).
            // TODO Unfathomably, chrome would not cache css for me even when receiving this header (verified in devtools).
            string str_long_cache = "\r\nCache-Control: public, max-age=31536000";
            
            *response << cstr_HTML_HEADER1 << body.length() << content_type << str_long_cache << cstr_HTML_HEADER2 << body;
        };
    }
}


inline void HttpAPIServer::createAPIDocumentationHandler() {
    resource["^/$"]["GET"]=[this](HRes response, HReq request) {
        string content = index_;
        *response << cstr_HTML_HEADER1 << content.length() << cstr_HTML_FULLHEADER2 << content;
    };
}


inline void HttpAPIServer::createBadRequestHandler() {
    default_resource["GET"]=[this](HRes response, HReq request) {
        badCall(response,requestError(*request,"Received unrecognized request"));
    };
}


inline string HttpAPIServer::requestError(const inherited::Request& request, const string msg) 
{ 
    return msg + ": " + request.method + " " + request.path; 
}

inline void HttpAPIServer::badCall(HRes& response, const string msg, int delay_secs)
{
    // We respond to all bad calls with a redirect to the index.
    // Note that this is no place to prevent DDOS - DDOS'ing to legit API calls is trivial.
    log(LV_ERROR,msg);

    // ALWAYS USE HTTP 302 HEADER for redirection!  
    // Not perfect.  But anything else is worse.
    // http://stackoverflow.com/questions/2839585/what-is-correct-http-status-code-when-redirecting-to-a-login-page
    // Avoid this OLD way using spam-abused redirection method:
    //  "<meta http-equiv=\"refresh\" content=\"1; URL='" + redirect + "'\" />";
    *response << cstr_HTML_302_HEADER1 << "/" << cstr_HTML_HEADER2;
}


inline void HttpAPIServer::load_index()
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
inline void HttpAPIServer::set_html_includes(const vector<string>& includes)
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
inline void HttpAPIServer::inject_includes(API_call& ac)
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

inline string HttpAPIServer::get_API_html()
{
    string html;
    for (auto& ac : vAPI_)
    {
        for (auto& type : ac.types_)
        {
            // The method becomes a button that is pressed to execute the call.
            // We need to build a form.

            // ----------------
            // We need to substitute for these strings:
            //
            //  __API_url__, __API_method__, __API_token__
            //
            // Create a copy of the wrappers and S&R in them.
            string url = ac.url();
            if (!type.empty())
            {
                url += ".";
                url += type;
            }
            string method = ac.method();
            vector<string> wrappers = wrappers_;
            for (auto& wr : wrappers)
            {
                replace(wr,"__API_url__",url);
                replace(wr,"__API_method__",method);
            }
            // ----------------

            html += wrappers[HW_LINE_BEGIN];
            html += wrappers[HW_METHOD] + " /";
            for (int n = 0; n < ac.path_tokens_.size(); ++n)
            {
                const string& path = ac.path_tokens_[n];
                if (!path.empty() && path[0] == ':')
                {
                    html += build_param(path.substr(1));

                } else
                {
                    html += build_path(path);
                }
                if (n < ac.path_tokens_.size() - 1)
                {
                    html += "/";
                }
            }
            if (!type.empty())
            {
                html += build_path(string(".")+type);
            }

            string separator(" ? ");
            for (auto& pair : ac.url_params_)
            {
                // Separator + name + the actual param.
                html += build_path(separator);
                html += build_path(pair.first) + "=";
                html += build_param(pair.second);

                separator = " & ";
            }

            html += wrappers[HW_LINE_END];
        }
    }

    return html;
}


// -------
// Helpers
// -------

inline bool HttpAPIServer::tokenize_API_querystring(const std::string& querystring, API_call& ac)
{
    // TODO
    // using namespace boost;

    typedef boost::tokenizer<boost::char_separator<char> > tokenizer;
    boost::char_separator<char> sep("&");
    tokenizer tokens(querystring, sep);
    for (tokenizer::iterator tok_iter = tokens.begin(); tok_iter != tokens.end(); ++tok_iter)
    {
        boost::char_separator<char> sep2("=");
        tokenizer pairs(*tok_iter, sep);
        tokenizer::iterator key_iter = tokens.begin();
        tokenizer::iterator value_iter = key_iter; ++value_iter;
        if (key_iter != tokens.end())
        {
            ac.url_params_[*key_iter] = (value_iter == tokens.end())? "" : *value_iter;
        }
    }
}


inline bool HttpAPIServer::tokenize_API_url(const std::string& url, std::string& protocol, std::string& host, API_call& ac)
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

    // One special case: /
    // Valid, but nothing to do but return true.
    if (url == "/") return true;

    // We must at least have /version/action.  No buffer overflow attempts please.
    if (url.length() < 4 || url.length() > 700)
        return false;

    protocol.clear();
    host.clear();
    ac.path_tokens_.clear();
    ac.url_params_.clear();

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
    // url params
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
        string key, value;
        if (
                url_decode(url.substr(walk1,walk2-walk1),key) 
            &&  url_decode(url.substr(walk2+1,walk3-walk2-1),value))
        {
            ac.url_params_[key] = value;
        }

        // All done if we ran past the end.
        if (walk3 == std::string::npos)
            return true;

        walk1 = walk3 + 1;
    }
    // ===================================

    // We actually shouldn't hit this.
    return false;
}

inline string HttpAPIServer::build_path(const string& param)
{
    // Substitute var as needed.
    string wr = wrappers_[HW_PATH];
    replace(wr,"__API_token__",param);
    return wr;
}

inline string HttpAPIServer::build_param(const string& param)
{
    // Substitute var as needed.
    string wr = wrappers_[HW_PARAM];
    replace(wr,"__API_token__",param);
    return wr;
}
