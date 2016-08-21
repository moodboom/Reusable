//
// This code is based on the HTTP Server 3 boost example from:
//     http://www.boost.org/doc/libs/1_53_0/doc/html/boost_asio/examples.html
//
// Description:
//     HTTP Server 3
//     An HTTP server using a single io_service and a thread pool calling io_service::run().
//
// Ideally we could use the example as-is, but it uses a file-based scheme that is not at all RESTful.
// To update this code when a new example becomes available,
// do a diff between the old and new example code and paste in the changes.
//
//
// Original copyright notice:
//
// request_handler.hpp
// ~~~~~~~~~~~~~~~~~~~
//
// Copyright (c) 2003-2012 Christopher M. Kohlhoff (chris at kohlhoff dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#ifndef QUICK_HTTP_SERVER_HANDLER_HPP
#define QUICK_HTTP_SERVER_HANDLER_HPP

#include <regex>

#include "http_helpers.hpp"
#include "quick_http_components/mime_types.hpp"
#include "quick_http_components/reply.hpp"
#include "quick_http_components/request.hpp"

using namespace QuickHttp;


// ------------------------------------------------------------------------------
// CONSTANTS GLOBALS STATICS
// ------------------------------------------------------------------------------
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
// ------------------------------------------------------------------------------


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

    virtual void operator() (const Request& req, reply& rep)
	{
	    // Let's see what type of request we got.
	    // Slice up the URL and switch on the pieces.
	    // Our tokenizer will validate that the url was in the correct format.

	    std::string protocol, host, action, type;
	    std::vector<std::string> path_tokens;
	    vector<pair<string,string>> pair_tokens;

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


    bool process_API_call(API_call& ac, QuickHttp::reply& rep)
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


#endif // QUICK_HTTP_SERVER_HANDLER_HPP
