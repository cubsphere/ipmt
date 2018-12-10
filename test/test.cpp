#include <iostream>
#include <fstream>
#include <ctime>
#include <string>

using namespace std;

string randword(int len)
{
    static const char alphanum[] =
        "0123456789 "
        "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
        "abcdefghijklmnopqrstuvwxyz";

    string s;
    s.push_back('"');
    for (int i = 0; i < len; ++i)
    {
        s.push_back(alphanum[rand() / (RAND_MAX / (sizeof(alphanum) - 1) + 1)]);
    }
    s.push_back('"');
    return s;
}

double time_ms(string s)
{
    cout << s << endl;
    clock_t tStart = clock();
    system(s.c_str());
    return (double)(clock() - tStart) / ((CLOCKS_PER_SEC) / 1000.0);
}

long filesize(string s)
{
    ifstream text_file(s, ios::ate);
    long ret = text_file.tellg();
    text_file.close();
    return ret;
}

const int fileslen = 5;
const string files[] = {"test1", "test2", "test3", "test4", "test5"};

const int wordsizeslen = 7;
const int wordsizes[] = {1, 2, 4, 8, 16, 32, 64};
const int repeat = 20;

int main()
{
    double index_times_7[fileslen];
    double search_times_7[fileslen][wordsizeslen];
    long sizes_7[fileslen];

    double index_times_8[fileslen];
    double search_times_8[fileslen][wordsizeslen];
    long sizes_8[fileslen];

    double index_times_n[fileslen];
    double search_times_n[fileslen][wordsizeslen];
    long sizes_n[fileslen];

    double index_times_l[fileslen];
    double search_times_l[fileslen][wordsizeslen];
    long sizes_l[fileslen];

    for (int k = 0; k < fileslen; ++k)
    {
        index_times_7[k] += time_ms("./ipmt index " + files[k]);
        sizes_7[k] = filesize(files[k] + ".idx");
        for (int nnb = 0; nnb < repeat; ++nnb)
        {
            for (int i = 0; i < wordsizeslen; ++i)
            {
                search_times_7[k][i] += time_ms("./ipmt search -c " + randword(wordsizes[i]) + " " + files[k] + ".idx");
            }
        }
    }

    for (int k = 0; k < fileslen; ++k)
    {
        index_times_n[k] += time_ms("./ipmt index -n " + files[k]);
        sizes_n[k] = filesize(files[k] + ".idx");
        for (int nnb = 0; nnb < repeat; ++nnb)
        {
            for (int i = 0; i < wordsizeslen; ++i)
            {
                search_times_n[k][i] += time_ms("./ipmt search -c " + randword(wordsizes[i]) + " " + files[k] + ".idx");
            }
        }
    }

    for (int k = 0; k < fileslen; ++k)
    {
        index_times_l[k] += time_ms("./ipmt index -n --lcp " + files[k]);
        sizes_l[k] = filesize(files[k] + ".idx");
        for (int nnb = 0; nnb < repeat; ++nnb)
        {
            for (int i = 0; i < wordsizeslen; ++i)
            {
                search_times_l[k][i] += time_ms("./ipmt search -c " + randword(wordsizes[i]) + " " + files[k] + ".idx");
            }
        }
    }

    cout.precision(5);
    cout.setf(ios::fixed, ios::floatfield);
    cout << "all timing results averaged over " << repeat << " random runs, all searches using -c\n";
    for (int k = 0; k < fileslen; ++k)
    {
        cout << ">> file " << files[k] << "\n";

        cout << "  >>lz-77: " << sizes_7[k] << " bytes\n";
        cout << "    indexed in: " << index_times_7[k] << " ms\n";
        for (int i = 0; i < wordsizeslen; ++i)
        {
            search_times_7[k][i] /= repeat;
            cout << "    " << wordsizes[i] << "-length word searched for in: " << search_times_7[k][i] << " ms\n";
        }
        cout << "  >>end of lz-77\n";

        cout << "  >>no compression: " << sizes_n[k] << " bytes\n";
        cout << "    indexed in: " << index_times_n[k] << " ms\n";
        for (int i = 0; i < wordsizeslen; ++i)
        {
            search_times_n[k][i] /= repeat;
            cout << "    " << wordsizes[i] << "-length word searched for in: " << search_times_n[k][i] << " ms\n";
        }
        cout << "  >>end of no compression\n";

        cout << "  >>no compression + lcp: " << sizes_l[k] << " bytes\n";
        cout << "    indexed in: " << index_times_l[k] << " ms\n";
        for (int i = 0; i < wordsizeslen; ++i)
        {
            search_times_l[k][i] /= repeat;
            cout << "    " << wordsizes[i] << "-length word searched for in: " << search_times_l[k][i] << " ms\n";
        }
        cout << "  >>end of no compression + lcp\n";
    }

    return 0;
}