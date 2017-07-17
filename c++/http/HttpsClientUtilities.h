#pragma once

#include "HttpsClient.hpp"
#include <boost/regex.hpp>

using namespace std;


// ------------------------------------------------------------------------------
// This header contains utilities that depend on HttpsClient.
// They are maintained outside of utilities.hpp to avoid bundling http code where it is not needed.
// ------------------------------------------------------------------------------


static bool scrape_url(const string& url, const string& regex, string& result)
{
    boost::match_results<std::string::const_iterator> mresults;
    boost::regex reg;
    reg.assign(regex);
    
    // We want regex_search, not regex_match (which must match the entire input).
    bool bSuccess = boost::regex_search(str,mresults,reg,boost::match_default);

    if (bSuccess && what.size() > 1)
      result = string(what[1].first,what[1].second);
      
    return bSuccess;

}

