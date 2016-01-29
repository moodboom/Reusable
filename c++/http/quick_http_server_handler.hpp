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

#include "http_helpers.hpp"
#include "quick_http_components/mime_types.hpp"
#include "quick_http_components/reply.hpp"
#include "quick_http_components/request.hpp"


using namespace QuickHttp;

// This functor creates the server reply for the given request.
// You must override it to provide custom server replies.
class base_server_handler
  : private boost::noncopyable
{
public:

	explicit base_server_handler(
	    const vector<string>& includes,
        const vector<API_call*>& vpAPI,
        int max_body_size = 100000
	) :
        // init vars
	    vpAPI_(vpAPI),
        max_body_size_(max_body_size)
	{
	    load_favicon();
        set_html_includes(includes);
	    for (auto& pAPI : vpAPI_)
	    {
	        if (pAPI->load_static_html())
	            inject_includes(*pAPI);
	    }
	}

	virtual ~base_server_handler()
	{
	    for (auto& pcall : vpAPI_)
	    {
	        delete pcall;
	    }
	}

    virtual string get_API_html() = 0;
    rep.content = "<p>A better Trader API</p>";
    rep.content += get_API_html("<button>","</button>","<button>","</button>","<button>","</button>","<br />");

protected:

	virtual void operator() (const request& req, reply& rep)
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
	    ac.method_ = req.method;
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
	        if (req.uri.substr(req.uri.size()-11,11) == "favicon.ico")
	        {
	            rep.content = favicon_;
	            type = "ico";
                rep.status = reply::ok;

	        } else
	        {
	            // We respond to all bad requests with the API html.
	            // Note that this is no place to prevent DDOS - DDOS'ing to legit API calls is trivial.
	            // Help the user out.
                log(LV_ERROR,string("Received unrecognized request: ") + req.method + " " + req.uri);
                rep.content = get_API_html();
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
        // We may need to build the relative path back to the root, based on the depth of the ac path.
        string relative_path;
        int steps = ac.path_tokens_.size() - 1;
        for (int n=0; n < steps; ++n)
            relative_path += "../";

        for (auto& include : includes_)
        {
            // Update any path to include the proper relative path.
            string target = include.first;
            replace(target,"href=\"",string("href=\"") + relative_path);
            replace(target,"src=\"",string("src=\"") + relative_path);

            // NOTE that this uses "replace" from utilities.hpp.
            replace(ac.static_html_, target, include.second);
        }
    }

    bool b_API_call_matches(const API_call& acDefinition, const API_call& ac, bool bIncludeParamPairs = false)
    {
        if (!strings_are_equal(acDefinition.method_,ac.method_))                                    return false;
        if (acDefinition.path_tokens_.size() != ac.path_tokens_.size())                             return false;

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
                return acloop->handle_call(rep);
        }
        return false;
    }


    string get_API_html(const string& method1, const string& method2, const string& path1, const string& path2, const string& param1, const string& param2, const string& endline)
    {
        string html;
        for (auto& ac : vpAPI_)
        {
            html += method1 + ac->method_ + method2 + " /";
            for (int n = 0; n < ac->path_tokens_.size(); ++n)
            {
                const string& path = ac->path_tokens_[n];
                if (!path.empty() && path[0] == ':')
                    html += path1 + path + path2;
                else
                    html += path;
                if (n < ac->path_tokens_.size() - 1)
                    html += "/";
            }
            // html += ac->action_ + "." + ac->type_;
            for (int n = 0; n < ac->pair_tokens_.size(); ++n)
            {
                const pair<string,string>& tokenpair = ac->pair_tokens_[n];
                if (n==0) html += " ? ";
                else      html += " & ";
                html += tokenpair.first + "=";
                html += param1 + tokenpair.second + param2;
            }
            html += endline;
        }

        return html;
    }


    // Helper
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

    std::size_t max_body_size_;
    const vector<API_call*>& vpAPI_;
    vector<pair<string,string>> includes_;
    string favicon_;
};


#endif // QUICK_HTTP_SERVER_HANDLER_HPP
