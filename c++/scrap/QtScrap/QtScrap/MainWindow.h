#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <map>


// ------------------------------------------------------------------------------
// CONSTANTS GLOBALS STATICS
// ------------------------------------------------------------------------------

class intpair
{
public:
        int v1;
        int v2;
        bool operator<(const intpair& right) const
        {
            if (v1 == right.v1)
                return v2 < right.v2;
            return v1 < right.v1;
        }
};

typedef std::vector<int> VInt;
typedef std::vector<int>::iterator VIntIt;
typedef std::vector<intpair> VPair;
typedef std::vector<intpair>::iterator VPairIt;
typedef std::vector<int>::reverse_iterator VIntRit;
typedef std::vector< VInt > AdjacencyList;
typedef std::vector< VInt >::iterator AdjacencyListIt;
typedef std::vector< long long int > VBigInt;
typedef std::vector< VBigInt >::iterator VBigIntIt;

class WeightedAdjacencyListItem
{
    public:
        int n;
        VPair adjs;
};

typedef std::vector< WeightedAdjacencyListItem > WeightedAdjacencyList;
typedef std::vector< WeightedAdjacencyListItem >::iterator WeightedAdjacencyListIt;

typedef std::map< int, bool> HashBucketSet;
typedef HashBucketSet::iterator HashBucketSetIt;
// ------------------------------------------------------------------------------


namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT
    
public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();
    
private:
    Ui::MainWindow *ui;


    //=============================================================
    // Algo Homework 1
    //=============================================================
    void sort_and_count_inversions(std::vector<int> &vi);
    long long int m_n_total_inversions;


    //=============================================================
    // Algo Homework 2
    //=============================================================
    void sort_and_count_comparisons(VInt &vi);
    VIntIt get_pivot(VInt& vi);
    long long int m_n_total_comparisons;

    //=============================================================
    // Algo Homework 4
    //=============================================================
    void DFSDigOne(int i);
    void DFSDigTwo(int i);
    void save_css();
    AdjacencyList graph;
    AdjacencyList graph_rev;
    std::vector<bool> m_been_there;
    VInt m_leader;
    VInt m_finish;
    VInt m_nodes_for_loop2;
    VInt m_v_css;
    int m_t;
    int m_s;
    int m_css_count;

    //=============================================================
    // Algo Homework 5
    //=============================================================
    WeightedAdjacencyList wgraph;
    VInt m_X;                   // where we've been
    VInt m_A;                   // greedy for each vertex
    int m_n_vertexes_remaining;

    //=============================================================
    // Algo Homework 6
    //=============================================================
    HashBucketSet m_buckets;


    QString m_strout;



};

#endif // MAINWINDOW_H
