#include <boost/unordered_map.hpp>
#include <boost/foreach.hpp>
#include <boost/assert.hpp>

#include <iostream>
#include <string>
#include <iterator>
#include <algorithm>
#include <vector>

using namespace boost;
using namespace std;

typedef boost::unordered_map<std::string, int> countmap;
typedef vector<string> vstr;
typedef pair<string,int> strint;

void get_counts(
    vstr& input,
    countmap& counts
) {
    for (vstr::iterator it =  input.begin(); it != input.end(); ++it)
    {
      if (counts.find(*it) != counts.end())
        ++counts[*it];
      else
        counts[*it]=1;
    }
}


bool pair_find_bigger(strint p1,pair<string,int> p2) { return (p1.second>p2.second); }

bool reshuffle(
    int distance,
    vstr& input,
    countmap& counts
) {
    int unique = counts.size();
    if (unique < 2)
        return true;

    // Seems there are several ways to go here.

    // We always need to start with the highest count first, since it might barely fit.
    // So we need to pull out and sort the data in the hashmap.
    // Should we have used a different approach, to sort by count as we pushed?
    // We could have, but I'm not sure it would have been more efficient - 
    // it may have just moved the processing... let's try it this way first...

    // Dang, we have to check for dupes too... use the map for that eh?
    // Once we hit an item we can pull it from map.  Subsequent misses are discarded.
    // There's got to be a better way...
    // An alternative is to keep another list of unique strings every time we 
    // missed the cache... more storage, less processing... let's just keep going...
    // Note to self: check boost multi_index...

    vector<strint> vp;
    BOOST_FOREACH(string str, input)
    {
        if (counts.find(str) != counts.end())
        {
            strint pr(str,counts[str]);
            vp.push_back(pr);
            counts.erase(str);
        }
        if (counts.size() == 0)
            break;
    }
    sort(vp.begin(),vp.end(),pair_find_bigger);

    // DEBUG how does pair vector look?
    /*
    cout<<endl;
    BOOST_FOREACH(strint si, vp)
    {
      cout<<si.first<<" "<<si.second<<endl;
    }
    cout<<endl;
    */

    // If the highest count is 1 or less, there's nothing to do.
    // We need to catch this here and return true;
    if (vp.begin()->second<2)
        return true;

    // Since we have an item with more than one occurance, it will
    // only work if we have more total different items than the distance.
    if (unique<distance)
    {
        input.clear();   
        return false;
    }

    // We should be able to step through map and spit out.  Carefully...
    vstr output;
    // while (vp.begin()->second > 0)
    while (!vp.empty())
    {
        int loop = 0;
        vector<strint>::iterator it = vp.begin();
        while (it!=vp.end() && it->second > 1)
        {
            // cout<<it->first<<endl;
            output.push_back(it->first);

            --it->second;
            ++it;
            ++loop;            
        }
        while (it!=vp.end() && loop<distance)
        {
            assert(it->second > 0);

            if (it==vp.end())
            {
                // Might as well save what we could manage...
                input = output;

                return false;
            }

            // cout<<it->first<<endl;
            output.push_back(it->first);

            // We need to carefully remove the element now.
            // Just decrementing isn't enough...
            // --it->second;
            vector<strint>::iterator it_prev = it;
            --it_prev;
            vp.erase(it);
            it = it_prev;
            ++it;

            ++loop;

            // If we hit the end we need to stop.
            if (vp.empty())
                break;
        }
        // If we ran out of 1's before we went the distance, we failed.
        if (loop<distance)
        {
            // Might as well save what we could manage...
            input = output;

            return false;
        }
    }
    input = output;

    // But can we do it in-place to save memory?
    // Multiple indexes on the original array may be better.
    // I probably won't be able to let this go.
    // But I'll save it for another time.
    // Great question...

    return true;
}


int main( int argc, char * argv[] )
{

    // ======================================
    //            TEST DATA
    // ======================================
    int distance = 3;
    vstr input;
    input.push_back(string("B"));    
    input.push_back(string("A"));    
    input.push_back(string("A"));    
    input.push_back(string("B"));    
    input.push_back(string("B"));    
    input.push_back(string("C"));    
    input.push_back(string("A"));    
    input.push_back(string("AB"));    
    input.push_back(string("B"));    
    input.push_back(string("A"));    
    input.push_back(string("A"));    
    input.push_back(string("B"));    
    input.push_back(string("F"));    
    input.push_back(string("F"));    
    input.push_back(string("I"));    
    // ======================================


    cout<<endl<<"Input"<<endl<<"-------"<<endl;
    cout<<"Distance = "<<distance<<endl;
    BOOST_FOREACH(string str, input)
    {
        cout<<str<<endl;
    }

    if (distance < 2)
    {
        cout<<endl<<"** Please give me a challenging distance..."<<endl;
        return 2;
    }

    countmap counts;
    get_counts(input, counts);

    // DEBUG how does the hashmap look?
    // BOOST_FOREACH(countmap::value_type i, counts) 
    // {
    //     cout<<i.first<<","<<i.second<<endl;
    // }

    if (reshuffle(distance, input, counts))
    {
        
        cout<<endl<<"Results:"<<endl<<"--------"<<endl;
        BOOST_FOREACH(string str, input)
        {
            cout<<str<<endl;
        }
        return 0;

    } else
    {
        cout<<endl<<"* failed, not enough space in array *"<<endl;
        cout<<endl<<"Partial dump:"<<endl;
        BOOST_FOREACH(string str, input)
        {
            cout<<str<<endl;
        }
        return 1;
    }
}
