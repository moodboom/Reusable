#ifndef utilities_hpp
#define utilities_hpp

#include "basic_types.hpp"
#include "miniz.h"


//=========================================================
// STRING HELPERS
//=========================================================
// Use this.  A lot.
// cout << boost::format("[%8.3f|%8.3f] bought at [%8.3f]") % d_stop_price_ % d_bump_price_ % d_price_paid_;
#include <boost/format.hpp>

// Create a global stringstream that we can reuse (and reuse and reuse and reuse... this thing is used ALL THE TIME!)
extern stringstream g_ss;

// Case-insensitive string comparison
// NOTE that to properly handle UTF-8 the source strings should be normalized/"fully decomposed"/whatever.
// That's why I set this up, so that we could handle that in one place if needed in the future.
static bool strings_are_equal(const string& s1, const string& s2, bool b_case_insensitive = true)
{
    if (b_case_insensitive)
        return boost::iequals(s1,s2);
    else
        return s1 == s2;
}
// Quick and easy simple global search and replace.
static bool replace(string& str, const string& from, const string& to)
{
    size_t start_pos = str.find(from);
    if(start_pos == std::string::npos)
        return false;
    while(start_pos != std::string::npos)
    {
        str.replace(start_pos, from.length(), to);
        start_pos = str.find(from,start_pos+to.length());
    }
    return true;
}
// If you know there is only one search term to replace,
// this is obviously faster than replace() (above).
static bool replace_once(string& str, const string& from, const string& to)
{
    size_t start_pos = str.find(from);
    if(start_pos != std::string::npos)
    {
        str.replace(start_pos, from.length(), to);
        return true;
    }
    return false;
}
// Replace regex
// The search term can go across lines if needed,
// depending on the "from" search term.
// Typically use "[\\s\\S]*" to accomplish this.
#include <boost/regex.hpp>
static bool replace_with_regex(string& str, const string& from, const string& to)
{
    // ----------
    // This would be a nice clean new way, using c++11.
    // But you need a swap string (or you get a crash with gcc 4.9.2).
    // And multiline searching using "TEST DATA[\\s\\S]*TEST DATA" was not successful.
    // We'll stick with boost for now.
    /*
    #include <regex>                                // For replace_with_regex() etc.
    std::regex reg(from);
    string newstr = std::regex_replace(str,reg,to);
    str = newstr;
    return true;
    */
    // ----------

    boost::regex reg;
    reg.assign(from);
    str = boost::regex_replace(str,reg,to,boost::match_default);
    return true;
}
static bool replace_once_with_regex(string& str, const string& from, const string& to)
{
    boost::regex reg;
    reg.assign(from);
    str = boost::regex_replace(str,reg,to,boost::match_default | boost::format_first_only);
    return true;
}
static bool b_string_ends_in(const string& source, const string& search)
{
    return (
            !search.empty()                     // NOTE that we fail if search string is empty
        &&  source.size() >= search.size()
        &&  source.substr(source.size()-search.size(),search.size()) == search
    );
}
//=========================================================


//=========================================================
//  VERSIONING
//=========================================================
// Started here: http://stackoverflow.com/questions/2941491/compare-versions-as-strings
// Will extract up to 4 single-char-separated integers so multiple strings can be compared.
// Source can be...
//
//      0.0.0.0.*  (that's a regex, with . = any character)
//      0.0.0.*
//      0.0.*
//      0.*
//
// So for example, this will extract [0,58,1] from [git describe] versions like 0.58-1-gacd6168.
//
// Following the precise definitions at semver.org,
// versions are only and always compatible if their MAJOR versions match.
// My versioning guidelines dictate that MINOR version differences mean
// a db upgrade can and should be performed.
// See http://bitpost.com/news/?p=1989.
static void VersionParse(int64_t result[4], const std::string& input)
{
    std::istringstream parser(input);
    for(int idx = 0; idx < 4; idx++)
    {
        if (idx > 0) parser.get(); // Skip ANY single character.
        parser >> result[idx];
        if (parser.eof()) { for (int idxz = idx; idxz < 4; idxz++) result[idxz] = 0; return; }  // Quit if we hit the end.
    }
}
static bool bVersionLessThan(const std::string& a,const std::string& b)
{
    int64_t parsedA[4], parsedB[4];
    VersionParse(parsedA, a);
    VersionParse(parsedB, b);
    return bVersionLessThan(parsedA,parsedB);
}
static bool bVersionLessThan(const int64_t& v1[4], const int64_t& v2[4])
{
    return std::lexicographical_compare(v1, v1 + 4, v2, v2 + 4);
}
static bool bVersionsAreCompatible(const std::string& a,const std::string& b)
{
    int64_t parsedA[4], parsedB[4];
    VersionParse(parsedA, a);
    VersionParse(parsedB, b);
    return bVersionsAreCompatible(parsedA,parsedB);
}
static bool bVersionsAreCompatible(const int64_t& v1[4], const int64_t& v2[4])
{
    return (v1[0] == v2[0]);  // same MAJOR
}
static bool bVersionNeedsUpgrade(const std::string& vDB,const std::string& vApp)
{
    // The caller should ensure that the db version is less than or equal to the app version before getting here.
    assert(!bVersionLessThan(vApp,vDB));

    int64_t parsedDB[4], parsedApp[4];
    VersionParse(parsedDB, vDB);
    VersionParse(parsedApp, vApp);
    return bVersionNeedsUpgrade(parsedDB,parsedApp);
}
static bool bVersionNeedsUpgrade(const int64_t& vDB[4], const int64_t& vApp[4])
{
    return (
            bVersionsAreCompatible(vDB,vApp)
        &&  (vDB[1] < vApp[1])     // lower MINOR
    );
}
//=========================================================


//=========================================================
//  TIME
//=========================================================
static ptime get_current_time()
{
    // return second_clock::local_time();       // LOCAL
    return second_clock::universal_time();      // UTC
}
static ptime string_to_ptime(const string& str_time, const string& str_format)
{
    std::istringstream is(str_time);
    is.imbue(std::locale(std::locale::classic(),new boost::posix_time::time_input_facet(str_format)));
    ptime result;
    is >> result;
    return result;

    // NOTE that this doesn't know about the "T"...
    // createdOn_ = boost::posix_time::from_iso_string(createdOn);
}
static ptime iso_string_to_ptime(const string& str_time)
{
    // Config for UTC format, eg: 2014-02-23T10:11:19Z
    return string_to_ptime(str_time,"%Y-%m-%dT%H:%M:%S");
}
static string ptime_to_string(const ptime& pt, const string& str_format)
{
    std::ostringstream is;
    is.imbue(std::locale(std::locale::classic(),new boost::posix_time::time_facet(str_format.c_str())));
    is << pt;
    return is.str();
}
static time_t ptime_to_time_t(const ptime& pt)
{
    // Wow this is a lot of work.
    ptime epoch(boost::gregorian::date(1970,1,1));
    time_duration::sec_type x = (pt - epoch).total_seconds();
    return time_t(x);
}
static ptime time_t_to_ptime(const time_t& tt)
{
    return from_time_t(tt);
}
static string time_t_to_string(const time_t& tt, const string& str_format)
{
    return ptime_to_string(time_t_to_ptime(tt), str_format);
}
static time_t get_current_time_t()
{
    return ptime_to_time_t(get_current_time());
}


static std::string generate_uuid()
{
	boost::uuids::random_generator rg;
	boost::uuids::uuid uuid = rg();
	return boost::uuids::to_string(uuid);
}

static std::string generate_random_hex(uint_fast32_t length)
{
    uint_fast32_t l = 0;
    std::string result;
    while (l < length)
    {
        std::string temp = generate_uuid();
        boost::replace_all(temp, "-", "");              // Remove uuid dashes
        uint_fast32_t tl = temp.length();
        result += temp.substr(0,min(tl,length-l));      // Grab what's needed but no more
        l += tl;
    }
    return result;
}


//=========================================================
// LOGGING TO FILE
//=========================================================
// These static functions log to "log.txt" in the current dir.
//
//  eg:     log(LV_DEBUG,"My complete message");
//
//          stringstream ss;
//          ss << "My " << 3 << "-part message";
//          log(LV_ALWAYS,ss.str());
//
//          // above method is preferred
//          log(LV_INFO,"My ",false,true);
//          log(LV_INFO,3,false,true);
//          log(LV_INFO,"-part message");
//
// When logging, provide a verbosity level that the user can use to control ...err... verbosity.
// The lower the level (LV_DEBUG being the lowest), the more logging you will get.
//
//=========================================================
typedef enum
{
    LV_DEBUG,
    LV_DETAIL,
    LV_INFO,
    LV_WARNING,
    LV_ERROR,
    LV_ALWAYS,

    // Search for this on any changes: 
    // assert(LOG_TO_FILE_VERBOSITY_COUNT == 6);

    LOG_TO_FILE_VERBOSITY_COUNT
} LOG_TO_FILE_VERBOSITY;

extern LOG_TO_FILE_VERBOSITY g_current_log_verbosity;
extern string g_base_log_filename;

static void log(LOG_TO_FILE_VERBOSITY v, string str, bool b_suppress_console = false, bool b_suppress_newline = false, bool b_suppress_file = false, int indent = 0)
{
    static boost::mutex log_guard_;
    
    if (v >= g_current_log_verbosity)
    {
        // Lock and open file for appended output.
        boost::mutex::scoped_lock scoped_lock(log_guard_);
        boost::filesystem::ofstream ofs_log(g_base_log_filename + ".log",boost::filesystem::ofstream::out | boost::filesystem::ofstream::app);

        for (int loop=0;loop<indent;++loop)
        {
            if (!b_suppress_file)
                ofs_log << " ";
            if (!b_suppress_console)
                cout << " ";
        }

        if (!b_suppress_file)
            ofs_log << str;
        if (!b_suppress_console)
            cout << str;

        // NOTE that [<< endl] will do a flush.
        // If we don't do a newline, we should add in a [<< std::flush] instead.
        if (!b_suppress_newline)
        {
            if (!b_suppress_file)
                ofs_log << endl;
            if (!b_suppress_console)
                cout << endl;
            else if (v == LV_ERROR)            // Special case: If this was the end of an ERROR that didn't go to console, pip the output as an alert to user.
                cout << "#" << std::flush;
        } else
            cout << std::flush;
    }
}
static void log(LOG_TO_FILE_VERBOSITY v, int n, bool b_suppress_console = false, bool b_suppress_newline = false, int indent = 0)
{
    log(v,lexical_cast<string>(n),b_suppress_console,b_suppress_newline,indent);
}

// Declarations with default params.
static bool backup_any_old_file(const string& filename, const string& prefix = "", const string& suffix = "");
static bool archive_any_old_file(const string& filename, const string& prefix = "", const string& suffix = "");

static void archive_any_old_log_file()
{
    archive_any_old_file(g_base_log_filename+".log","backup/",string("__old__") + generate_uuid() + ".log");
}
static bool set_log_verbosity(string str_v)
{
    bool b_return = false;

    assert(LOG_TO_FILE_VERBOSITY_COUNT == 6);
    if (strings_are_equal(str_v,"DEBUG"     )) { g_current_log_verbosity = LV_DEBUG     ; b_return = true; }
    if (strings_are_equal(str_v,"DETAIL"    )) { g_current_log_verbosity = LV_DETAIL    ; b_return = true; }
    if (strings_are_equal(str_v,"INFO"      )) { g_current_log_verbosity = LV_INFO      ; b_return = true; }
    if (strings_are_equal(str_v,"WARNING"   )) { g_current_log_verbosity = LV_WARNING   ; b_return = true; }
    if (strings_are_equal(str_v,"ERROR"     )) { g_current_log_verbosity = LV_ERROR     ; b_return = true; }
    if (strings_are_equal(str_v,"OFF"       )) { g_current_log_verbosity = LV_ALWAYS    ; b_return = true; }

    if (b_return)
    {
        stringstream ss;
        ss << "Log level: " << str_v;
        log(LV_ALWAYS, ss.str());

    } else
        log(LV_WARNING,string("Log verbosity change request to ") + str_v + " failed...");

    return b_return;
}
//=========================================================


//=========================================================
// FILE
//=========================================================
// Don't mess around, let boost::filesystem do all the nasty work.
// Folder recursion can be done directly:
/*
        using namespace boost::filesystem;
        recursive_directory_iterator it = recursive_directory_iterator("START_DIR");
        recursive_directory_iterator end;
        while(it != end)
        {
            // -----
            // DO SOMETHING.  Like, eg, cout...
            cout << (is_directory(*it) ? 'D' : ' ') << ' ';
            cout << (is_symlink(*it) ? 'L' : ' ') << ' ';
            for(int i = 0; i < it.level(); ++i)
                cout << ' ';
            cout << it->path().string() << endl;
            // -----

            try
            {
                if(is_directory(*it) && is_symlink(*it)) it.no_push();
                ++it;
            } catch(std::exception& ex) { it.no_push(); }
        }
 */
//=========================================================

// NOTE that the declaration (above) provides a default of "" for pre/suffix.
static bool archive_any_old_file(const string& filename, const string& prefix, const string& suffix)
{
    try
    {
        using namespace boost::filesystem;      // rename, path
        path p(filename);
        if (exists(p))
        {
            stringstream ss;
            ss << prefix << filename << (suffix.empty() ? string(".") + generate_uuid() : suffix);
            rename(p,path(ss.str()));
        }
        return true;
    }
    catch(...)
    {
        // Not much we can do, but we'll try to tell the user.
        g_ss.str(string());
        g_ss << "Failed to remove old file: [" << filename << "]";
        log(LV_ERROR,g_ss.str());
        return false;
    }
}
// NOTE that the declaration (above) provides a default of "" for pre/suffix.
static bool backup_any_old_file(const string& filename, const string& prefix, const string& suffix)
{
    try
    {
        using namespace boost::filesystem;      // rename, path
        path p(filename);
        if (exists(p))
        {
            stringstream ss;
            ss << prefix << filename << (suffix.empty() ? string(".") + generate_uuid() : suffix);
            copy(p,path(ss.str()));
        }
        return true;
    }
    catch(...)
    {
        // Not much we can do, but we'll try to tell the user.
        g_ss.str(string());
        g_ss << "Failed to back up file: [" << filename << "]";
        log(LV_ERROR,g_ss.str());
        return false;
    }
}
// There are low-level good ways, and very many higher-level/STL bad ways, to read files:
//  http://insanecoding.blogspot.com/2011/11/how-to-read-in-file-in-c.html
#include <fstream>
#include <cerrno>
static string read_file(string filename)
{
  std::ifstream in(filename.c_str(), std::ios::in | std::ios::binary);
  if (in)
  {
    std::string contents;
    in.seekg(0, std::ios::end);
    contents.resize(in.tellg());
    in.seekg(0, std::ios::beg);
    in.read(&contents[0], contents.size());
    in.close();
    return(contents);
  }
  throw(errno);
}
//=========================================================


// ===========================================
// JSON encoding/decoding
// ===========================================

// -----------------------------------------------------
// https://gist.github.com/moodboom/0ad810280635ead63d0f
// rapidjson.org
// https://github.com/miloyip/rapidjson/
// it's FAST: https://github.com/mloskot/json_benchmark
// but it obsesses over allocation (read: F'IN INCONVENIENT for std::string's)
// Here we attempt to make rapidjson practical:
//
//   1) throw proper exceptions during runtime
//   2) directly handle std::string

// We override the default behavior of asserting on parse errors
// with throwing of this exception.  Then we can gracefully handle errors.
class rapidjson_exception : public std::runtime_error
{
public:
    rapidjson_exception() : std::runtime_error("json schema invalid") {}
};
#define RAPIDJSON_ASSERT(x)  if(x); else throw rapidjson_exception();

#include <rapidjson/document.h>
#include <rapidjson/stringbuffer.h>       // These two includes allow you to get an object as a string
#include <rapidjson/writer.h>             // That allows an array of objects to use an object parser function

static string json_get_string(rapidjson::Document& d)
{
    rapidjson::StringBuffer sb;
    rapidjson::Writer<rapidjson::StringBuffer> writer( sb );
    d.Accept( writer );
    return sb.GetString();
}

// 2016/03/12 Sure rapidjson is fast but it sucks ass at handling std::string.  No excuse.
// See docs here:
//
//      https://github.com/miloyip/rapidjson/blob/master/example/tutorial/tutorial.cpp
//
static void json_add_string(rapidjson::Document& d, rapidjson::Value& parent, const string& name, const string& value)
{
    rapidjson::Value n;
    n.SetString(name.c_str(),name.size(),d.GetAllocator());
    rapidjson::Value v;
    v.SetString(value.c_str(),value.size(),d.GetAllocator());
    parent.AddMember(n,v,d.GetAllocator());
}
// -----------------------------------------------------


// ===========================================
//   boost XML parsing via property_tree
// ===========================================
// - Does not do SAX-style parsing
// - Messy
// - Uses RapidXML underneath (good i think)
//
// I'll keep the headers here since we only plan
// to use it here to extract customer xml.
//
// NOTE that ptree JSON handling does not preserve
// int/bool/etc type - rendering it totally useless.
// ===========================================
#define BOOST_SPIRIT_THREADSAFE
#include <boost/property_tree/ptree.hpp>
// #include <boost/property_tree/json_parser.hpp>
#include <boost/property_tree/xml_parser.hpp>
using boost::property_tree::ptree;
// ===========================================


//=========================================================
// PROFILING
//=========================================================
static void start_profile(time_t& start_time)
{
	time(&start_time);
}
static void end_profile(const time_t& start_time, std::string msg)
{
	time_t now;
	time(&now);
	int n_seconds = (int)(now - start_time);
	cout << msg << ": " << n_seconds / 60 << ":" << n_seconds % 60 << endl;
}
#include <chrono>
static void start_profile_ms(uint64_t& start_time)
{
    start_time =
            std::chrono::system_clock::now().time_since_epoch() /
            std::chrono::milliseconds(1);
}
static uint64_t end_profile_ms(const uint64_t& start_time, std::string msg = "")
{
    uint64_t now =
        std::chrono::system_clock::now().time_since_epoch() /
        std::chrono::milliseconds(1);
    uint64_t n_ms = (now - start_time);
    if (msg.size() > 0)
        cout << msg << ": " << n_ms << "ms" << endl;
    return n_ms;
}
//=========================================================


//=========================================================
// math
//=========================================================
static bool bEqual(const double& a, const double& b)
{
    return                                                                                  // nicked from http://stackoverflow.com/questions/4010240/comparing-doubles
            (a == b)                                                                        // Test 1: Very cheap, but can result in false negatives
        ||  (std::abs(a-b)<std::abs(std::min(a,b))*std::numeric_limits<double>::epsilon())  // Test 2: More expensive, but comprehensive
    ;
}
//=========================================================


//=========================================================
// MISC
//=========================================================

static void sleep(int n_secs)
{
    boost::this_thread::sleep(boost::posix_time::seconds(n_secs));
}


static bool unzip_first_file(string& str_zip, string& str_unzipped)
{
	// Using miniz:
	//		http://code.google.com/p/miniz/

	mz_zip_archive zip_archive;
	mz_bool status;

	// Now try to open the archive.
	memset(&zip_archive, 0, sizeof(zip_archive));

	status = mz_zip_reader_init_mem(&zip_archive, str_zip.c_str(), str_zip.size(), 0);

	// NOTE that miniz can also handle a file directly...
	// status = mz_zip_reader_init_file("myfile.zip", str_zip.c_str(), 0);

	if (!status)
	{
		cout << "zip file appears invalid..." << endl;
		return false;
	}

	// Get the first file in the archive.
	// By definition, this is our object model.
	if (mz_zip_reader_get_num_files(&zip_archive) != 1)
	{
		cout << "zip file does not contain 1 file..." << endl;
		return false;
	}

	mz_zip_archive_file_stat file_stat;
	if (!mz_zip_reader_file_stat(&zip_archive, 0, &file_stat))
	{
		cout << "zip file read error..." << endl;
		mz_zip_reader_end(&zip_archive);
		return false;
	}
		
	// Unzip the file to heap.
	size_t uncompressed_size = (size_t)file_stat.m_uncomp_size;
    void* p = mz_zip_reader_extract_file_to_heap(&zip_archive, file_stat.m_filename, &uncompressed_size, 0);
	if (!p)
	{
		cout << "mz_zip_reader_extract_file_to_heap() failed..." << endl;
		mz_zip_reader_end(&zip_archive);
		return false;
	}

	str_unzipped.assign((const char*)p,uncompressed_size);

	// Close the archive, freeing any resources it was using
	mz_free(p);
	mz_zip_reader_end(&zip_archive);

	return true;
}
//=========================================================


#endif

