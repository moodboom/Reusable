#include <boost/lambda/lambda.hpp>
#include <boost/regex.hpp>
#include <iostream>
#include <string>
#include <iterator>
#include <algorithm>

int main()
{

    // ===================================================================================
    // lamba test.  simple and does not require build of boost, just access to headers.
    /*
    using namespace boost::lambda;
    typedef std::istream_iterator<int> in;
    std::for_each(
                in(std::cin), in(), std::cout << (_1 * 3) << " " );
    */
    // ===================================================================================


    // ===================================================================================
    // regex test.  requires fully-built boost.
    std::string line;
    boost::regex pat( "^Subject: (Re: |Aw: )*(.*)" );
    while (std::cin)
    {
        std::getline(std::cin, line);
        boost::smatch matches;
        if (boost::regex_match(line, matches, pat))
          std::cout << matches[2] << std::endl;
    }
    // ===================================================================================


}
