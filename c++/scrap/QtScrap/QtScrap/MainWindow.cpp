#include <QFile>
#include <QMessageBox>
#include <QTextStream>
#include "MainWindow.h"
#include "ui_MainWindow.h"


MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    // MDM Force the layout.
    // Never quite sure why I have to force the top layout.....
    ui->centralWidget->setLayout(ui->gridLayout);

    // MDM change one edit to be the child of the other, and overlap them
    // ui->textEdit_2->setParent(ui->textEdit);


    //=============================================================
    // Algo Homework 1
    //=============================================================
    /*
    // read file
    QFile file("../IntegerArray.txt");
    // QFile file("../testReverseOrdered.txt");
    // QFile file("../SixToOne.txt");
    if(!file.open(QIODevice::ReadOnly)) {
        QMessageBox::information(0, "error", file.errorString());
    }
    QTextStream in(&file);
    std::vector<int> v_array;
    while(!in.atEnd()) {
        QString line = in.readLine();
        v_array.push_back(line.toInt());
    }
    file.close();
    ui->textEdit_2->setText(QString("%1").arg(v_array.size()));

    // calc inversions
    // pseudocode:
    //
    //     sort_and_count(input,100000)
    //
    //         split input in half
    //         x = sort_and_count left
    //         y = sort_and_count right
    //         count_split_inversions(x,y,A,n)
    //             walk first array
    //             if element is smaller than "next" right array, just copy it out
    //             if element is larger, copy right element; inversions += number of items left in the left array
    //
    m_n_total_inversions = 0;
    sort_and_count_inversions(v_array);
    ui->textEdit->setPlainText(QString("Total inversions = %1").arg(m_n_total_inversions));
    */
    //=============================================================


    //=============================================================
    // Algo Homework 2
    //=============================================================
    /*
    // got it after some debugging
    // matched PA2 Test Cases from forum:
    //     input array
    //     [1..10] in order                     45      45      19
    //     [1..10] in reverse order             45      45      19
    //     [1..100] in order                    4950    4950    480
    //     [1..100] in reverse order            4950    4950    1302
    //     [2, 8, 9, 3, 7, 5, 10, 1, 6, 4]      25      20      19
    //     the IntegerArray.txt file from PA
    //
    // to create files:
    //      seq 100 -1 1 > testReverseOrdered.txt
    //      seq 1 1 100 > OneToOneHundred.txt
    //
    QFile file("../QuickSort.txt");
    // QFile file("../OneToTen.txt");
    // QFile file("../testReverseOrdered.txt");
    // QFile file("../test2test.txt");
    // QFile file("../TenToOne.txt");

    // read file
    if(!file.open(QIODevice::ReadOnly)) {
        QMessageBox::information(0, "error", file.errorString());
    }
    QTextStream in(&file);
    std::vector<int> v_array;
    while(!in.atEnd()) {
        QString line = in.readLine();
        v_array.push_back(line.toInt());
    }
    file.close();

    m_n_total_comparisons = 0;
    // m_n_total_comparisons = v_array.size() - 1;
    sort_and_count_comparisons(v_array);
    ui->textEdit->setPlainText(QString("Comparisons = %1").arg(m_n_total_comparisons));

    // show the sorted array, too
    QString str_sorted;
    for (VIntIt it = v_array.begin(); it != v_array.end(); ++it)
        str_sorted += QString("%1\n").arg(*it);
    ui->textEdit_2->setText(str_sorted);
    */
    //=============================================================


    //=============================================================
    // Algo Homework 3
    //=============================================================
    /*

    The file contains the adjacency list representation of a simple undirected graph.
    There are 200 vertices labeled 1 to 200.
    The first column in the file represents the vertex label,
    and the particular row (other entries except the first column) tells all the vertices that the vertex is adjacent to.
    So for example, the 6th row looks like : "6	155	56	52	120	......".
    This just means that the vertex with label 6 is adjacent to (i.e., shares an edge with) the vertices with labels 155,56,52,120,......,etc

    Your task is to code up and run the randomized contraction algorithm for the min cut problem and use it on the above graph to compute the min cut.
    (HINT: Note that you'll have to figure out an implementation of edge contractions.
    Initially, you might want to do this naively, creating a new graph from the old every time there's an edge contraction.
    But you should also think about more efficient implementations.)
    (WARNING: As per the video lectures, please make sure to run the algorithm many times with different random seeds, and remember the smallest cut that you ever find.)
    Write your numeric answer in the space provided. So e.g., if your answer is 5, just type 5 in the space provided.

    btw the answer is not 11 or 20

    */

    /*
    // QFile file("../kargerEight.txt");
    // QFile file("../kargerOneExtra.txt");
    // QFile file("../kargerThreeExtra.txt");
    // QFile file("../kargerTwentyVertices.txt");
    QFile file("../kargerMinCut.txt");

    // read file
    if(!file.open(QIODevice::ReadOnly)) {
        QMessageBox::information(0, "error", file.errorString());
    }
    QTextStream in(&file);

    std::map<int,std::vector<int> > orig_map_vertices;
    while(!in.atEnd()) {
        QString line = in.readLine();

        // Each line has a vertex and a list of adjacent vertices
        QStringList str_vertices = line.split(QRegExp("\\s"),QString::SkipEmptyParts);

        std::vector<int> v_adjacents;
        for (int n = 1; n < str_vertices.length(); ++n)
        {
            v_adjacents.push_back(str_vertices.at(n).toInt());
        }
        orig_map_vertices[str_vertices.at(0).toInt()] = v_adjacents;
    }
    file.close();

    QString m_strout;

    // map sanity check
    // for (std::map<int,std::vector<int> >::iterator it = map_vertices.begin(); it != map_vertices.end(); ++it)
    // {
    //     m_strout += QString("%1 ").arg(it->first);
    // }
    // ui->textEdit->setPlainText(m_strout);

    // build an edge map
    std::map<intpair,bool> orig_m_edges;
    for (std::map<int,std::vector<int> >::iterator it = orig_map_vertices.begin(); it != orig_map_vertices.end(); ++it)
    {
        std::vector<int> ved = it->second;
        for (std::vector<int>::iterator it2 = ved.begin(); it2 != ved.end(); ++it2)
        {
            intpair ip;
            ip.v1 = std::min(it->first,*it2);
            ip.v2 = std::max(it->first,*it2);
            orig_m_edges[ip] = true;
        }
    }

    // Edges look good?  yep!
    // ha, didn't use em tho, once i figured out that i wasn't reusing the orig array on each run, DOH
    // fixed that and i was golden
    // for (std::map<intpair,bool >::iterator it = orig_m_edges.begin(); it != orig_m_edges.end(); ++it)
    // {
    //     m_strout += QString("{%1,%2}\n").arg(it->first.v1).arg(it->first.v2);
    // }
    // ui->textEdit_2->setPlainText(m_strout);

    int maxloops = 1000;
    int loops = 0;
    int mincuts = 200;
    while (loops < maxloops)
    {
        std::map<intpair,bool> m_edges = orig_m_edges;
        std::map<int,std::vector<int> > map_vertices = orig_map_vertices;

        while (map_vertices.size() > 2)
        {
            // pick an edge uniformly at random
            // every edge should be in the list twice
            // count the list of edges, pick a random number, "walk" to that edge

            int edges = 0;
            for (std::map<int,std::vector<int> >::iterator it = map_vertices.begin(); it != map_vertices.end(); ++it)
            {
                edges += it->second.size();
            }
            int random_edge = rand() % edges;
            int v1 = -1;
            int v2 = -1;
            edges = 0;
            for (std::map<int,std::vector<int> >::iterator it = map_vertices.begin(); it != map_vertices.end(); ++it)
            {
                if (edges + it->second.size() > random_edge)
                {
                    v1 = it->first;
                    v2 = it->second.at(random_edge - edges);
                    break;
                }
                edges += it->second.size();
            }

            // m_strout += QString("{%1,%2}\n").arg(v1).arg(v2);


            // contract the edge's nodes
            // get rid of self loops
            // but track "double links"
            // ha easy
            //      remove all v2 from v1 list
            //      move v2 list to v1 list where item != v1
            //      remove v2 from map_vertices
            std::vector<int> v1_adj = map_vertices[v1];
            std::vector<int> v2_adj = map_vertices[v2];
            std::vector<int>::iterator it = v1_adj.begin();
            while ( it != v1_adj.end())
            {
                if ((*it) == v2)
                    v1_adj.erase(it);
                else
                    ++it;
            }
            for (std::vector<int>::iterator it = v2_adj.begin(); it != v2_adj.end(); ++it)
            {
                if ((*it) != v1)
                    v1_adj.push_back(*it);
            }
            map_vertices[v1] = v1_adj;
            map_vertices.erase(v2);

            // whoa
            // now i need to patch all other references to the deleted v2 to point to the merged v1
            // we can find them all by walking v1's list out one level
            // that's gross
            // but should work
            for (std::vector<int>::iterator it = v1_adj.begin(); it != v1_adj.end(); ++it)
            {
                std::vector<int> patched_v1_neighbor = map_vertices[*it];
                for (std::vector<int>::iterator itinner = patched_v1_neighbor.begin(); itinner != patched_v1_neighbor.end(); ++itinner)
                {
                    if ((*itinner) == v2)
                        (*itinner) = v1;
                }
                map_vertices[*it] = patched_v1_neighbor;
            }

        }

        // determine remaining cut for the loop
        int loopcuts = map_vertices.begin()->second.size();

        if (loopcuts<mincuts)
            mincuts=loopcuts;

        ++loops;
    }

    ui->textEdit->setPlainText(QString("%1").arg(mincuts));
    */

    //=============================================================


    //=============================================================
    // Algo Homework 4
    //=============================================================
    // Find the strongly-connected components in a graph.
    //=============================================================

    /*
    // Each line has a tail vertex and a head vertex
    // Inconvenient, we'll turn it into an adjacency list.
    // QFile file("../scc_lecture.txt");
    // QFile file("../scc_test2.txt");
    // QFile file("../test7.txt");
    QFile file("../SCC.txt");

    // read file
    if(!file.open(QIODevice::ReadOnly)) {
        QMessageBox::information(0, "error", file.errorString());
    }
    QTextStream in(&file);

    int n = 0;
    VInt v_adjacents;
    graph.push_back(v_adjacents);
    while(!in.atEnd()) {
        QString line = in.readLine();

        QStringList str_vertices = line.split(QRegExp("\\s"),QString::SkipEmptyParts);

        // ADJUST ALL INPUT ARRAYS FROM START-AT-1 INDEXING TO START-AT-0.
        int tail = str_vertices.at(0).toInt() - 1;
        int head = str_vertices.at(1).toInt() - 1;

        if (tail == n)
        {
            graph[n].push_back(head);

        } else
        {
            while (tail > n)
            {
                graph.push_back(v_adjacents);
                ++n;
            }
            graph[n].push_back(head);
        }
    }
    file.close();

    // Get the reverse.
    // We now know how big so we can preallocate.
    // Then just save the tail->head pairs as head->tail pairs.
    graph_rev.resize(graph.size());
    n = 0;
    for (AdjacencyListIt it = graph.begin(); it != graph.end(); ++it)
    {
        VInt& adjs = (*it);
        for (VIntIt ita = adjs.begin(); ita != adjs.end(); ++ita)
            graph_rev[(*ita)].push_back(n);
        ++n;
    }

    // graph sanity check
    // We'll add one when outputting so we match the class notes.
    n = 0;
    for (AdjacencyListIt it = graph.begin(); it != graph.end(); ++it)
    {
        m_strout += QString("%1: ").arg(n+1);
        VInt& adjs = (*it);
        for (VIntIt ita = adjs.begin(); ita != adjs.end(); ++ita)
            m_strout += QString("%1 ").arg((*ita) + 1);
        m_strout += QString("\n");
        ++n;
    }
    n = 0;
    for (AdjacencyListIt it = graph_rev.begin(); it != graph_rev.end(); ++it)
    {
        m_strout += QString("%1: ").arg(n+1);
        VInt& adjs = (*it);
        for (VIntIt ita = adjs.begin(); ita != adjs.end(); ++ita)
            m_strout += QString("%1 ").arg((*ita) + 1);
        m_strout += QString("\n");
        ++n;
    }
    ui->textEdit_2->setPlainText(m_strout);
    m_strout.clear();

    // Here's the algo explanation from lectures.
    // Figuring out what is going on looks like the toughest part.
    // Also making sure we don't overflow the stack recursing.
    //
    // for each vertex n..0
    //  run DFS-Loop, gathering "finishing time" of each vertex
    // for each vertex 0..n
    //  run DFS-Loop, processing in decreasing order of finishing times
    //  each SCC is found under each "leader"
    //

    // t = 0  (#nodes explored so far)
    // s = null (leader)
    // loop backwards thru nodes for i = (n-1) to 0
    //      if node not seen
    //          s = i "the leader for this recursion"
    //          DFS(G,i)
    //          ---
    //              mark i as explored FOR THIS LOOP
    //              set leader(i) = node s
    //              for each arc
    //                  if j not explored
    //                      DFS(G,j)
    //              t++
    //              set f(i) = t
    //          ---
    //
    //

    // (Re)set flags.
    m_been_there.assign(graph.size(), false);
    m_leader.assign(graph.size(), -1);
    m_finish.assign(graph.size(), -1);
    m_nodes_for_loop2.assign(graph.size(), -1);

    // LOOP ONE
    // Loop the reverse graph from last node to first
    // In this case, n = actual graph slot.
    // Adjust by -1 when accessing our vectors.
    m_t = 0;
    m_s = -1;
    int last_n = graph_rev.size() - 1;
    for (int i = last_n; i >= 0; --i)
    {
        if (!m_been_there[i])
        {
            // who cares in loop one
            // m_s = i;

            DFSDigOne(i);
        }
    }

    // ^^ Checked and validated against lecture example...

    // (Re)set flags.
    m_been_there.assign(graph.size(), false);
    m_leader.assign(graph.size(), -1);
    m_finish.assign(graph.size(), -1);

    // Not good enough, the adjacency vectors are not updated to match
    // {
    //     AdjacencyList graph_new;
    //     graph_new.resize(graph.size());
    //     n = 0;
    //     for (AdjacencyListIt it = graph.begin(); it != graph.end(); ++it)
    //     {
    //         graph_new[m_finish[n]] = graph[n];
    //         ++n;
    //     }
    //     graph = graph_new;
    // }

    // LOOP TWO
    // Loop original array
    // SUBSTITUTING FINISH for original vector numbering.  WHAT??  :P
    // how do you process the nodes in decreasing order of finishing time
    // you dont want to sort the nodes
    // remember them in a way that you can do a linear pass

    // I need a list of nodes in REVERSE FINISHING ORDER from first pass

    // ie, not this...
    // last_n = graph.size() - 1;
    // for (int i = last_n; i >= 0; --i)
    // {

    m_css_count = -1;
    for (VIntRit it2 = m_nodes_for_loop2.rbegin(); it2 != m_nodes_for_loop2.rend(); ++it2)
    {
        int i = (*it2);

        if (!m_been_there[i])
        {
            // Save the previous css count.
            if (m_css_count != -1)
                save_css();

            // New css, prep to count!
            m_s = i;
            m_css_count = 0;

            // LOOP TWO sanity check
            // m_strout += QString("%1\n").arg(i + 1);

            DFSDigTwo(i);
        }
    }
    save_css();

    // m_v_css has our counts, find the top 5 and report em
    m_strout = QString("Counts\n");
    n = 0;
    std::sort(m_v_css.begin(),m_v_css.end(),std::greater<int>());
    for (VIntIt itc = m_v_css.begin(); itc != m_v_css.end() && n < 5; ++itc)
    {
        m_strout += QString("%1\n").arg((*itc));
        n++;
    }

    ui->textEdit->setPlainText(m_strout);
    */


    //=============================================================
    // Algo Homework 5
    //=============================================================
    // code up Dijkstra's shortest-path algorithm
    //=============================================================

    /*
    QFile file("../dijkstraData.txt");

    // read file
    if(!file.open(QIODevice::ReadOnly)) {
        QMessageBox::information(0, "error", file.errorString());
    }
    QTextStream in(&file);

    int n = 0;
    while(!in.atEnd()) {
        QString line = in.readLine();

        // Input is: vertex adj_vertex,length adj_vertex,length adj_vertex,length
        // ADJUST ALL INPUT ARRAYS FROM START-AT-1 INDEXING TO START-AT-0.

        QStringList str_vertices = line.split(QRegExp("\\s"),QString::SkipEmptyParts);
        WeightedAdjacencyListItem item;
        item.n = str_vertices.at(0).toInt() - 1;
        for (int n = 1; n < str_vertices.length(); ++n)
        {
            QStringList tuple = str_vertices.at(n).split(QString(","));
            intpair nextpair;
            nextpair.v1 = tuple.at(0).toInt() - 1;
            nextpair.v2 = tuple.at(1).toInt();
            item.adjs.push_back(nextpair);
        }
        wgraph.push_back(item);
    }
    file.close();

    // graph sanity check
    // We'll add one when outputting so we match the class notes.
    n = 0;
    for (WeightedAdjacencyListIt it = wgraph.begin(); it != wgraph.end(); ++it)
    {
        WeightedAdjacencyListItem& item = (*it);
        m_strout += QString("%1: ").arg(item.n + 1);

        VPair& adjs = item.adjs;
        for (VPairIt ita = adjs.begin(); ita != adjs.end(); ++ita)
            m_strout += QString("%1,%2 ").arg((*ita).v1 + 1).arg((*ita).v2);
        m_strout += QString("\n");
        ++n;
    }
    ui->textEdit->setPlainText(m_strout);

    m_strout.clear();

    // Along the way, grab and output answers for these:
    int an[] = {
        2, 3, 4, 5, 6,                                  // test case
        7, 37, 59, 82, 99, 115, 133, 165, 188, 197,     // actual assignment
        -1                                              // terminator
    };
    VInt ans(&an[0], &an[0]+sizeof(an));
    int next_ans_index = 0;

    m_A.assign(wgraph.size(), 1000000); // set shortest distances high to start

    m_X.push_back(0);                   // our starting point
    m_A[0] = 0;

    while (m_X.size() < 199)
    {
        // look for each vertex that crosses out of X
        //  start by looping through all of X
        int min_greedy = -1;
        int min_w = -1;
        for (VIntIt itx = m_X.begin(); itx != m_X.end(); ++itx)
        {
            WeightedAdjacencyListItem& item = wgraph[*itx];
            int v = item.n;

            // get the adjacency list for this vertex
            // loop through the heads and find lowest greedy
            VPair& adjs = item.adjs;
            for (VPairIt ita = adjs.begin(); ita != adjs.end(); ++ita)
            {
                // is this next head outside of X?
                int w = (*ita).v1;
                int length = (*ita).v2;
                bool b_inside_X = false;
                for (VIntIt itx = m_X.begin(); itx != m_X.end(); ++itx)
                {
                    if (w == (*itx))
                    {
                        b_inside_X = true;
                        break;
                    }
                }

                // if we found one outside of X, we have a (v,w) to consider
                // we want to get its greedy and compare to others
                // we will be picking the vertex for the smallest greedy to add to X next
                if (!b_inside_X)
                {
                    // compute greedy
                    int greedy = m_A[v] + length;
                    if (min_greedy == -1 || greedy < min_greedy)
                    {
                        min_w = w;
                        min_greedy = greedy;
                    }
                }
            }
        }
        if (min_greedy > -1)
        {
            // We found our new vertex, add to X and set its A.
            m_X.push_back(min_w);
            m_A[min_w] = min_greedy;

            // At this point we have a min path from the first vertex to w.
            // Spit it out as needed.
            for (VIntIt ita = ans.begin(); ita != ans.end(); ++ita)
            {
                if (min_w+1 == (*ita))
                {
                    // print the answer and prep for next.
                    m_strout += QString("shortest path for %1 = %2\n").arg(min_w+1).arg(min_greedy);
                }
            }
        }
    }

    //=============================================================
    */


    //=============================================================
    // Algo Homework 6
    //=============================================================
    //
    // #1 use a heap to find int pairs that add up to the same sum
    //
    // we need to find which values in the set (2500..4000) can be created from the sum of two distict values from the input
    // the input values have duplicates
    // it appears that this is the desired approach:
    //
    //      read in the numbers, using some kind of hash to store the number of times you see a number in the input
    //      for a given T
    //          totalcombos = 0
    //          walk the hash - for each element...
    //              find the "other" number needed to make the sum T
    //              is there at least one instance of it?  if so, add T to the success list
    //
    // A: 1479
    // tc1: 50
    //
    //=============================================================

    // add each index
    QFile file("../HashInt.txt");
    // QFile file("../TwoSumMedium.txt");

    // read file
    if(!file.open(QIODevice::ReadOnly)) {
        QMessageBox::information(0, "error", file.errorString());
    }
    QTextStream in(&file);

    int n = 0;
    while(!in.atEnd()) {
        QString line = in.readLine();

        // Mark the bucket
        m_buckets[line.toInt()] = true;
    }
    file.close();

    // loop through all the Ts
    int t_count = 0;
    for (int t = 2500; t <=4000; ++t)
    // for (int t = 30; t <= 60; ++t)
    {
        // loop through the map
        for (HashBucketSetIt it = m_buckets.begin(); it != m_buckets.end(); ++it)
        {
            int x = (*it).first;
            int y = t - x;
            if ( x != y )
            {
                if (m_buckets.find(y) != m_buckets.end())
                {
                    // m_strout += QString("%1 = %2 + %3\n").arg(t).arg(x).arg(y);
                    ++t_count;
                    break;
                }
            }
        }
    }
    m_strout += QString("#Ts = %1\n").arg(t_count);


    //=============================================================
    // Algo Homework 6
    //=============================================================
    //
    // #2 (SERIOUSLY??) "median maintenance"
    // find the sum of each median in a stream of integers loaded one at a time
    // fuck it im doing this one brute force
    //=============================================================
    // add each index
    QFile file2("../Median.txt");
    // QFile file2("../assign6_tc1.txt");

    // read file
    if(!file2.open(QIODevice::ReadOnly)) {
        QMessageBox::information(0, "error", file2.errorString());
    }
    QTextStream in2(&file2);

    long long int sum = 0;
    VInt vi;
    while(!in2.atEnd()) {
        QString line = in2.readLine();

        // insert into a vector
        vi.push_back(line.toInt());

        // sort it
        std::sort(vi.begin(),vi.end());

        // find the median
        // median of 1,2,3 is 2
        // median of 1,2,3.4 is 2
        int median_index = (int)(vi.size()/2.0 + 0.5) - 1;
        int median = vi[median_index];
        sum += median;
    }
    file2.close();

    m_strout += QString("Median sum = %1\n").arg(sum);


    //=============================================================
    // stackoverflow
    //=============================================================


    ui->textEdit->setPlainText(m_strout);
}


//=============================================================
// AlgoHW 4
//=============================================================
void MainWindow::DFSDigOne(int i)
{
    m_been_there[i] = true;

    // who cares in loop one
    // m_leader[i] = m_s;

    VInt& adjs = graph_rev[i];
    for (VIntIt ita = adjs.begin(); ita != adjs.end(); ++ita)
    {
        int j = (*ita);
        if (!m_been_there[j])
            DFSDigOne(j);
    }
    m_finish[i] = m_t;
    m_nodes_for_loop2[m_t] = i;
    ++m_t;

    // LOOP ONE sanity check, LOOKS GOOD.
    // m_strout += QString("%1:%2\n").arg(i + 1).arg(m_t);
}
void MainWindow::DFSDigTwo(int i)
{
    ++m_css_count;

    m_been_there[i] = true;
    m_leader[i] = m_s;
    VInt& adjs = graph[i];
    for (VIntIt ita = adjs.begin(); ita != adjs.end(); ++ita)
    {
        int j = (*ita);
        if (!m_been_there[j])
            DFSDigTwo(j);
    }

    // who cares in loop two
    // ++m_t;
    // m_finish[i] = m_t;

}
void MainWindow::save_css()
{
    m_v_css.push_back(m_css_count);
}


//=============================================================
// Algo Homework 1 recursive call
//=============================================================
void MainWindow::sort_and_count_inversions(std::vector<int>& vi)
{

    // Base cases
    if (vi.size() < 2) return;
    if (vi.size() < 3)
    {
        if (vi[0]>vi[1])
        {
            int temp = vi[0];
            vi[0] = vi[1];
            vi[1] = temp;

            ++m_n_total_inversions;
        }
        return;
    }

    std::vector<int> vil = vi; vil.erase(vil.begin() + vil.size()/2, vil.end());
    // ui->textEdit_2->setPlainText(ui->textEdit_2->toPlainText() + QString("\nleft %1 ").arg(vil.size()));
    sort_and_count_inversions(vil);

    std::vector<int> vir = vi; vir.erase(vir.begin(),vir.begin() + vir.size()/2);
    // ui->textEdit_2->setPlainText(ui->textEdit_2->toPlainText() + QString("\nright %1").arg(vir.size()));
    sort_and_count_inversions(vir);

    // Now we have the subarrays, step through them and both sort and count.
    // We copy into a new array, and then replace our own values when done.
    std::vector<int> vd;
    vd.resize(vi.size());
    std::vector<int>::iterator itl = vil.begin();
    std::vector<int>::iterator itr = vir.begin();
    for (int k = 0; k < vi.size(); ++k)
    {
        if (itl == vil.end())
        {
            // Just copy the rest of the right.
            while (itr != vir.end())
            {
                vd[k] = (*itr);
                ++itr;
                ++k;
            }

        } else if (itr == vir.end())
        {
            // Just copy the rest of the left.
            while (itl != vil.end())
            {
                vd[k] = (*itl);
                ++itl;
                ++k;
            }

        } else
        {
            if ((*itl)<(*itr))
            {
                vd[k] = (*itl);
                ++itl;

            } else
            {
                // We have inversions,equal to the number of elements left in the left array, heh.
                // NOTE: this should always be positive, since we already handled end-of-subvector, above.
                // ui->textEdit_2->setPlainText(ui->textEdit_2->toPlainText() + QString("\n%1").arg((vil.end() - itl) - 1));
                m_n_total_inversions += (vil.end() - itl);

                vd[k] = (*itr);
                ++itr;
            }
        }
    }

    // COPY OUT THE RESULTS, idjit.
    vi = vd;
}


//=============================================================
// Algo Homework 2 recursive call
//
// count comparisons in QuickSort that uses three different versions of pivoting
//      a) first item
//      b) last element
//              just before the main Partition subroutine, you should exchange the pivot element (i.e., the last element) with the first element
//      c) median-of-three
//
// use the algorithm pseudocode from class
//      partition (A,l,r)  where input = A[l...r]
//          p = A[l]
//          i = l+1
//          for j=l+1 to r
//              if A[j] < p
//                  swap A[j] and A[i]
//              ++i
//          swap A[l] and A[i-1]
//
//=============================================================
void MainWindow::sort_and_count_comparisons(VInt& vi)
{
    // Base case
    if (vi.size() < 2) return;

    m_n_total_comparisons += vi.size() - 1;

    VIntIt it_pivot = get_pivot(vi);

    // WHATEVER the pivot is, make sure it's swapped into the first slot.
    if (it_pivot != vi.begin())
    {
        int n_swap = *vi.begin();
        *vi.begin() = *it_pivot;
        *it_pivot = n_swap;
    }

    // Here's how it should go for TenToOne.txt
    // [ 10 9 8 7 6 5 4 3 2 1 ]
    // 10 will be our pivot for first problem
    // i dictates middle point of our partitioned part
    //      it's the "next available slot" for a "low" number
    // j dictates the unpartitioned part - we'll point it at the next element of the unpartitioned part

    // #1 begin
    // [ 10  9  8 7 6 5 4 3 2 1 ]
    //    p  ij

    // #1 compare: 9 is less than 10

    // #1 swap the item after i with the item pointed to by j
    // in this case they are the same item so nothing happens
    // then we increment i
    // [ 10  9  8 7 6 5 4 3 2 1 ]
    //    p  j  i

    // #2 begin
    // [ 10  9  8  7 6 5 4 3 2 1 ]
    //    p     ij

    // #2 compare: 8 is less than 10
    // swap - again, with itself
    // then we increment i
    // [ 10  9  8  7 6 5 4 3 2 1 ]
    //    p     ij

    VIntIt p = vi.begin();
    VIntIt i = p; ++i;
    for (VIntIt j = i; j < vi.end(); ++j)
    {
        if (*j < *p)
        {
            int n_swap = *j;
            *j = *i;
            *i = n_swap;

            ++i;
        }
    }

    --i;
    int n_swap = *vi.begin();
    *vi.begin() = *i;
    *i = n_swap;

    // now recurse
    // we need two arrays, all items before "i", and all items after "i"
    // i'm bailing on doing an in-place vector parameter pass just to get done faster, sorry, i got Pho to make...
    VInt vil;
    if (i - vi.begin() > 0)
    {
        vil = vi;
        vil.erase(vil.begin() + (i - vi.begin()), vil.end());
        sort_and_count_comparisons(vil);
    }

    VInt vir;
    if (vi.end() - i > 1)
    {
        vir = vi;
        vir.erase(vir.begin(),vir.begin() + (i - vi.begin()) + 1);
        sort_and_count_comparisons(vir);
    }

    // Rebuild the result
    vi = vil;
    vi.push_back(*i);
    vi.insert(vi.end(), vir.begin(), vir.end());
}


VIntIt MainWindow::get_pivot(VInt& vi)
{
    // Comparisons = 162085
    // return vi.begin();

    // Comparisons = 164123
    // VIntIt pivot = vi.end();
    // --pivot;
    // return pivot;

    // Comparisons = 138382
    // Here we need to select the "median-of-three".
    // get the first, middle and last, and pick the median
    // to get middle of an even set, we were told to ROUND DOWN!
    VIntIt first = vi.begin();
    VIntIt mid = vi.end() - (int)(vi.size()/2) - 1;
    VIntIt last = vi.end() - 1;

    if ((*first  < *mid  ) && (*mid  < *last )) return mid;
    if ((*last   < *mid  ) && (*mid  < *first)) return mid;
    if ((*first  < *last ) && (*last < *mid  )) return last;
    if ((*mid    < *last ) && (*last < *first)) return last;
    return first;
}


MainWindow::~MainWindow()
{
    delete ui;
}
