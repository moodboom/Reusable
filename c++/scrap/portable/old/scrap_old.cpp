#include "scrap_old.hpp"

// MOVE OLD / OUTDATED SCRAP CHUNKS HERE


int main( int argc, char * argv[] )
// ==================================================================================
// ==================================================================================
{
    std::cout << "NOTE: NDEBUG is ";
    #ifndef NDEBUG
        std::cout << "NOT";
    #endif
    std::cout << " defined\n\n";


    // 5 ===================================================================================
    std::cout << "== 5 === stanford 1 ====" << endl;
    // 5 ===================================================================================

    // Algorithms: Design and Analysis, Part 1
	// https://www.coursera.org/course/algo
	// Tim Roughgarden, Associate Professor
	// Stanford University
	//
	// Question 1
	// Download the text file here. (Right click and save link as)
	// This file contains all of the 100,000 integers between 1 and 100,000 (inclusive) in some order, with no integer repeated.
	//
	// Your task is to compute the number of inversions in the file given, where the ith row of the file indicates the ith entry of an array.
	// Because of the large size of this array, you should implement the fast divide-and-conquer algorithm covered in the video lectures.
	// The numeric answer for the given input file should be typed in the space below.
	// So if your answer is 1198233847, then just type 1198233847 in the space provided without any space / commas / any other punctuation marks.
	// You can make up to 5 attempts, and we'll use the best one for grading.
	// (We do not require you to submit your code, so feel free to use any programming language you want ---
	// just type the final numeric answer in the following space.)
	//
	// [TIP: before submitting, first test the correctness of your program on some small test files or your own devising.
	// Then post your best test cases to the discussion forums to help your fellow students!]

	// Got these three right...
	// q1 q1 = n log(n) (same as 50/50 split)
	// q1 q2 = true (constants drop out)
	// q1 q3 = 3 + 4 (by induction)

    // quiz 1 question 4
    // merge two sorted arrays takes c*2n
    // third takes c*(2n+n)
    // last takes c*(n*k)
    // answer should be 0(nk)
	// no... that's just the LAST one, doh! :-)
	// c*2n + c*(2n+n) +...
	// c*2n + c*3n +c*4n + c*5n... c*kn
	// cn(2+3+...k)
	// so the total is...
	// cnk! or cnk^2

    // quiz 1 question 5
    // arrange by growth rate
    // n^2 x log(n)
    // 2^n
    // 2^2^n
    // n^(log(n))
    // n^2

    // n^(log(n)), n^2, n^2 x log(n), 2^n, 2^2^n

    // plug in 10
    // 10, 100, 100, 1024, infinity
	// WRONG
	// i think it's the first one...
	// try 1000
	// yep gotta slide that one out...
    // n^2, n^2 x log(n), n^(log(n)), 2^n, 2^2^n
	// try that...

	// erp, it switched, redo
	// 2^(log(n)), 2^2^(log(n)), n^(5/2), 2^n^2, n^2 x log(n)

	// ranked: 2^(log(n)), n^2 x log(n), n^(5/2), 2^2^(log(n)), 2^n^2

	// check it with 100000, log(n) = 5
	// 15, 5E10, 3e12, 4E9, huge

	// check it with 1000000, log(n) = 6
	// yep, 4th eclipsed the 3rd, nice
	// submit it!

	// 5/5, WINNING, YAY! :-)

	// Now for the programming problem...
	// need to use 64-bit int
	// this was suggested: long long int

	// Programming Question-1
	/*
	Download the text file here. (Right click and save link as)
	This file contains all of the 100,000 integers between 1 and 100,000 (inclusive) in some order, with no integer repeated.

	Your task is to compute the number of inversions in the file given, where the ith row of the file indicates the ith entry of an array.
	Because of the large size of this array, you should implement the fast divide-and-conquer algorithm covered in the video lectures.
	The numeric answer for the given input file should be typed in the space below.
	So if your answer is 1198233847, then just type 1198233847 in the space provided without any space / commas / any other punctuation marks.
	You can make up to 5 attempts, and we'll use the best one for grading.
	(We do not require you to submit your code, so feel free to use any programming language you want ---
	just type the final numeric answer in the following space.)

	[TIP: before submitting, first test the correctness of your program on some small test files or your own devising.
	Then post your best test cases to the discussion forums to help your fellow students!]
	*/

	// easy test:
	//	inversions in a reversed array = 1 + 2 + 3...(n-1) = 4950 for 100 elements
	//	to generate list: seq 100 -1 1 > testReverseOrdered.txt

	// I ended up doing this in QtScrap, see there for details.



    /*
    // 11 ==================================================================================
    std::cout << "== 11 === ptree (bleh) ==" << endl;
    // 11 ==================================================================================
    // JSON array handling with boost ptree
    // NOTE I moved to rapidjson, WAY BETTER THAN BOOST ptree, which loses variable type!!!  SUCKS!!
    // ==================================================================================
    
    // Create a boost property tree consisting of an array.
    ptree pt;
    for (int i = 0; i < 4; ++i)
    {
        ptree area_loop;
        area_loop.put("areaId",i+1);
        area_loop.put("areaName",i==0?"North":i==1?"South":i==2?"Northwest":"Northeast");
        area_loop.put("parentAreaId",i==0?0:i==1?0:1);

        pt.push_back(std::make_pair("",area_loop));
    }

    // Extract as JSON and inspect...
    std::ostringstream buf; 
    write_json (buf, pt, false);
    string str_json_array = buf.str();

    // str_json_array (with whitespace added):
    // {
    //      "":{"areaId":"1","areaName":"North","parentAreaId":"0"},
    //      "":{"areaId":"2","areaName":"South","parentAreaId":"0"},
    //      "":{"areaId":"3","areaName":"Northwest","parentAreaId":"1"},
    //      "":{"areaId":"4","areaName":"Northeast","parentAreaId":"1"}
    // }


    // What if we try to directly parse "good" JSON?
    //
    //  [
    //      {"areaId":1,"areaName":"North","parentAreaId":0},
    //      {"areaId":2,"areaName":"South","parentAreaId":0},
    //      {"areaId":3,"areaName":"Northwest","parentAreaId":1},
    //      {"areaId":4,"areaName":"Northeast","parentAreaId":1}
    // ]
    //
    string good_json =
    "[{\"areaId\":1,\"areaName\":\"North\",\"parentAreaId\":0},{\"areaId\":2,\"areaName\":\"South\",\"parentAreaId\":0},{\"areaId\":3,\"areaName\":\"Northwest\",\"parentAreaId\":1},{\"areaId\":4,\"areaName\":\"Northeast\",\"parentAreaId\":1}]";
    stringstream ssg(good_json);
    ptree ptg;
    read_json(ssg, ptg);
    
    std::ostringstream ss_good_back_out; 
    write_json (ss_good_back_out, ptg, false);
    string str_good_back_out = ss_good_back_out.str();

    // str_good_back_out (whitespace added):
    //  {
    //      "":{"areaId":"1","areaName":"North","parentAreaId":"0"},
    //      "":{"areaId":"2","areaName":"South","parentAreaId":"0"},
    //      "":{"areaId":"3","areaName":"Northwest","parentAreaId":"1"},
    //      "":{"areaId":"4","areaName":"Northeast","parentAreaId":"1"}
    // }


    // So... we can receive and parse "good" JSON arrays just fine.
    // When we want to send arrays, we really need to tweak the final result to make them "good":
    //
    //      convert outer {} to []
    //      remove "":
    //
    // And we're good!
    replace_all(str_good_back_out, "\"\":", "");
    replace_all(str_good_back_out, "{{", "[{");
    replace_all(str_good_back_out, "}}", "}]");

    // NOTE I moved to rapidjson, WAY BETTER THAN BOOST ptree, which loses variable type!!!  SUCKS!!
    */



    // We can keep it running if needed to better see the output.
    // while (true)
    // {}

}
