/* Copyright (c) Hilmi Yildirim 2010,2011. Yilin Zheng 2018.

The software is provided on an as is basis for research purposes.
There is no additional support offered, nor are the author(s) 
or their institutions liable under any circumstances.

The modified client/server version is basis for testing.
There is no additional support offered, nor are the author(s) 
or their institutions liable under any circumstances.
 */

/*
Add necessary headers for creating server
*/
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/shm.h>
#include <stdio.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <fcntl.h>

#include "Graph.hpp"
#include "GraphUtil.hpp"
#include "Grail.hpp"
#include <sys/time.h>
#include <signal.h>
#include <cstring>
#include "exception_list.hpp"
#include "exception_list_incremental_base.hpp"
#include "exception_list_incremental_plus.hpp"

#include "utils.hpp"

/* Define port, queue size, buffer size*/
#define PORT 24816
#define QUEUE_SIZE 10
#define BUFFER_SIZE 102400

bool SKIPSCC = false;
bool BIDIRECTIONAL = false;
int LABELINGTYPE = 0;
bool UseExceptions = false;
bool UsePositiveCut = false;
bool POOL = false;
int POOLSIZE = 100;
int DIM = 5;
int query_num = 100000;
char* filename = NULL;
char* testfilename = NULL;
bool debug = false;
bool LEVEL_FILTER = false;
bool LEVEL_FILTER_I = false;

float labeling_time, query_time, query_timepart,exceptionlist_time;
int alg_type = 1;
std::ostringstream result;  // the result string which will be sent back to client through socket

// utility
static void handle(int sig);
static int usage(void);
static int parse_args(char *buffer);
static int grail(char *buffer, int sockfd);  // original `main`
static void reply_echo(int sockfd);  // echo the reply, which will call `grail` to search the result 


int main(int argc, char *argv[])
{
    int server_sockfd = ::socket(AF_INET, SOCK_STREAM, 0);

    struct sockaddr_in server_sockaddr;
    server_sockaddr.sin_family = AF_INET;
    server_sockaddr.sin_addr.s_addr = htonl(INADDR_ANY);
    server_sockaddr.sin_port = htons(PORT);

    if (::bind(server_sockfd, (struct sockaddr *)&server_sockaddr, sizeof(server_sockaddr)) == -1)
    {
        cerr << "Fail to bind port: " << PORT << "maybe the port is occupied by another process." << endl;
        exit(-1);
    }
    cout << "Bind success." << endl;

    if (::listen(server_sockfd, QUEUE_SIZE) == -1)
    {
        cerr << "Falied to listen to " << INADDR_ANY << ":" << PORT  << endl;
        exit(-1);
    }
    cout << "Listen at " << PORT << " success."<< endl;

    while(1)
    {
        signal(SIGINT, handle);
        struct sockaddr_in client_addr;
        socklen_t length = sizeof(client_addr);
        int connection = ::accept(server_sockfd, (struct sockaddr *)&client_addr, &length);
        if (connection < 0)
        {
            cerr << "Fail to establish connection." << endl;
            exit(1);
        }
        cout << "New client connetced." << endl;

        /* The most important part, which implementing the functionality of multiple client connecting 
           to a same server.*/
        pid_t child_id;
        if ((child_id = fork()) == 0) // fork a subprocess to handle the connection
        {
            cout << "Child process: " << getpid() << " created." << endl;
            close(server_sockfd);  // close listen in child process so that the child process will not act as its father
            reply_echo(connection);  // handle the connection
            exit(0);
        }
        else
        {
            cerr << "Client fails to connect server." << endl;
        }
    }

    cout << "closed." << endl;
    close(server_sockfd);
    return 0;
}

static void handle(int sig)
{
    if (sig == SIGINT)
    {
        cout << "\nServer closed." << endl;
        exit(0);
    }
    
    const char *alg_name;

    switch (alg_type)
    {
    case 1:
        alg_name = "GRAIL";
        break;
    case 2:
        alg_name = "GRAILLF";
        break;
    case 3:
        alg_name = "GRAILBI";
        break;
    case 6:
        alg_name = "GRAILBILF";
        break;
    case -1:
        alg_name = "GRAIL*";
        break;
    case -2:
        alg_name = "GRAIL*LF";
        break;
    case -3:
        alg_name = "GRAIL*BI";
        break;
    case -6:
        alg_name = "GRAIL*BILF";
        break;
    default:
        break;
    }

    cout << "COMPAR: " << alg_name << DIM << "\t" << labeling_time << "\t"
         << "TOut"
         << "\t" << endl;
    result << "COMPAR: " << alg_name << DIM << "\t" << labeling_time << "\t"
           << "TOut"
           << "\t" << endl;

    exit(1);
}

static int usage(void)
{
    cout << "\nUsage:\n"
            " grail [-h] <filename> -test <testfilename> [-dim <DIM>] [-skip] [-ex] [-ltype <labelingtype>] [-t <alg_type>]\n"
            "Description:\n"
            "	-h			Print the help message.\n"
            "  <filename> is the name of the input graph in gra format.\n"
            "	-test		Set the queryfile which contains a line of <source> <target> <reachability> for each query. \n"
            "	-dim 		Set the number of traversals to be performed. The default value is 5.\n"
            "	-ex 		Use exception lists method instead of pruning. Default is not using exception lists.\n"
            "	-skip		Skip the phase converting the graph into a dag. Use only when the input is already a DAG.\n"
            "	-t <alg_type>		alg_type can take 8 different values.  Default value is 1.\n"
            " \t\t\t 1  : Basic Search (used in the original VLDB'10 paper) \n"
            " \t\t\t 2  : Basic Search + Level Filter \n"
            " \t\t\t 3  : Bidirectional Search \n"
            " \t\t\t 6  : Bidirectional Search + Level Filter \n"
            " \t\t\t -1 : Positive Cut + Basic Search\n"
            " \t\t\t -2 : Positive Cut + Basic Search + Level Filter (usually provides the best query times) \n"
            " \t\t\t -3 : Positive Cut + Bidirectional Search \n"
            " \t\t\t -6 : Positive Cut + Bidirectional Search + Level Filter\n"
            "	-ltype <labeling_type>		labeling_type can take 6 different values.  Default value is 0.\n"
            " \t\t\t 0  : Completely randomized traversals.  \n"
            " \t\t\t 1  : Randomized Reverse Pairs Traversals (usually provides the best quality) \n"
            " \t\t\t 2  : Heuristicly Guided Traversal: Maximum Volume First \n"
            " \t\t\t 3  : Heuristicly Guided Traversal: Maximum of Minimum Interval First \n"
            " \t\t\t 4  : Heuristicly Guided Traversal: Maximum Adjusted Volume First \n"
            " \t\t\t 5  : Heuristicly Guided Traversal: Maximum of Adjusted Minimum Volume First \n"
         << endl;
    result << "\nUsage:\n"
              " grail [-h] <filename> -test <testfilename> [-dim <DIM>] [-skip] [-ex] [-ltype <labelingtype>] [-t <alg_type>]\n"
              "Description:\n"
              "	-h			Print the help message.\n"
              "  <filename> is the name of the input graph in gra format.\n"
              "	-test		Set the queryfile which contains a line of <source> <target> <reachability> for each query. \n"
              "	-dim 		Set the number of traversals to be performed. The default value is 5.\n"
              "	-ex 		Use exception lists method instead of pruning. Default is not using exception lists.\n"
              "	-skip		Skip the phase converting the graph into a dag. Use only when the input is already a DAG.\n"
              "	-t <alg_type>		alg_type can take 8 different values.  Default value is 1.\n"
              " \t\t\t 1  : Basic Search (used in the original VLDB'10 paper) \n"
              " \t\t\t 2  : Basic Search + Level Filter \n"
              " \t\t\t 3  : Bidirectional Search \n"
              " \t\t\t 6  : Bidirectional Search + Level Filter \n"
              " \t\t\t -1 : Positive Cut + Basic Search\n"
              " \t\t\t -2 : Positive Cut + Basic Search + Level Filter (usually provides the best query times) \n"
              " \t\t\t -3 : Positive Cut + Bidirectional Search \n"
              " \t\t\t -6 : Positive Cut + Bidirectional Search + Level Filter\n"
              "	-ltype <labeling_type>		labeling_type can take 6 different values.  Default value is 0.\n"
              " \t\t\t 0  : Completely randomized traversals.  \n"
              " \t\t\t 1  : Randomized Reverse Pairs Traversals (usually provides the best quality) \n"
              " \t\t\t 2  : Heuristicly Guided Traversal: Maximum Volume First \n"
              " \t\t\t 3  : Heuristicly Guided Traversal: Maximum of Minimum Interval First \n"
              " \t\t\t 4  : Heuristicly Guided Traversal: Maximum Adjusted Volume First \n"
              " \t\t\t 5  : Heuristicly Guided Traversal: Maximum of Adjusted Minimum Volume First \n"
           << endl;
    return 0;
}

static int parse_args(char *buffer)
{
    /*
        get argc and argv
    */
    int argc = 1;
    char **argv;
    int i = 0;
    // get argc
    while (buffer[i])
    {
        if (buffer[i] == ' ')
        {
            ++argc;
        }
        ++i;
    }
    // get argv
    char *rest = buffer;
    char *token = NULL;
    argv = (char **)malloc((sizeof(char *) * argc));
    i = 0;
    while ((token = strtok_r(rest, " ", &rest)) && i < argc)
    {
        argv[i] = (char *)malloc(sizeof(char) * (strlen(token) + 1));
        strcpy(argv[i], token);
        ++i;
    }

    if (argc == 1)
    {
        usage();
        return -1;
    }

    i = 1;

    while (i < argc)
    {
        if (strcmp("-h", argv[i]) == 0)
        {
            usage();
            return -1;
        }
        if (strcmp("-d", argv[i]) == 0)
        {
            i++;
            debug = true;
        }
        if (strcmp("-n", argv[i]) == 0)
        {
            i++;
            query_num = atoi(argv[i++]);
        }
        else if (strcmp("-dim", argv[i]) == 0)
        {
            i++;
            DIM = atoi(argv[i++]);
        }
        else if (strcmp("-lfi", argv[i]) == 0)
        {
            LEVEL_FILTER_I = true;
            i++;
        }
        else if (strcmp("-lf", argv[i]) == 0)
        {
            LEVEL_FILTER = true;
            alg_type *= 2;
            i++;
        }
        else if (strcmp("-test", argv[i]) == 0)
        {
            i++;
            testfilename = argv[i++];
        }
        else if (strcmp("-skip", argv[i]) == 0)
        {
            i++;
            SKIPSCC = true;
        }
        else if (strcmp("-ltype", argv[i]) == 0)
        {
            i++;
            LABELINGTYPE = atoi(argv[i++]);
        }
        else if (strcmp("-t", argv[i]) == 0)
        {
            i++;
            alg_type = atoi(argv[i++]);
        }
        else if (strcmp("-ex", argv[i]) == 0)
        {
            i++;
            UseExceptions = true;
        }
        else if (strcmp("-pp", argv[i]) == 0)
        {
            i++;
            UsePositiveCut = true;
            alg_type *= -1;
        }
        else if (strcmp("-bi", argv[i]) == 0)
        {
            i++;
            BIDIRECTIONAL = true;
            alg_type *= 3;
        }
        else if (strcmp("-pool", argv[i]) == 0)
        {
            i++;
            POOL = true;
            POOLSIZE = atoi(argv[i++]);
        }
        else
        {
            filename = argv[i++];
        }
    }
    if (!testfilename)
    {
        cout << "Please provide a test file : -test <testfilename> " << endl;
        //	exit(0);
        result << "Please provide a test file : -test <testfilename> " << endl;
    }
    return 0;
}

static int grail(char *buffer, int sockfd)
{
    signal(SIGALRM, handle);

    int ret = parse_args(buffer);
    if (ret == -1)
    {
        send(sockfd, result.str().c_str(), result.str().size(), 0);
        return -1;
    }
    /*
		Read Graph from the input file	
     */
    ifstream infile(filename);
    if (!infile)
    {
        cout << "Error: Cannot open " << filename << endl;
        result << "Error: Cannot open " << filename << endl;
        send(sockfd, result.str().c_str(), result.str().size(), 0);
        return -1;
    }

    Graph g(infile);
    infile.close();
    cout << "#vertex size:" << g.num_vertices() << "\t#edges size:" << g.num_edges() << endl;
    result << "#vertex size:" << g.num_vertices() << "\t#edges size:" << g.num_edges() << endl;

    int s, t, num_reachable = 0;
    int left = 0;
    int gsize = g.num_vertices();

    bool r;
    struct timeval after_time, before_time;

    int *sccmap;
    if (!SKIPSCC)
    {
        sccmap = new int[gsize]; // store pair of orignal vertex and corresponding vertex in merged graph
        vector<int> reverse_topo_sort;

        // merge strongly connectionected component
        cout << "merging strongly connectionected component..." << endl;
        result << "merging strongly connectionected component..." << endl;
        gettimeofday(&before_time, NULL);
        GraphUtil::mergeSCC(g, sccmap, reverse_topo_sort);
        gettimeofday(&after_time, NULL);
        query_time = (after_time.tv_sec - before_time.tv_sec) * 1000.0 +
                     (after_time.tv_usec - before_time.tv_usec) * 1.0 / 1000.0;
        cout << "merging time:" << query_time << " (ms)" << endl;
        result << "merging time:" << query_time << " (ms)" << endl;
        cout << "#DAG vertex size:" << g.num_vertices() << "\t#DAG edges size:" << g.num_edges() << endl;
        result << "#DAG vertex size:" << g.num_vertices() << "\t#DAG edges size:" << g.num_edges() << endl;
    }

    GraphUtil::topo_leveler(g);

    // prepare queries
    srand48(time(NULL));
    cout << "preparing " << query_num << " queries..." << endl;
    result << "preparing " << query_num << " queries..." << endl;
    vector<int> src;
    vector<int> trg;
    vector<int> labels;
    vector<int>::iterator sit, tit, lit;
    int success = 0, fail = 0;
    int label;
    if (testfilename == NULL)
    {
        while (left < query_num)
        {
            s = lrand48() % gsize;
            t = lrand48() % gsize;
            if (s == t)
                continue;
            src.push_back(s);
            trg.push_back(t);
            ++left;
        }
    }
    else
    {
        std::ifstream fstr(testfilename);
        while (!fstr.eof())
        {
            fstr >> s >> t >> label;
            src.push_back(s);
            trg.push_back(t);
            labels.push_back(label);
        }
    }

    cout << "queries are ready" << endl;
    result << "queries are ready" << endl;
    gettimeofday(&before_time, NULL);
    Grail grail(g, DIM, LABELINGTYPE, POOL, POOLSIZE);
    grail.set_level_filter(LEVEL_FILTER);
    gettimeofday(&after_time, NULL);

    labeling_time = (after_time.tv_sec - before_time.tv_sec) * 1000.0 +
                    (after_time.tv_usec - before_time.tv_usec) * 1.0 / 1000.0;
    cout << "#construction time:" << labeling_time << " (ms)" << endl;
    result << "#construction time:" << labeling_time << " (ms)" << endl;
    ExceptionList *el = NULL;
    exceptionlist_time = 0;
    if (UseExceptions)
    {
        gettimeofday(&before_time, NULL);
        el = new ExceptionListIncrementalPlus(g, DIM, 0);
        gettimeofday(&after_time, NULL);
        exceptionlist_time = (after_time.tv_sec - before_time.tv_sec) * 1000.0 +
                             (after_time.tv_usec - before_time.tv_usec) * 1.0 / 1000.0;
        cout << "exceptionlist time:" << exceptionlist_time << " (ms)" << endl;
        result << "exceptionlist time:" << exceptionlist_time << " (ms)" << endl;
    }

    // process queries
    cout << "process queries..." << endl;
    result << "process queries..." << endl;
    gettimeofday(&before_time, NULL);

    for (sit = src.begin(), tit = trg.begin(), lit = labels.begin(); sit != src.end(); ++sit, ++tit /*, ++lit*/)
    {
        if (!SKIPSCC)
        {
            s = sccmap[*sit];
            t = sccmap[*tit];
        }
        else
        {
            s = *sit;
            t = *tit;
        }

        switch (alg_type)
        {
        case 1:
            r = grail.reach(s, t, el);
            break; // Default
        case 2:
            r = grail.reach_lf(s, t, el);
            break;
        case 3:
            r = grail.bidirectionalReach(s, t, el);
            break;
        case 6:
            r = grail.bidirectionalReach_lf(s, t, el);
            break;

        case -1:
            r = grail.reachPP(s, t, el);
            break;
        case -2:
            r = grail.reachPP_lf(s, t, el);
            break; // Usually provide best performance
        case -3:
            r = grail.bidirectionalReachPP(s, t, el);
            break;
        case -6:
            r = grail.bidirectionalReachPP_lf(s, t, el);
            break;
        }
        if (r)
            num_reachable++;
    }

    gettimeofday(&after_time, NULL);
    query_time = (after_time.tv_sec - before_time.tv_sec) * 1000.0 +
                 (after_time.tv_usec - before_time.tv_usec) * 1.0 / 1000.0;
    cout << "#total query running time:" << query_time << " (ms)" << endl;
    result << "#total query running time:" << query_time << " (ms)" << endl;
    cout.setf(ios::fixed);
    result.setf(ios::fixed);
    cout.precision(2);
    result.precision(2);
    cout << "GRAIL REPORT " << endl;
    result << "GRAIL REPORT " << endl;
    cout << "filename = " << filename << "\t testfilename = " << (testfilename ? testfilename : "Random") << "\t DIM = " << DIM << endl;
    result << "filename = " << filename << "\t testfilename = " << (testfilename ? testfilename : "Random") << "\t DIM = " << DIM << endl;
    cout << "Labeling_time = " << labeling_time + exceptionlist_time << "\t Query_Time = " << query_time << "\t Index_Size = " << gsize * DIM * 2 << "\t Mem_Usage = " << print_mem_usage() << " MB" << endl;
    result << "Labeling_time = " << labeling_time + exceptionlist_time << "\t Query_Time = " << query_time << "\t Index_Size = " << gsize * DIM * 2 << "\t Mem_Usage = " << print_mem_usage() << " MB" << endl;

    const char *alg_name;

    switch (alg_type)
    {
    case 1:
        alg_name = "GRAIL";
        break;
    case 2:
        alg_name = "GRAILLF";
        LEVEL_FILTER = true;
        break;
    case 3:
        alg_name = "GRAILBI";
        break;
    case 6:
        alg_name = "GRAILBILF";
        LEVEL_FILTER = true;
        break;
    case -1:
        alg_name = "GRAIL*";
        break;
    case -2:
        alg_name = "GRAIL*LF";
        LEVEL_FILTER = true;
        break;
    case -3:
        alg_name = "GRAIL*BI";
        break;
    case -6:
        alg_name = "GRAIL*BILF";
        LEVEL_FILTER = true;
        break;
    }

    if (grail.PositiveCut == 0)
        grail.PositiveCut = 1;

    int totalIndexSize;
    if (alg_type < 0)
    {
        totalIndexSize = gsize * DIM * 3;
    }
    else
    {
        totalIndexSize = gsize * DIM * 2;
    }
    if (LEVEL_FILTER)
    {
        totalIndexSize += gsize;
    }
    if (UseExceptions)
    {
        totalIndexSize += el->Size();
    }

    cout << "COMPAR: "
         << alg_name << DIM << "\t"
         << labeling_time + exceptionlist_time << "\t"
         << query_time << "\t"
         << totalIndexSize << "\t"
         << print_mem_usage() << "\t"
         << grail.TotalCall << "\t"
         << grail.PositiveCut << "\t"
         << grail.NegativeCut << "\t"
         << num_reachable << "\t"
         << "AvgCut:" << (grail.TotalDepth / grail.PositiveCut) << endl;
    result << "COMPAR: "
           << alg_name << DIM << "\t"
           << labeling_time + exceptionlist_time << "\t"
           << query_time << "\t"
           << totalIndexSize << "\t"
           << print_mem_usage() << "\t"
           << grail.TotalCall << "\t"
           << grail.PositiveCut << "\t"
           << grail.NegativeCut << "\t"
           << num_reachable << "\t"
           << "AvgCut:" << (grail.TotalDepth / grail.PositiveCut) << endl;
    send(sockfd, result.str().c_str(), result.str().size(), 0);
    return 0;
}

/*
create connectionection
*/
static void reply_echo(int sockfd)
{
    char buffer[BUFFER_SIZE];
    pid_t pid = getpid();
    while (1)
    {
        memset(buffer, 0, sizeof(buffer));
        int len = recv(sockfd, buffer, sizeof(buffer), 0);
        if (strcmp(buffer, "exit\n") == 0)
        {
            cout << "child process: " << pid << " exited." << endl;
            break;
        }
        cout << "pid:" << pid << " receive." << endl;
        cout << "Querying..." << endl;
        grail(buffer, sockfd);  //buffer == argv, data will be reply in grail
    }
    close(sockfd);
}