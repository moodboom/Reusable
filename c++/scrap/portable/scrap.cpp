#include "scrap.hpp"

// 16 c++11 futures and atomic variables
#include <future>
#include <thread>
#include <chrono>

// 18 NLOHMANN JSON, can be used with boost JSON with the right namespaces
#include <json.hpp>


// ===============================
// == 26 === lambda redux ========
// ===============================
// These must be outside another function's scope.
void use_it(auto func) {
    func();
}
class A {
    public:
    int a_;
};
class MyObject {
    // ...
    public:

    // THIS is what we want, the rest is just futzing around.
    // We want a lambda that defaults to a no-op, 
    // but can be optionally provided by the caller to do extra work to A.
    void applyFunctionToAWithDefault(A& a, std::function<void(A&)> func = [](A& a) -> void { })
    {
        func(a);
    }

    void applyFunctionToA(A& a, auto* func) {
        (*func)(a);
    }

    void SetXGetter(auto func) {
        mLambda = std::move(func);
    }

    void SetXGetterProperForwarding(std::function<int()> func) {
        mLambda = std::move(func);
    }

    void call()
    {
        mLambda();
    }

    int callProperForwarding()
    {
        return mLambda();
    }

    int x_;

    private:
        std::function<int()> mLambda;
};
// ===============================


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

    std::cout << "Jumping to latest scrap..." << std::endl;
    goto run_from_here;

    // 1 ===================================================================================
    cout << endl << "== 1 === cast ==========" << endl;
    // 1 ===================================================================================
    {
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
    }


    // 3 ===================================================================================
    cout << endl << "== 3 === regex =========" << endl;
    // 3 ===================================================================================
    {
        // regex test.  requires fully-built boost.
        string line;
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

    }
    // 4 =================================================================================== 
    cout << endl << "== 4 === hash ==========" << endl;
    // 4 ===================================================================================
    {
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

    }
    // 6 ===================================================================================
    cout << endl << "== 6 === sort iterator =" << endl;
    // 6 ===================================================================================
    {
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
    }
    // 7 ===================================================================================
    cout << endl << "== 7 === rvalues =======" << endl;
    // 7 ===================================================================================
    {
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
    }
    // 8 ===================================================================================
    cout << endl << "== 8 === hashmap =======" << endl;
    // 8 ===================================================================================
    {
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
    }
    // 9 ===================================================================================
    cout << endl << "== 9 === BGL ===========" << endl;
    // 9 ===================================================================================
    {
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
    }
    // 10 ==================================================================================
    cout << endl << "== 10 === storage vs speed" << endl;
    // 10 ==================================================================================
    {
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
    }
    // 11 ================================================================================== 
    cout << endl << "== 11 === unordered_set of pointers ftw! ==" << endl;
    // 11 ==================================================================================
    {
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

    }
    // 12 ==================================================================================
    cout << endl << "== 12 === streams =======" << endl;
    // 12 ==================================================================================
    {
        boost::asio::streambuf asio_buf;
        std::iostream request_stream(&asio_buf);
        request_stream << "Accept: */*\r\n";

        size_t req_length = request_stream.tellp();
        cout << "ostream p length: " << req_length;

        req_length = request_stream.tellg();
        cout << "ostream g length: " << req_length;

        request_stream.seekp(0, std::ios::beg);
        stringstream ss_request;
        ss_request << request_stream.rdbuf();
        string str_request = ss_request.str();
        request_stream.seekp(req_length);
        string str_backup;
        // str_backup << request_stream;

        cout << endl;
    }

    // 13 ==================================================================================
    cout << endl << "== 13 === unordered_multiset secondary index that includes dupe keys ==" << endl;
    // 13 ==================================================================================
    {
        // We want a master container with pointers to objects.  There will be many objects in the container.
        // Use unique_ptr to allocate, and let it do the cleanup automatically when the container goes out of scope.
        // We use an unordered_set for fast lookup.  That means we need both equals and hash functions for the object.

        // We also want a secondary container of pointers to the same objects, using a different key.
        // This secondary container can use a key that is not unique across the objects.
        // This is very common to be able to quickly gather "categories" of objects.
        // We use an unordered_multiset.  We need equals and hash functions, and we need to walk through similar key values.
        // (TODO)
    }
    // 14 ==================================================================================
    cout << endl << "== 14 === version strings =======" << endl;
    // 14 ==================================================================================
    {
        string v1, v2;

        v1 = "0.1.2.3 and then some description";
        v2 = "1";
        cout << "[" << v1 << "] is " << (SemVer(v1).bLessThan(v2) ? " less   " : " greater ") << " than [" << v2 << "]"
            << endl;

        v1 = "0.57 desc";
        v2 = "1.0";
        cout << "[" << v1 << "] is " << (SemVer(v1).bLessThan(v2) ? " less   " : " greater ") << " than [" << v2 << "]"
            << endl;
    }
    // 15 ==================================================================================
    cout << endl << "== 15 === 0xdeadbeef =======" << endl;
    // 15 ==================================================================================
    {
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

    }
    // 16 ==================================================================================
    cout << endl << "== 16 === thread stage detection =======" << endl;
    // 16 ==================================================================================
    {
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
        // Great pnsterns, use them brother!
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
        std::this_thread::sleep_for(300ms);
        cout << "done sleeping" << endl;

        // NOW we will block to ensure the thread finishes.
        cout << "joining" << endl;
        t.join();
        cout << "all done" << endl;
        // --------------------------------------------------------------------

    }
    // 17 ==================================================================================
    cout << endl << "== 17 === stupid little fizzbuzz :-) =======" << endl;
    // 17 ==================================================================================
    {
        for (int32_t i = 0; i < 100; i++)
        {
            if (i%3 ==0) cout << i << " is a fizz multiple of 3" << endl;
            if (i%5 ==0) cout << i << " is a buzz multiple of 5" << endl;
            if (i%15==0) cout << i << " is ALSO A FIZZBUZZ, WHOOHOO!" << endl;
        }
    }
    // 18 ==================================================================================
    cout << endl << "== 18 === repeat var name in deeper scope =======" << endl;
    // 18 ==================================================================================
    {
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
    }
    // nlohmann JSON, alternative to boost/json.hpp
    // 19 ==================================================================================
    cout << endl << "== 19 === Broker quiz =======" << endl;
    // 19 ==================================================================================
    {
        using json = nlohmann::json;
        string dia = read_file("data/DIA_2018.json");
        string spy = read_file("data/SPY_2018.json");
        auto dia_json = json::parse(dia);
        auto spy_json = json::parse(spy);
        for (auto& oneday:dia_json)
        {
            nstime t = isoDateStringToNstime(oneday["Date"].get<string>());
            auto d = getDayOfWeek(t);
            if (d == std::chrono::Thursday)
            {
                double close = oneday["Close"].get<double>();
                {
                if (isPrime((int)(close*1000)))
                    cout << close << " on " << ISODateFormat(t) << endl;
                }
            }
        }
        for (auto& oneday:spy_json)
        {
            nstime t = isoDateStringToNstime(oneday["Date"].get<string>());
            auto d = getDayOfWeek(t);
            if (d == std::chrono::Thursday)
            {
                double close = oneday["Close"].get<double>();
                if (isPrime((int)(close*1000)))
                {
                    cout << close << " on " << ISODateFormat(t) << endl;
                }
            }
        }
    }
    // 20 ==================================================================================
    cout << endl << "== 20 === Parse order confirmation =======" << endl;
    // 20 ==================================================================================
    {
        using json = nlohmann::json;
        // Real-world example
        json jMsg = json::parse(
            "{\"data\":{\"event\":\"new\",\"execution_id\":\"bce038d1-eed2-4a7e-8d2e-a405b483302c\",\"order\":{\"asset_class\":\"us_equity\",\"asset_id\":\"69b15845-7c63-4586-b274-1cfdfe9df3d8\",\"canceled_at\":null,\"client_order_id\":\"09c047be-0302-4696-bd27-e0884b9f3e12\",\"created_at\":\"2021-08-27T19:50:00.537731Z\",\"expired_at\":null,\"extended_hours\":false,\"failed_at\":null,\"filled_at\":null,\"filled_avg_price\":null,\"filled_qty\":\"0\",\"hwm\":null,\"id\":\"18bdb31f-25d2-4d3e-9369-d9d7f270c9d2\",\"legs\":null,\"limit_price\":null,\"notional\":null,\"order_class\":\"\",\"order_type\":\"market\",\"qty\":\"3\",\"replaced_at\":null,\"replaced_by\":null,\"replaces\":null,\"side\":\"buy\",\"status\":\"new\",\"stop_price\":null,\"submitted_at\":\"2021-08-27T19:50:00.519205Z\",\"symbol\":\"GOOGL\",\"time_in_force\":\"day\",\"trail_percent\":null,\"trail_price\":null,\"type\":\"market\",\"updated_at\":\"2021-08-27T19:50:01.011148Z\"},\"position_qty\":null,\"price\":null},\"stream\":\"trade_updates\"}"
        );

        // Log it!
        if (jMsg.count("stream"))
        {
            if (jMsg["stream"].get<string>() == "trade_updates")
            {
                stringstream ss;
                try
                {
                    json& jData = jMsg["data"];
                    json& jOrder = jData["order"];

                    string event = ( jData.count("event") ? jData["event"] : "not-found" );
                    int64_t qty = (
                            jOrder.count("qty")
                        ? (
                                jOrder["qty"].is_string()
                            ?   boost::lexical_cast<int64_t>(jOrder["qty"].get<string>()) 
                            :   jOrder["qty"].get<int64_t>()
                        ) 
                        : -1
                    );
                    int64_t filled_qty = (
                            jOrder.count("filled_qty")
                        ? (
                                jOrder["filled_qty"].is_string()
                            ?   boost::lexical_cast<int64_t>(jOrder["filled_qty"].get<string>()) 
                            :   jOrder["filled_qty"].get<int64_t>()
                        ) 
                        : -1
                    );

                    ss.str(string());
                    ss  << "Trade confirmation msg: " 
                        << " event " << event
                        << " qty " << ( jOrder.count("qty") ? jOrder["qty"] : "not-found" )
                        << " filled qty " << jOrder["filled_qty"]
                        // FIXED:
                        << " position qty " << jData["position_qty"];
                    cout << ss.str() << endl;


                    // DEBUG
                    // CRASHING soon after ^this...
                    // Check everything manually, even tho try/catch should FUCKING CATCH it.
                    // NOTE: when we cleanup, we want to add price to the above logging, so we can check partial fill pricing.
                    ss.str(string());
                    ss  << "Trade confirmation msg checked: " << jData["event"]
                        << " qty " << ( jOrder["qty"].is_string() ? boost::lexical_cast<int64_t>(jOrder["qty"].get<string>()) : -1 )
                        << " filled qty " << jOrder["filled_qty"]
                        << " position qty " << jData["position_qty"]
                        << " price " << ( jData["price"].is_string() ? boost::lexical_cast<double>(jData["price"].get<string>()) : -1.0 )
                        << " order_id " << jOrder["client_order_id"].get<string>()
                        << " side " << jOrder["side"].get<string>();
                    cout << ss.str() << endl;


                    // if (jData["event"] == "fill" || jData["event"] == "partial_fill")
                    if (jData["event"] == "fill")
                    {
                        // conf.price_ = boost::lexical_cast<double>(jData["price"].get<string>());
                        // if (jOrder["qty"].is_string())
                        //     conf.quantity_ = boost::lexical_cast<int64_t>(jOrder["qty"].get<string>());
                        // else
                        //     conf.quantity_ = jOrder["qty"].get<int64_t>();

                        // conf.order_id_ = jOrder["client_order_id"].get<string>();
                        // conf.b_buy_ = jOrder["side"].get<string>() == "buy";
                        // conf.commission_ = 0.0; // :-)
                    }
                } catch (std::exception& e)
                {
                    ss.str(string());
                    ss << "ERROR parsing trade confirmation: " << e.what() << endl << jMsg.dump();
                    cout << ss.str() << endl;
                }
            }
        }
        {
            // NOTE that auto skirts need for boost::json::value
            // but WHY doesn't value work by itself like object does??!
            // or for array? It's madness.
            //
            // I *guess* some are provided and some are not?
            //
            // Workaround is to just always provide array and value, seems to do the job.

            // boost/json.hpp
            using boost::json::value;
            using boost::json::array;
            using boost::json::object;
            using boost::json::value_from;
            using boost::json::parse;
            using boost::json::serialize;

            cout << "===================== Boost JSON notes =======================" << endl << endl;

            cout << "Boost JSON isn't THAT bad... but the string part is the WORST..." << endl;
            cout << "It uses its own string class, and doesn't provide complete bidirectional compatibility." << endl;
            cout << "Yes, everyone but the author would agree that that is batshit.  Rrr..." << endl << endl;

            cout << "The main takeaway is that to parse a string, you should include an extra string() wrapper:" << endl << endl;

            cout << "   string strQty( jOrder[\"qty\"].as_string() );" << endl << endl;

            cout << "Worst case is with Alpaca numbers, which are provided as either strings or null (idiots)." << endl;
            cout << "To parse the data to an int64_t (similar for double):" << endl << endl;

            cout << "   int64_t qty = jOrder[\"qty\"].kind() == boost::json::kind::string" << endl;
            cout << "       ? boost::lexical_cast<int64_t>( string( jOrder[\"qty\"].as_string() ))" << endl << endl;
            cout << "       : 0" << endl << endl;

            cout << "===================== Boost JSON notes =======================" << endl << endl;

            // Silly sample
            object jv = {
                { "pi", 3.141 },
                { "happy", true },
                { "name", "Boost" },
                { "nothing", nullptr },
                { "answer", {
                    { "everything", 42 } } },
                {"list", {1, 0, 2}},
                {"object", {
                    { "currency", "USD" },
                    { "value", 42.99 }
                        } }
                };
            cout << "boost json sample data" << endl << jv << endl << endl;

            // Real-world example
            // NOTE This is a pile of hand-coded work.
            // A better way will be found below...
            object jMsg = parse(
                "{\"data\":{\"event\":\"new\",\"execution_id\":\"bce038d1-eed2-4a7e-8d2e-a405b483302c\",\"order\":{\"asset_class\":\"us_equity\",\"asset_id\":\"69b15845-7c63-4586-b274-1cfdfe9df3d8\",\"canceled_at\":null,\"client_order_id\":\"09c047be-0302-4696-bd27-e0884b9f3e12\",\"created_at\":\"2021-08-27T19:50:00.537731Z\",\"expired_at\":null,\"extended_hours\":false,\"failed_at\":null,\"filled_at\":null,\"filled_avg_price\":null,\"filled_qty\":\"0\",\"hwm\":null,\"id\":\"18bdb31f-25d2-4d3e-9369-d9d7f270c9d2\",\"legs\":null,\"limit_price\":null,\"notional\":null,\"order_class\":\"\",\"order_type\":\"market\",\"qty\":\"3\",\"replaced_at\":null,\"replaced_by\":null,\"replaces\":null,\"side\":\"buy\",\"status\":\"new\",\"stop_price\":null,\"submitted_at\":\"2021-08-27T19:50:00.519205Z\",\"symbol\":\"GOOGL\",\"time_in_force\":\"day\",\"trail_percent\":null,\"trail_price\":null,\"type\":\"market\",\"updated_at\":\"2021-08-27T19:50:01.011148Z\"},\"position_qty\":null,\"price\":null},\"stream\":\"trade_updates\"}"
            ).as_object();

            // Log it!
            if (jMsg.count("stream"))
            {
                if (jMsg["stream"].as_string() == "trade_updates")
                {
                    stringstream ss;
                    try
                    {
                        object jData = jMsg["data"].as_object();
                        object jOrder = jData["order"].as_object();

                        ss.str(string());
                        ss
                            << "Trade confirmation count-checked: " 
                            << " qty-with-check " << ( jOrder.count("qty") ? jOrder["qty"] : "not-found" )
                            << " price-with-check " << ( jData.count("price") ? jData["price"] : "not-found" )
                        ;
                        cout << ss.str() << endl << endl;

                        cout << "Trade confirm bits... " << endl;
                        cout << jData.count("price") << endl;
                        cout << jData["price"].kind() << endl;
                        cout << jData["badfield"].kind() << endl;
                        // THROW: cout << boost::lexical_cast<double>( jData["price"].as_string() );
                        cout << jOrder.count("qty") << endl;
                        cout << jOrder["qty"].kind() << endl;
                        // THROW: cout << boost::lexical_cast<int64_t>( jOrder["qty"].as_string() ) << endl;
                        string strQty( jOrder["qty"].as_string() );
                        cout << boost::lexical_cast<int64_t>( strQty ) << endl;
                        cout << endl;

                        ss.str(string());
                        ss
                            << "Trade confirmation null-checked: " 
                            << " qty " << (
                                jOrder["qty"].kind() == boost::json::kind::string
                                ? boost::lexical_cast<int64_t>( string( jOrder["qty"].as_string() ))
                                : 0 )
                            << " price " << (
                                jData["price"].kind() == boost::json::kind::string
                                ? boost::lexical_cast<double>( string( jData["price"].as_string() ))
                                : 0.0 )
                        ;
                        cout << ss.str() << endl << endl;

                        string event( jData["event"].as_string() );
                        int64_t qty = jOrder["qty"].kind() == boost::json::kind::string
                            ? boost::lexical_cast<int64_t>( string( jOrder["qty"].as_string() ))
                            : 0;
                        int64_t filled_qty = jOrder["filled_qty"].kind() == boost::json::kind::string
                            ? boost::lexical_cast<int64_t>( string( jOrder["filled_qty"].as_string()))
                            : 0;
                        int64_t position_qty = jData["position_qty"].kind() == boost::json::kind::string
                            ? boost::lexical_cast<int64_t>( string( jData["position_qty"].as_string()))
                            : 0;

                        ss.str(string());
                        ss
                            << "Trade confirmation msg: " 
                            << " event " << event
                            << " qty " << qty
                            << " filled qty " << filled_qty
                            << " position qty " << position_qty
                        ;
                        cout << ss.str() << endl << endl;

                        double price = jData["price"].kind() == boost::json::kind::string
                            ? boost::lexical_cast<double>( string( jData["price"].as_string() ))
                            : 0.0;
                        ss.str(string());
                        ss
                            << "Trade confirmation msg: " 
                            << " price " << price
                        ;
                        cout << ss.str() << endl << endl;

                        // DEBUG
                        // CRASHING soon after ^this...
                        // Check everything manually, even tho try/catch should FUCKING CATCH it.
                        // ---
                        // It was the stupid Alpaca null values, eg:
                        //    data.price: null
                        // So we have to check for that.  Lame.

                        // NOTE: when we cleanup, we want to add price to the above logging, so we can check partial fill pricing.
                        ss.str(string());
                        ss
                            << "Trade confirmation msg extended: "
                            << " event " << jData["event"]
                            << " qty " << jOrder["qty"]
                            << " filled qty " << jOrder["filled_qty"]
                            << " position qty " << jData["position_qty"]
                            << " price " << jData["price"]
                            << " order_id " << jOrder["client_order_id"]
                            << " side " << jOrder["side"]
                        ;
                        cout << ss.str() << endl << endl;

                    } catch (std::exception& e)
                    {
                        ss.str(string());
                        ss << "ERROR parsing trade confirmation: " << e.what() << endl << jMsg;
                        cout << ss.str() << endl << endl;
                    }
                }
            }

            // Another bit of alpaca JSON to parse.
            // Example JSON
            // {
            //   "bars":[
            //   {
            //     "t":"2021-04-06T04:00:00Z",
            //     "o":126.5,
            //     "h":127.13,
            //     "l":125.65,
            //     "c":126.21,
            //     "v":824655675640
            //   }
            //   ],
            //   "symbol":"AAPL",
            //   "next_page_token":null
            // }
            stringstream ss;
            try
            {
                object jResponse = parse(
        R"(
        {
        "bars":[
        {
            "t":"2021-04-06T04:00:00Z",
            "o":126.5,
            "h":127.13,
            "l":125.65,
            "c":126.21,
            "v":824655675640
        }
        ],
        "symbol":"AAPL",
        "next_page_token":null
        }
        )"
                ).as_object();
                double high, low;

                array jBars = jResponse["bars"].as_array();
                object jBar = jBars[0].as_object();
                high = jBar["h"].as_double();
                low = jBar["l"].as_double();
                cout << "low, high " << low << " " << high << endl << endl;

            } catch (std::exception& e)
            {
                ss.str(string());
                ss << "ERROR parsing bars: " << e.what() << endl << jMsg;
                cout << ss.str() << endl << endl;
            }

            // vvv THIS IS WHAT YOU WANT TO DO vvv
            // whenever there is a specific set of known fields to extract.
            // However, it will fail if any are null or missing!

            // Boost JSON <-> class conversion
            // See scrap.hpp for the mapper helper functions.
            // Make sure you use a boost::json::value (not object).

            // PARSE JSON TO CLASS
            try {
                object jData = jMsg["data"].as_object();
                boost::json::value jOrder = jData["order"];
                my_order::OrderConfirmation o( value_to<my_order::OrderConfirmation>( jOrder ));

            } catch (std::exception& e)
            {
                stringstream ss;
                ss << "ERROR parsing my_order::OrderConfirmation: " << e.what() << endl;
                cout << ss.str() << endl << endl;
            }
            cout << "Successful parse" << endl << endl;

            // PARSE CLASS TO JSON
            try {
                my_order::OrderConfirmation o{ "1001", "30", "buy", "yaba-daba-order" };
                std::cout << serialize( value_from( o ) ) << endl;

            } catch (std::exception& e)
            {
                stringstream ss;
                ss << "ERROR serializing my_order::OrderConfirmation: " << e.what() << endl;
                cout << ss.str() << endl << endl;
            }
            cout << "Successful serialize" << endl << endl;
        }
    }
    // 21 ==================================================================================
    cout << endl << "== 21 === Day of week =======" << endl;
    // 21 ==================================================================================
    {
        nstime now = getCurrentTimeUtc();
        cout << now << endl;
        auto dow = getDayOfWeek(now);
        cout 
            << "Raw day of week: " << dow << endl
            << std::format(std::locale(""), "Today is {:%A}", dow) << endl      // Full name, e.g., "Thursday"
            << std::format(std::locale(""), "aka {:%a}", dow) << endl           // Abbreviated name, e.g., "Thu"
            << endl;

        // Find last monday.
        // Calculate days since Monday (0 = Monday, 6 = Sunday)
        auto days_since_monday = (dow - Monday).count();
        if (days_since_monday < 0) days_since_monday += 7;
        
        // Subtract those days from today's date to get last Monday
        auto last_monday = nstime(getDate(now)) - days(days_since_monday);
        cout << "Last Monday: " << last_monday << endl;

        cout << endl;
    }
    // 22 ==================================================================================
    cout << endl << "== 22 === Boost JSON double extraction: USE to_number<>() =======" << endl;
    // 22 ==================================================================================
    {
        // boost/json.hpp
        using boost::json::value;
        using boost::json::array;
        using boost::json::object;
        using boost::json::value_from;
        using boost::json::parse;
        using boost::json::serialize;

        // Silly sample
        object jn = {
            { "pi0", 3.141 },
            { "pi1", 3. },
            { "pi2", 3 },
            { "pi3", .141 },
            { "pi4", 3.00 },
            { "pi5", "3.1416" },
        };
        cout << "boost json numeric samples" << endl << jn << endl;

        cout << "pi0: " << jn["pi0"] << " - " << jn["pi0"].to_number<double>() << endl;
        cout << "pi1: " << jn["pi1"] << " - " << jn["pi1"].to_number<double>() << endl;
        cout << "pi2: " << jn["pi2"] << " - " << jn["pi2"].to_number<double>() << endl;
        cout << "pi3: " << jn["pi3"] << " - " << jn["pi3"].to_number<double>() << endl;
        cout << "pi4: " << jn["pi4"] << " - " << jn["pi4"].to_number<double>() << endl;
        cout << "pi5: " << jn["pi5"] << " - " << boost::lexical_cast<double>( jn["pi5"].as_string().c_str() ) << endl;
        cout << endl;
    }
    // 23 ==================================================================================
    cout << endl << "== 23 === Boost JSON array stupidity =======" << endl;
    // 23 ==================================================================================
    {
        // boost/json.hpp
        // NOTE that array is in NLOHMANN and in BOOST so we have to specify namespace during usage.
        using boost::json::value;
        // using boost::json::array;
        using boost::json::object;
        using boost::json::value_from;
        using boost::json::parse;
        using boost::json::serialize;

        cout << "Use value_from() for vector to boost::json::array conversions..." << endl;
        vector< string > vs = { "one", "two", "three" };
        boost::json::value bvs = value_from( vs );
        boost::json::array abvs = bvs.as_array();

        cout << "In theory you can use tag_invoke() then value_to() for boost::json::array to vector conversions... PITA..." << endl;
        cout << "In practice, people bail and use boost ptree or nlohmann, that is JUST SAD..." << endl;
        cout << "Let's just manually loop the damned thing then." << endl;
        const boost::json::array cja = { "one", "two", "three" };
        vector<string> vcja;
        for ( auto& jv : cja )
            vcja.push_back( string( jv.as_string()));
        for ( auto& jv : vcja )
            cout << jv;

        cout << endl;
    }
    // 24 ==================================================================================
    cout << endl << "== 24 === Boost JSON const stupidity =======" << endl;
    // 24 ==================================================================================
    {
        const object cjn = {
            { "pi0", 3.141 },
            { "pi1", 3. },
            { "pi2", 3 },
            { "pi3", .141 },
            { "pi4", 3.00 },
            { "pi5", "3.1416" },
        };
        cout << cjn.at("pi0") << endl;
        // cout << cjn["pi0"] << endl;  // FAILS TO COMPILE.  YOU SUCK BOOST.
    }
    // 25 ==================================================================================
    cout << endl << "== 25 === std::chrono THE FUTURE =======" << endl;
    // 25 ==================================================================================
    {
        using namespace std::chrono;

        // ---------------------------------------------------------------
        // start/stop a clock and capture the duration (use steady_clock)
        // ---------------------------------------------------------------

        // ---------------
        // IN MILLISECONDS
        // ---------------
        // defs
        // using duration_ms = duration<double, std::ratio<1, 1000>>;
        typedef duration<double, std::ratio<1, 1000>> duration_ms;

        // bracket an event
        auto t0 = steady_clock::now();
        std::this_thread::sleep_for(50ms);
        auto t1 = steady_clock::now();

        // capture the duration
        duration_ms d = t1 - t0;
        std::cout << d.count() << " milliseconds\n";      
        // ---------------

        // ---------------
        // IN SECONDS
        // ---------------
        // defs
        typedef duration<float> duration_seconds;

        auto t2 = steady_clock::now();
        std::this_thread::sleep_for(1300ms);
        auto t3 = steady_clock::now();
        duration_seconds fs = t3 - t2;
        std::cout << fs.count() << " seconds\n";
        // ---------------

        // ---------------------------------------------------------------
        // capture and output a timestamp (use system_clock):
        // ---------------------------------------------------------------
        auto t4 = system_clock::now();
        const std::time_t tt_t4 = std::chrono::system_clock::to_time_t(t4);
        const string strLocalTime = std::ctime(&tt_t4);
        std::cout << "time: " << tt_t4 << " " << strLocalTime << std::endl;

        // TIMEZONE support is here!  Yay!  
        // Arrived for me in gcc (Ubuntu 14.2.0-19ubuntu2) 14.2.0
        // NOTE gcc 11.4.0 was supposed to have this, but no go.
        std::cout << std::chrono::zoned_time{"Asia/Singapore", system_clock::now()} << '\n';
        std::cout << std::chrono::zoned_time{"America/New_York", system_clock::now()} << '\n';
        std::cout << std::chrono::zoned_time{"Europe/London", system_clock::now()} << '\n';
        std::cout << std::chrono::zoned_time{"America/Los_Angeles", system_clock::now()} << '\n';
        std::cout << std::chrono::zoned_time{"America/Chicago", system_clock::now()} << '\n';
    }
    // 26 ===================================================================================
    cout << endl << "== 26 === lambda redux ========" << endl;
    // 26 ===================================================================================
    {
        // WHAT WE WANT
        // Pass lambda by value
        // That's the way the STL does it.
        // It's pretty ideal as long as there is not a lot of data contained in the parameter(s).
        // And we never pass a lot of data - we try to always use refs - so this is perfect.
        // MyObject m;
        // m.useLambdaToModifyOurselves( [&m](){ m.x_ = 555; } );
        // std::cout << "m.x_ is " << m.x_ << std::endl;

        // some other examples from stack overflow
        use_it([](){std::cout << "HALLO" << std::endl;});

        MyObject mo;
        mo.SetXGetter([](){std::cout << "BEYE" << std::endl; return 0;});
        mo.call();

        mo.SetXGetterProperForwarding([n=42](){std::cout << "FORWARD BEYE " << n << std::endl; return n + 19;});
        int v = mo.callProperForwarding();
        std::cout << "v is " << v << std::endl;

        A a;
        a.a_ = 10;
        auto func = [](A& a_ref){ a_ref.a_ += 123; };
        mo.applyFunctionToA(a, &func);
        std::cout << "a.a_ is " << a.a_ << std::endl;
        mo.applyFunctionToAWithDefault(a, func);
        std::cout << "a.a_ is " << a.a_ << std::endl;
        mo.applyFunctionToAWithDefault(a);
        std::cout << "a.a_ is " << a.a_ << std::endl;
    }
    // 27 ==================================================================================
    run_from_here:
    cout << endl << "== 27 === boost::posix > std::chrono REFACTOR =======" << endl;
    // 27 ==================================================================================
    {
        // it's "TIME" (ugg) to switch completely out of boost::posix to std::chrono
        // We decided NANOSECONDS are the way to go.
        // They fit in 64 bits, and cover from 1970 to 2262.
        // utilities.hpp completely sets up our nstime clock (where ns = nanoseconds).

        nstime start_time = getCurrentTimeUtc();

        // Analyze std::chrono clocks, including precision, storage and year limits.
        cout << endl;
        cout << "FIRST NOTE that nstime::max() is meaningless as a timestamp." << endl;
        cout << nstime::max() << " nstime::max()" << endl;
        cout << "  ^ 'max(): Returns a time_point with the largest possible duration'; streamed value is MEANINGLESS" << endl;
        cout << nstime::max().time_since_epoch() << " nstime::max().time_since_epoch(), basically MAX - ZERO, same as std::numeric_limits<int64_t>::max()"<< endl;
        cout << "  ^ this is basically MAX - ZERO, same as std::numeric_limits<int64_t>::max(); it has no meaning as a date" << endl;
        cout << "    To get meaning, you have to use system_clock, which has a 1970 epoch, and convert to your clock's duration." << endl;
        cout << endl;

        // TODAY
        auto dpn = floor<std::chrono::days>(system_clock::now());
        year_month_day ymdn{dpn};
        int64_t y = int(ymdn.year());
        cout << endl << "Year " << y << endl;
        cout << "Today is " << dpn << ", using year_month_day of system_clock" << endl << endl;

        // Vet 64-bit limits, signed vs unsigned
        cout << "FIRST: RAW 64-bit signed and unsigned limits:\n";
        cout << std::pow(2,63) << " 2^63\n";
        int64_t max_int64_t = std::numeric_limits<int64_t>::max();
        cout << max_int64_t << " max int64_t\n";
        cout << std::numeric_limits<uint64_t>::max() << " max uint64_t\n";
        cout << "Note: we will use int64_t, to support negative time in calculations." << endl;
        cout << endl;

        // --- YEAR ---
        cout << "NEXT: YEAR RANGE in ms and us clocks, stored in int64_t" << endl;
        cout << std::numeric_limits<int64_t>::max() / 1e6 << " seconds can fit, in microseconds\n";
        int64_t us_in_a_year = 365 * 24 * 60 * 60 * 1e6;
        cout << us_in_a_year << " microseconds in a year\n";
        int64_t year_range_us = max_int64_t / us_in_a_year;
        cout << year_range_us << " theoretical year range in microseconds" << endl;
        cout << endl;

        cout << std::numeric_limits<int64_t>::max() / 1e9 << " seconds can fit, in nanoseconds\n";
        int64_t ns_in_a_year = 365 * 24 * 60 * 60 * 1e9;
        cout << ns_in_a_year << " nanoseconds in a year\n";
        int64_t year_range_ns = max_int64_t / ns_in_a_year;
        cout << year_range_ns << " theoretical year range in nanoseconds" << endl;
        cout << endl;

        // --- NSTIME ---
        cout << endl;
        cout << "FINALLY: the NSTIME clock and usage";
        cout << endl;
        cout << endl;

        cout << "nstime::min() " << nstime::min() << " aka " << getNanoseconds(nstime::min()) << endl;
        cout << "nstime::max() " << nstime::max() << " aka " << getNanoseconds(nstime::max()) << endl;
        cout << endl;

        auto dpnat = floor<std::chrono::days>(getCurrentTimeUtc());
        year_month_day ymdnat{dpnat};
        int64_t yat = int(ymdnat.year());
        cout << endl << "Year " << yat << endl;
        cout << "Today is " << getDayOfWeek(dpnat) << ", " << dpnat << ", using year_month_day of our clock" << endl << endl;

        {
            const string iso_date = "2025-12-17";
            nstime t = isoDateStringToNstime( iso_date );
            auto d = getDayOfWeek(t);
            auto idf = ISODateFormat(t);
            cout << "Parsing ISO date string " << iso_date << "..." << endl;
            cout << "getDayOfWeek: " << d << endl;
            cout << "ISODateFormat: " << idf << endl;

            auto days_since_monday = (d - Monday).count();
            if (days_since_monday < 0) days_since_monday += 7;
            
            // Subtract those days from today's date to get last Monday
            auto last_monday = nstime(getDate(t)) - days(days_since_monday);
            cout << "Previous Monday: " << last_monday << endl;

            cout << endl;
        }

        // hour, minute, second
        // Worth a read, thanks Howard Hinnant:
        // https://stackoverflow.com/a/15958113/717274

        const string iso_timestamps[] = {
            "2025-12-17T15:45:30.123456789Z",
            "2025-12-17T15:45:30Z",
            "2025-2-7T15:5:1Z"
        };
        for ( const auto& iso_timestamp : iso_timestamps )
        {
            nstime t = utcStringToNstime( iso_timestamp );
            year_month_day ymd = getYMD(t);
            hh_mm_ss time = getHMS(t);
            auto y = ymd.year();
            auto m = ymd.month();
            auto d = ymd.day();
            auto h = time.hours();
            auto M = time.minutes();
            auto s = time.seconds();
            auto ns = time.subseconds();
            cout << "Parsing ISO timestamp string " << iso_timestamp << "..." << endl;
            cout << "Year: " << y << endl;
            cout << "Month: " << m << endl;
            cout << "Day: " << d << endl;
            cout << "Hour: " << h.count() << endl;
            cout << "Minute: " << M.count() << endl;
            cout << "Second: " << s.count() << endl;
            cout << "Nanosecond: " << ns.count() << endl;

            nsduration diff = t.time_since_epoch();
            cout << "Elapsed time since epoch: " << endl;
            cout << diff << endl;
            cout << getElapsedTime(diff) << " (getElapsedTime)" << endl;
            cout << getSeconds( diff ) << " seconds (getSeconds(nsduration))" << endl;
            cout << getSeconds( t ) << " seconds (getSeconds(nstime))" << endl;

            cout << endl;
        }

        nstime end_time = getCurrentTimeUtc();
        nsduration diff = end_time - start_time;
        cout << "Elapsed time since this scrap step started:" << endl;
        cout << diff << endl;
        cout << getElapsedTime(diff) << " (getElapsedTime)" << endl;
        cout << getSeconds( diff ) << " seconds (getSeconds)" << endl;
        cout << endl;

        cout << "Ended std::chrono scrap at " << end_time << endl;
        cout << endl;

        cout << "OUR FORMATTERS:" << endl;
        cout << "ISODateFormat: " << ISODateFormat( end_time ) << endl;
        cout << "ISOFormat: " << ISOFormat( end_time ) << endl;
        cout << "RFC3339Format: " << RFC3339Format( end_time ) << endl;
        cout << "americanFormat: " << americanFormat( end_time ) << endl;
        cout << "Year + nstimeToString(%Y  Month of %h): " << "Year " <<nstimeToString( end_time, "%Y  Month of %h" ) << endl;
        cout << "nstimeToString(%Y): " << nstimeToString( end_time, "%Y" ) << endl;
        cout << "nstimeToString(%Y-%m-%d %H:%M:%S): " << nstimeToString( end_time, "%Y-%m-%d %H:%M:%S" ) << endl;
        cout << "nstimeToString(%Y-%m-%d %H %M %S): " << nstimeToString( end_time, "%Y-%m-%d %Hh %Mm %Ss" ) << endl;
        cout << "nstimeToString(%Y-%m-%d %T): " << nstimeToString( end_time, "%Y-%m-%d %T" ) << endl;
        cout << "nstimeToString(%Y-%m-%d %H): " << nstimeToString( end_time, "%Y-%m-%d %H" ) << endl;
        cout << "nstimeToString(%Y-%m-%d %M): " << nstimeToString( end_time, "%Y-%m-%d %M" ) << endl;
        cout << "nstimeToString(%Y-%m-%d %S): " << nstimeToString( end_time, "%Y-%m-%d %S" ) << endl;
        cout << "nstimeToString(%Y-%m-%d %T): " << nstimeToString( end_time, "%Y-%m-%d %T" ) << endl;
        cout << endl;
        cout << "NOTE C++23 can't yet specify precision, eg this fails:" << endl;
        cout << "nstimeToString(%Y-%m-%d %H:%M:%2S)" << endl;
        cout << "Check back, C++26 may bring improvements" << endl;
        cout << endl;
    }

    // ========= end ========
    // We can keep it running if needed to better see the output.
    // while (true)
    // {}
}
