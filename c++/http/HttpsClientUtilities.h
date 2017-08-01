#pragma once

#include "HttpsClient.hpp"
#include <boost/regex.hpp>

using namespace std;


// ------------------------------------------------------------------------------
// This header contains utilities that depend on HttpsClient.
// They are maintained outside of utilities.hpp to avoid bundling http code where it is not needed.
// ------------------------------------------------------------------------------


// INPUT:
//  regex   provide a regex search string that includes a group; if a match is found, the group will be returned
//          eg, for the rows in a particular table: "<table[^>]+class[^>]+width=\"100[^>]+>([\\s\\S]+?)<[/]table>"
static bool scrape_http(
    const string& domain,
    const string& path,
    const string& regex,
    string& result
) {
  HttpClient client(domain);
  auto http_response = client.make_request(
    "GET",
    path
  );

  if (get_status_code(http_response) != 200)
    return false;

  string body = get_body(http_response);

  boost::match_results<std::string::const_iterator> mresults;
  boost::regex reg;
  reg.assign(regex);

  // We want regex_search, not regex_match (which must match the entire input).
  bool bSuccess = boost::regex_search(body,mresults,reg,boost::match_default);

  if (bSuccess && mresults.size() > 1)
    result = string(mresults[1].first,mresults[1].second);

  return bSuccess;

}


