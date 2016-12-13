#pragma once

#include "HttpsServer.hpp"
#include <utilities.hpp>
#include "HttpsAPIServerConstants.hpp"


// ------------------
// HttpsAPIServer
// ------------------
// Extends HttpsServer to add support for easy creation of RESTful APIS.
// This server expects a RESTful API defined by an array of API_call objects.
// 
// QUICKSTART:
//
//      HttpsAPIServer api_server(...);
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


class HttpsAPIServer : public HttpsServer
{
    typedef HttpsServer inherited;

public:
    
    HttpsAPIServer(
            
        // required params
        const vector<string>& includes,
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
        title_(title),
        wrappers_(wrappers),
        max_body_size_(max_body_size)
        
    {
        assert(wrappers_.size() == HW_COUNT);
        set_html_includes(includes);
    }

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
        createAPIDocumentationHandler();
        createBadRequestHandler();

        inherited::startServer();
    }
    
protected:
    
    void badCall(std::shared_ptr<HttpsServer::Response> response, std::shared_ptr<Request> request);

private:
    
    void createFaviconHandler();
    void createAPIDocumentationHandler();
    void createBadRequestHandler();

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

    string get_API_html()
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
                for (int n = 0; n < ac.pair_tokens_.size(); ++n)
                {
                    // We wrap the bit of non-param html in path tokens.
                    string nonparam;
                    const pair<string,string>& tokenpair = ac.pair_tokens_[n];
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

        badCall(response,request);
        
    };
}


inline void HttpsAPIServer::badCall(std::shared_ptr<HttpsServer::Response> response, std::shared_ptr<HttpsServer::Request> request)
{
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
}


// TODO TOTHINK how to incorporate this stuff....

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

