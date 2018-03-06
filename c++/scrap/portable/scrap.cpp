#include "scrap.hpp"

// 16 c++11 futures and atomic variables
#include <future>
#include <thread>
#include <chrono>


int main(int argc, char *argv[])
{
    // We have a VERY SMALL number of globals defined for us in utilities.cpp.
    // Here we can initialize them to our values.
    // g_ss - ok as is
    g_current_log_verbosity = LV_ALWAYS;
    g_base_log_filename = "portable";

    std::cout << "NOTE: NDEBUG is ";
#ifndef NDEBUG
    std::cout << "NOT";
#endif
    std::cout << " defined\n\n";

    // 1 ===================================================================================
    cout << endl << "== 1 === cast ==========" << endl;
    // 1 ===================================================================================

    // const_cast<>() does no error checking.
    // polymorphic_downcast<>() will cast from base to derived class, 
    // using assert(dynamic_cast<>()) in DEBUG mode to check if the classes align.
    //
    // dynamic_cast<>() returns 0, some may find inconvenient I guess?  So
    // polymorphic_cast<>() performs a dynamic_cast, and throws on failure.  Bleh.

    using namespace casty;
    //  tests which should succeed
    Base *base = new Derived;
    Base2 *base2 = 0;
    Derived *derived = 0;
    derived = polymorphic_downcast<Derived *>(base);  // downcast
    assert(derived->kind() == 'D');
    derived = polymorphic_cast<Derived *>(base); // downcast, throw on error
    assert(derived->kind() == 'D');
    base2 = polymorphic_cast<Base2 *>(base); // crosscast
    assert(base2->kind2() == '2');
    delete base;

    //  tests which should result in errors being detected
    int err_count = 0;
    base = new Base;
    bool caught_exception = false;
    try
    {
        derived = polymorphic_cast<Derived *>(base);
    }
    catch (std::bad_cast)
    {
        std::cout << "caught bad_cast\n";
        caught_exception = true;
    }
    if (!caught_exception)
        ++err_count;

    std::cout << err_count << " errors detected\nTest " << (err_count == 0 ? "passed\n" : "failed\n");


    // 2 ===================================================================================
    cout << endl << "== 2 === lambda ========" << endl;
    // 2 ===================================================================================

    // lamba test.  simple and does not require build of boost, just access to headers.
    {
        using namespace boost::lambda;
        typedef std::istream_iterator<int> in;

        // MDM This works but I'm commenting out so other tests can run without need for input.
        // Just uncomment to re-enable this scrap.
        // std::cout << "type something..." << endl;
        // std::for_each(
        //           in(std::cin), in(), std::cout << " results " << (boost::lambda::_1 * 3) << " "
        // );
    }


    // 3 ===================================================================================
    cout << endl << "== 3 === regex =========" << endl;
    // 3 ===================================================================================

    // regex test.  requires fully-built boost.
    std::string line;
    boost::regex pat("^Subject: (Re: |Aw: )*(.*)");

    // test with user input
    // MDM commenting so we can run other tests
    /*
    while (std::cin)
    {
        std::getline(std::cin, line);
        boost::smatch matches;
        if (boost::regex_match(line, matches, pat))
          std::cout << matches[2] << std::endl;
    }*/
    line = "Subject: Re: wtf is Aw?";
    boost::smatch matches;
    if (boost::regex_match(line, matches, pat))
        std::cout << "found match: " << matches[2] << std::endl;

    // ===================================================================================


    // 4 =================================================================================== 
    cout << endl << "== 4 === hash ==========" << endl;
    // 4 ===================================================================================

    // hash tables
    // unordered_map, unordered_set (and _multi...) in boost
    typedef boost::unordered_map<std::string, int> hashmap;
    hashmap x;
    x["one"] = 1;
    x["two"] = 2;
    x["three"] = 3;

    // avoid boost's FOREACH, it's not standard - but it works!
    BOOST_FOREACH(hashmap::value_type
    i, x) {
        std::cout << i.first << "," << i.second << "\n";
    }
    // try the lambda way instead... nope, not yet available
    // int my_array[5] = {1, 2, 3, 4, 5};
    // for(int &x : my_array)
    // {
    //    x *= 2;
    // }

    // if ( argc > 1 && *argv[1] == '1' )


    // 6 ===================================================================================
    cout << endl << "== 6 === sort iterator =" << endl;
    // 6 ===================================================================================

    // let's find out what happens to iterators and pointers on a sort...
    // we have a vector of objects
    // and a vector of pointers to objects within the original vector
    // after sorting, we want to know...
    //	do iterators point to the same object, or to the new object that got stored in the same "slot" eg myV[4]?
    //	do pointers get invalidated?
    std::vector<int> vi;
    vi.push_back(5);
    vi.push_back(4);
    vi.push_back(3);
    vi.push_back(2);
    vi.push_back(1);

    std::vector<int>::iterator it4 = vi.begin() + 1;
    std::sort(vi.begin(), vi.end());

    // four = 2
    // I think that really screws me up!
    int four = *it4;

    std::vector <blah> vt;
    vt.push_back(blah(5, 6));
    vt.push_back(blah(4, 6));
    vt.push_back(blah(3, 6));
    vt.push_back(blah(3, 5));
    vt.push_back(blah(3, 3));

    std::vector<blah>::iterator it46 = vt.begin() + 1;
    std::sort(vt.begin(), vt.end());

    // four = 35
    // I think that really screws me up!
    blah fourth = *it46;

    std::vector < blah * > vpt;
    vpt.push_back(new blah(5, 6));
    vpt.push_back(new blah(4, 6));
    vpt.push_back(new blah(3, 6));
    vpt.push_back(new blah(3, 5));
    vpt.push_back(new blah(3, 3));

    std::vector<blah *>::iterator itp46 = vpt.begin() + 1;
    blah *p46 = vpt[1];
    blah &r46 = *(vpt[1]);

    std::sort(vpt.begin(), vpt.end(), blah_ptr_lessthan());

    // it four = 35; pointer & reference four = 4,6
    // perfect, use pointer vectors, sort all day long, store pointers and references all day long
    blah &fourth1 = **itp46;
    blah &fourth2 = *p46;
    blah &fourth3 = r46;


    // 7 ===================================================================================
    cout << endl << "== 7 === rvalues =======" << endl;
    // 7 ===================================================================================

    // c++11 rvalues

    Value v(L"Hello World");

    wstring str = v;
    wcout << str << endl;

    ArgsConfig config;

    // This works in VS2012
    const wstring &&value = config("key");
    wcout << value << endl;

    // But this does not 
    // this pukes in VS 2012 bc both wstring&& and wchar_t const* constructors could be used?
    // const wstring value2 = config("key");
    // wcout << value2 << endl;

    Value int_val(L"1");
    int i = int_val;

    cout << i + 1 << endl;


    // 8 ===================================================================================
    cout << endl << "== 8 === hashmap =======" << endl;
    // 8 ===================================================================================

    // hash tables
    // c++11 unordered_map availability
    //	VS2010?		yep
    //	gcc?		yep

    // unordered_map, unordered_set (and _multi...) in c++11

    // unordered_map
    // uses a hash table plus buckets for hash collisions; all keys are unique
    hashmap m;
    m["one"] = 1;
    m["two"] = 2;
    m["three"] = 3;

    cout << "unordered_map lookup: " << m["one"] << endl;

    // unordered_multimap
    // Elements with equivalent keys are grouped together in the same bucket and in such a way that an iterator (see equal_range) can iterate through all of them.
    hashmmap mm;
    mm.insert(hashmap::value_type("bee", 2));
    mm.insert(hashmap::value_type("c", 3));
    mm.insert(hashmap::value_type("dee", 4));
    mm.insert(hashmap::value_type("a", 3));
    mm.insert(hashmap::value_type("bee", 232));
    mm.insert(hashmap::value_type("bee", 6162));
    mm.insert(hashmap::value_type("a", 3));
    mm.insert(hashmap::value_type("a", 1));
    mm.insert(hashmap::value_type("a", 3));

    // Most of the time you'll want to use equal_range() which gives you an iterator to all the matches for a key.
    // Here are a couple ways to do it.

    cout << "unordered_map all 'a' values:";
    auto range = mm.equal_range("a");
    for_each(
        range.first,
        range.second,
        [](hashmmap::value_type &x) { std::cout << " " << x.second; }
    );
    cout << endl;

    cout << "unordered_map all 'a' values:";
    auto bucket_range = mm.equal_range("a");
    for (auto &bucket_loop = bucket_range.first; bucket_loop != bucket_range.second; ++bucket_loop)
    {
        cout << " " << bucket_loop->second;
    }
    cout << endl;

    // Here we'll iterate the keys in a multimap, then all key values in a nested loop.
    cout << "unordered_map grouped key values:";
    auto hash_prev = mm.begin();
    for (auto hash_next = mm.begin(); hash_next != mm.end(); ++hash_next)
    {
        // If it's not the same key as last time...
        // (Are the keys guaranteed to come in order tho??)
        if (hash_next == mm.begin() || hash_next->first != hash_prev->first)
        {
            auto bucket_start = mm.equal_range(hash_next->first);
            if (bucket_start.first != mm.end())
            {
                cout << endl;
                cout << bucket_start.first->first << ":";
                for (auto &bucket_loop = bucket_start.first; bucket_loop != bucket_start.second; ++bucket_loop)
                {
                    cout << " " << bucket_loop->second;
                }
            }
        }
        hash_prev = hash_next;
    }
    cout << endl;


    // try the lambda way instead... nope, not yet available
    // int my_array[5] = {1, 2, 3, 4, 5};
    // for(int &x : my_array)
    // {
    //    x *= 2;
    // }

    // DEBUG pause
    /*

    void waiter(int n_secs)
    {
        boost::this_thread::sleep_for( boost::chrono::seconds(n_secs) );
        std::cout << endl;
        std::cout << "Done." << endl;
    }

    cout << "Waiting for " << n_secs << " seconds..." << endl;
    cout.flush();
    boost::thread thr_waiter(waiter, n_secs);
    thr_waiter.join();

    */


    // 9 ===================================================================================
    cout << endl << "== 9 === BGL ===========" << endl;
    // 9 ===================================================================================

    //
    // boost graph library
    // family tree
    //
    // i would have NEVER GUESSED it was THIS EASY, awesomeness
    // nodes are all given integer ids
    // SO SIMPLE

    enum family
    {
        Jeanie, Debbie, Rick, John, Amanda, Margaret, Benjamin, N
    };

    const char *name[] = {"Jeanie", "Debbie", "Rick", "John", "Amanda", "Margaret", "Benjamin"};

    adjacency_list<> g(N);

    add_edge(Jeanie, Debbie, g);
    add_edge(Jeanie, Rick, g);
    add_edge(Jeanie, John, g);
    add_edge(Debbie, Amanda, g);
    add_edge(Rick, Margaret, g);
    add_edge(John, Benjamin, g);

    graph_traits<adjacency_list<> >::vertex_iterator vit, end;
    graph_traits<adjacency_list<> >::adjacency_iterator ai, a_end;
    property_map<adjacency_list<>, vertex_index_t>::type
        index_map = get(vertex_index, g);

    for (boost::tie(vit, end) = vertices(g); vit != end; ++vit)
    {
        std::cout << name[get(index_map, *vit)];
        boost::tie(ai, a_end) = adjacent_vertices(*vit, g);
        if (ai == a_end)
            std::cout << " has no children";
        else
            std::cout << " is the parent of ";
        for (; ai != a_end; ++ai)
        {
            std::cout << name[get(index_map, *ai)];
            if (boost::next(ai) != a_end)
                std::cout << ", ";
        }
        std::cout << std::endl;
    }


    // 10 ==================================================================================
    cout << endl << "== 10 === storage vs speed" << endl;
    // 10 ==================================================================================

    // Compare int8 vs bit storage and speed

    // BIT WINS of course
    // release build optimizes this all out and always runs in zero time :-)
    // debug build, executed, results:
    //
    //  BUILT PHASING
    //      bit took: 0:30
    //      byte took: 1:06 
    //
    //  ENERGIZED PHASING
    //      bit took: 0:19
    //      byte took: 0:30

    uint_fast8_t bit_result;
    uint_fast8_t byte_result[3];

    // energized phasing
    // built abc energized cba
    // 2 bits per phase slot
    // 2 bits = 
    //      0   NONE
    //      1   A
    //      2   B
    //      3   C

    // LOTS OF CONSTANTS to make code easy to read :-)

    const uint_fast8_t PHASE_NONE = 0;      // --CCBBAA

    const uint_fast8_t PHASE_A = 1;
    const uint_fast8_t PHASE_B = 2;
    const uint_fast8_t PHASE_C = 3;

    const uint_fast8_t PHASE_A_IN_A = 1;    // 00000001
    const uint_fast8_t PHASE_A_IN_B = 4;    // 00000100
    const uint_fast8_t PHASE_A_IN_C = 16;   // 00010000
    const uint_fast8_t PHASE_B_IN_A = 2;    // 00000010
    const uint_fast8_t PHASE_B_IN_B = 8;    // 00001000
    const uint_fast8_t PHASE_B_IN_C = 32;   // 00100000
    const uint_fast8_t PHASE_C_IN_A = 3;    // 00000011
    const uint_fast8_t PHASE_C_IN_B = 12;   // 00001100
    const uint_fast8_t PHASE_C_IN_C = 48;   // 00110000

    const uint_fast8_t PHASE_A_MASK = 3;    // 00000011
    const uint_fast8_t PHASE_B_MASK = 12;   // 00001100
    const uint_fast8_t PHASE_C_MASK = 48;   // 00110000

    const uint_fast8_t PHASE_COUNT = 3;
    const uint_fast8_t PHASE_A_SHIFT = 4;
    const uint_fast8_t PHASE_B_SHIFT = 2;

    time_t t_go;
    start_profile(t_go);

    // MDM ratcheting this way down so other tests don't have to wait
    // for (int n = 0; n < 1000; n++)
    // for (int n = 0; n < 10000000; n++)
    for (int n = 0; n < 100000000; n++)
    {
        bool b_has_a, b_has_ac;

        // int n_init_phase = rand() % 7;
        int n_init_phase = 7;
        uint_fast8_t bit_phases = n_init_phase;
        b_has_a = (bit_phases | 4) > 0;
        b_has_ac = (bit_phases | 1) && (bit_phases | 4);
        bit_phases = 5;
        bit_phases = bit_phases | 2;
        bit_result = bit_phases;

        uint_fast8_t bit_ephases = PHASE_C_IN_A | PHASE_B_IN_B | PHASE_A_IN_C;
        b_has_a = (bit_ephases | PHASE_A_MASK) == PHASE_A_IN_A || (bit_ephases | PHASE_B_MASK) == PHASE_A_IN_B ||
                  (bit_ephases | PHASE_C_MASK) == PHASE_A_IN_C;
        b_has_ac =
            ((bit_ephases | PHASE_A_MASK) == PHASE_A_IN_A || (bit_ephases | PHASE_B_MASK) == PHASE_A_IN_B ||
             (bit_ephases | PHASE_C_MASK) == PHASE_A_IN_C)
            && ((bit_ephases | PHASE_A_MASK) == PHASE_C_IN_A || (bit_ephases | PHASE_B_MASK) == PHASE_C_IN_B ||
                (bit_ephases | PHASE_C_MASK) == PHASE_C_IN_C);

    }
    end_profile(t_go, "bit took:");

    start_profile(t_go);
    // MDM ratcheting this way down so other tests don't have to wait
    // for (int n = 0; n < 1000; n++)
    // for (int n = 0; n < 10000000; n++)
    for (int n = 0; n < 100000000; n++)
    {
        bool b_has_a, b_has_ac;

        uint_fast8_t byte_phases[3];
        // int n_init_phase = rand() % 7;
        byte_phases[0] = 1;
        byte_phases[1] = 1;
        byte_phases[2] = 1;
        b_has_a = byte_phases[0] == 0;
        b_has_ac = (byte_phases[0] == 0) && (byte_phases[2] == 0);
        byte_phases[0] = 1;
        byte_phases[1] = 0;
        byte_phases[2] = 1;
        byte_phases[1] = 1;
        memcpy(byte_result, byte_phases, sizeof(byte_result));

        uint_fast8_t byte_ephases[3];
        byte_ephases[0] = PHASE_C;
        byte_ephases[1] = PHASE_B;
        byte_ephases[2] = PHASE_A;
        b_has_a = (byte_ephases[0] == PHASE_A) || (byte_ephases[1] == PHASE_A) || (byte_ephases[2] == PHASE_A);
        b_has_ac = ((byte_ephases[0] == PHASE_A) || (byte_ephases[1] == PHASE_A) || (byte_ephases[2] == PHASE_A)) &&
                   ((byte_ephases[0] == PHASE_C) || (byte_ephases[1] == PHASE_C) || (byte_ephases[2] == PHASE_C));
    }
    end_profile(t_go, "byte took:");


    // 11 ================================================================================== 
    cout << endl << "== 11 === unordered_set of pointers ftw! ==" << endl;
    // 11 ==================================================================================

    // unordered set of pointers
    // To use c++11 undordered containers with object keys, we need hash_value and equal_to.
    // When using pointers to objects, we need the functions to use pointers, and we have to specify them when defining the set or map.
    // hash_value and equal_to functors work perfectly with pointer parameters.
    // 
    // Below, you can see that the equal_to and hash_value functors drive the behavior of the set.
    // If an item is equal according to the functor, it is not allowed to be reinserted!  whoop!
    // ==================================================================================

    BlahPointerSet blah_uset;
    BlahPointerAltSet blah_alt_uset;

    blah_uset.insert(new blah(1, 2, "first"));
    blah_uset.insert(new blah(3, 4));
    blah_uset.insert(new blah(5, 6));

    blah_uset.insert(new blah(1, 7));
    blah_uset.insert(new blah(3, 8));
    blah_uset.insert(new blah(5, 9));

    // No effect, element already exists.
    blah_uset.insert(new blah(1, 2, "second"));

    blah_alt_uset.insert(new blah(1, 2));
    blah_alt_uset.insert(new blah(3, 4));
    blah_alt_uset.insert(new blah(5, 6));

    // NOTE: No effect, element already exists using alt equal_to!!
    blah_alt_uset.insert(new blah(1, 7));
    blah_alt_uset.insert(new blah(3, 8));
    blah_alt_uset.insert(new blah(5, 9));

    cout << "    uset size = " << blah_uset.size() << endl;         // 6
    cout << "alt uset size = " << blah_alt_uset.size() << endl;     // 3

    // which of the two was used?  the first one!  "first"
    // c++11 doesn't like this temp var usage, use a key on the stack: auto it1 = blah_uset.find(&blah(1,2));
    blah key(1, 2);
    cout << "finding... " << endl;
    auto it1 = blah_uset.find(&key);
    cout << "find gives... " << (*it1)->name_ << endl;


    // 12 ==================================================================================
    cout << endl << "== 12 === streams =======" << endl;
    // 12 ==================================================================================

    boost::asio::streambuf asio_buf;
    std::iostream request_stream(&asio_buf);
    request_stream << "Accept: */*\r\n";

    size_t req_length = request_stream.tellp();
    cout << "ostream p length: " << req_length;

    req_length = request_stream.tellg();
    cout << "ostream g length: " << req_length;

    request_stream.seekp(0, std::ios::beg);
    std::stringstream ss_request;
    ss_request << request_stream.rdbuf();
    string str_request = ss_request.str();
    request_stream.seekp(req_length);
    string str_backup;
    // str_backup << request_stream;

    cout << endl;


    // 13 ==================================================================================
    cout << endl << "== 13 === unordered_multiset secondary index that includes dupe keys ==" << endl;
    // 13 ==================================================================================

    // We want a master container with pointers to objects.  There will be many objects in the container.
    // Use unique_ptr to allocate, and let it do the cleanup automatically when the container goes out of scope.
    // We use an unordered_set for fast lookup.  That means we need both equals and hash functions for the object.

    // We also want a secondary container of pointers to the same objects, using a different key.
    // This secondary container can use a key that is not unique across the objects.
    // This is very common to be able to quickly gather "categories" of objects.
    // We use an unordered_multiset.  We need equals and hash functions, and we need to walk through similar key values.
    // (TODO)


    // 14 ==================================================================================
    cout << endl << "== 14 === version strings =======" << endl;
    // 14 ==================================================================================
    string v1, v2;

    v1 = "0.1.2.3 and then some description";
    v2 = "1";
    cout << "[" << v1 << "] is " << (SemVer(v1).bLessThan(v2) ? " less   " : " greater ") << " than [" << v2 << "]"
         << endl;

    v1 = "0.57 desc";
    v2 = "1.0";
    cout << "[" << v1 << "] is " << (SemVer(v1).bLessThan(v2) ? " less   " : " greater ") << " than [" << v2 << "]"
         << endl;


    // 15 ==================================================================================
    cout << endl << "== 15 === 0xdeadbeef =======" << endl;
    // 15 ==================================================================================

    // FWIW In all 4 cases (static/nonstatic, debug/release), unassigned memory was zeroed out.

    const int charcount = 10;
    char nonstatictest[charcount] = "12345";
    static char statictest[charcount] = "12345";

    cout << "nonstatic init: ";
    for (int i = 0; i < charcount; ++i)
        cout << " 0x" << std::hex << std::uppercase << setw(2) << setfill('0') << (int) nonstatictest[i];
    cout << endl;

    cout << "static init:    ";
    for (int i = 0; i < charcount; ++i)
        cout << " 0x" << std::hex << std::uppercase << setw(2) << setfill('0') << (int) statictest[i];
    cout << endl;


    // 16 ==================================================================================
    cout << endl << "== 16 === thread stage detection =======" << endl;
    // 16 ==================================================================================

    // NOTE that chrono_literals requires c++14!  time to push portable there - updating CMakeLists.txt...
    using namespace std::chrono_literals; // c++14; for 2s, 0ms, etc; so cool!

    // ----------
    // std::async
    // Not sure what it gets us.
    // ----------
    // Run some task on new thread. The launch policy std::launch::async
    // makes sure that the task is run asynchronously on a new thread.
    auto future = std::async(std::launch::async, [] {
        std::this_thread::sleep_for(200ms);
        return 8;
    });

    // Use wait_for() with zero milliseconds to check thread status.
    auto status = future.wait_for(20ms);

    // Print status.
    if (status == std::future_status::ready) {
        std::cout << "Thread finished" << std::endl;
    } else {
        std::cout << "Thread still running" << std::endl;
    }

    // THIS SUCKS don't do it...
    std::cout << "std::future get() will block until done... :-( " << std::endl;
    auto result = future.get(); // Get result.
    std::cout << "Thread finished" << std::endl;
    // ----------

    /*
    // ----------
    // c++11 has atomic variables, here's why they ROCK, you can use them to get thread status
    // ----------
    std::atomic<int32_t> job_count(0);
    std::thread t([&job_count] {
        for (int loop = 0; loop < 20 && job_count < 100; ++loop)
        {
            std::this_thread::sleep_for(100ms);
            job_count++; // Is done atomically.
        }
        std::cout << "thread exiting..." << std::endl;
    });

    for (int check = 0; check < 7; ++check)
    {
        std::this_thread::sleep_for(200ms);
        // not sure why i was getting std::hex output...
        std::cout << "check " << check << " job count: " << std::dec << job_count << std::endl;
    }

    // We can trigger the thread to exit.
    // If we don't do this, the loop will complete after 20 times, which the join() will wait for.
    // job_count.store(101);
    job_count = 101;

    // Let's see what happens if we don't join until after the thread is done.
    std::this_thread::sleep_for(300ms);
    std::cout << "done sleeping" << std::endl;

    // NOW we will block to ensure the thread finishes.
    std::cout << "joining" << std::endl;
    t.join();
    std::cout << "all done" << std::endl;
    // ----------
    */

    // --------------------------------------------------------------------
    // CONCISE EXAMPLE OF THREAD WITH EXTERNALLY-ACCESSIBLE STATUS AND DATA
    // --------------------------------------------------------------------
    // We create a vector, and create a thread to start stuffing it.
    // Externally, we can check the status of the job, and have mutex access to the data.
    // The atomic job stage is SO CHEAP to change and check, do it all day long as needed.
    // Initially, externally, we check the job stage.
    // Meanwhile, we do a bunch of intense work inside the mutex.
    // Then we do smaller work with frequent mutexing, allowing interruption.
    // Great patterns, use them brother!
    // Hells to the yeah.
    // --------------------------------------------------------------------
    std::atomic<int32_t> job_stage(0);
    std::unordered_set<int> data;
    boost::shared_mutex data_guard;
    data.insert(-1);
    std::thread t([&job_stage,&data,&data_guard] {
        // stage 1 = jam in data
        job_stage = 1;
        {
            boost::upgrade_lock<boost::shared_mutex> lock(data_guard);
            boost::upgrade_to_unique_lock<boost::shared_mutex> uniqueLock(lock);
            for (int loop = 0; loop < 2000; ++loop)
            {
                std::this_thread::sleep_for(1ms);
                data.insert(loop);
            }
        }
        // stage 2 = mutex-jam data, allowing intervention
        job_stage = 2;
        for (int loop = 3000000; loop < 4000000 && job_stage == 2; ++loop)
        {
            boost::upgrade_lock<boost::shared_mutex> lock(data_guard);
            boost::upgrade_to_unique_lock<boost::shared_mutex> uniqueLock(lock);
            data.insert(loop);
        }
        cout << "thread exiting..." << endl;
    });

    cout << "pre mutex job stage: " << job_stage << endl;

    for (int check = 0; check < 6; ++check)
    {
        std::this_thread::sleep_for(200ms);
        // not sure why i was getting std::hex output...
        cout << "check " << check << " job stage: ";
        {
            boost::upgrade_lock<boost::shared_mutex> lock(data_guard);
            cout  << dec << job_stage << " data size " << data.size();
        }
        cout << endl;
    }

    // We can trigger the thread to exit.
    job_stage = 3;

    // Let's see what happens if we don't join until after the thread is done.




    cout << "done sleeping" << endl;

    // NOW we will block to ensure the thread finishes.
    cout << "joining" << endl;
    t.join();
    cout << "all done" << endl;
    // --------------------------------------------------------------------


    // 17 ==================================================================================
    cout << endl << "== 17 === stupid little fizzbuzz :-) =======" << endl;
    // 17 ==================================================================================

    for (int32_t i = 0; i < 100; i++)
    {
        if (i%3 ==0) cout << i << " is a fizz multiple of 3" << endl;
        if (i%5 ==0) cout << i << " is a buzz multiple of 5" << endl;
        if (i%15==0) cout << i << " is ALSO A FIZZBUZZ, WHOOHOO!" << endl;
    }


    // 17 ==================================================================================
    cout << endl << "== 17 === repeat var name in deeper scope =======" << endl;
    // 17 ==================================================================================

    int sloppy;
    sloppy = 1;
    {
        int sloppy;
        sloppy = 2;
        {
            cout << "inner scope outer var = " << sloppy << endl;
            int sloppy;
            sloppy = 3;
            cout << "inner scope = " << sloppy << endl;
        }
        cout << "mid scope = " << sloppy << endl;
    }
    cout << "outer scope = " << sloppy << endl;


    // ========= end ========
    // We can keep it running if needed to better see the output.
    // while (true)
    // {}
}
