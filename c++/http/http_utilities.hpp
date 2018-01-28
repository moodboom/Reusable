#pragma once

#include <Simple-Web-Server/client_https.hpp>
#include <boost/regex.hpp>

using namespace std;


// ------------------------------------------------------------------------------
// This header contains utilities that depend on HttpsClient.
// They are maintained outside of utilities.hpp to avoid bundling http code where it is not needed.
// ------------------------------------------------------------------------------


// Results extraction helpers
static string get_body(std::shared_ptr<Client<HTTP>::Response> response)          { if (response) return response->content.string(); return ""; }
static int32_t get_status_code(std::shared_ptr<Client<HTTP>::Response> response)  { if (response) return boost::lexical_cast<int32_t>(response->status_code.substr(0,3));; return 500; }
static string get_body(std::shared_ptr<Client<HTTPS>::Response> response)         { if (response) return response->content.string(); return ""; }
static int32_t get_status_code(std::shared_ptr<Client<HTTPS>::Response> response) { if (response) return boost::lexical_cast<int32_t>(response->status_code.substr(0,3));; return 500; }

// scrape_website
// --------------
// Scraping websites in general is an ugly brittle job.
// No website publisher thinks of their site as a stable API.
// With that in mind, whenever we scrape, we do it in as fault-tolerant a way as possible.
// That means searching for a snippet of regex, in every case.
//
// EXAMPLE:
//
//  Client<HTTPS> client("google.com");
//  if (
//    scrape_website(
//      client,
//      "",
//      "(Feeling Lucky)"
//      result
//  )
//    cout << result;  // "Feeling Lucky" punk? :-)
//
// INPUT:
//  regex   provide a regex search string that includes a group; if a match is found, the group will be returned
//          eg, for the rows in a particular table: "<table[^>]+class[^>]+width=\"100[^>]+>([\\s\\S]+?)<[/]table>"
//
// REQUIREMENTS
// The common case is to search not just for your target text, but for surrounding text as well.
// This function is designed for such use case.
// With that in mind, you MUST use a GROUP to identify your target text, as that is OFTEN
// but not always needed. Examples:
//
// If you want to find target text that MUST be surrounded by a <div> tag:
//
//    regex: "<div>(My Target Text)</div>"
//
// If you just want a search string, you MUST use a group, eg:
//
//    regex: "(My Target Text)"
//
// NOTE It's become pretty annoying that SWS separates out http from https so severely
// via hard templates with no non-templated base class.
// Results in a shittone of dupe code, the bane of my existence.
// And should be yours too.  Rewrite SWS some day.
// ALWAYS USE A NON-TEMPLATED BASE CLASS for all your templates!
static bool scrape_website_body(
    const string& body,
    const string& regex,
    string& result
) {
    assert(regex.find('(') != std::string::npos);  // Remember to always provide a "(group)" in your regex!
    return find_substring(body,regex,result);
}

static bool scrape_website(
    Client<HTTP>& client,     // Use: Client<HTTP> client("mydomain");
    const string& path,
    const string& regex,
    string& result
) {
  auto http_response = client.request_without_exception("GET",path);
  return
          get_status_code(http_response) == 200
      &&  scrape_website_body(http_response->content.string(),regex,result);
}

static bool scrape_website(
    Client<HTTPS>& client,     // Use: Client<HTTPS> client("mysecuredomain");
    const string& path,
    const string& regex,
    string& result
) {
  auto http_response = client.request_without_exception("GET",path);
  return
          get_status_code(http_response) == 200
      &&  scrape_website_body(http_response->content.string(),regex,result);
}
