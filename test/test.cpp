#include <iostream>
#include <fstream>
#include <ctime>
#include <string>

using namespace std;

string randword(int len)
{
    static const char alphanum[] =
        "0123456789"
        "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
        "abcdefghijklmnopqrstuvwxyz";

    string s;
    for (int i = 0; i < len; ++i)
    {
        s.push_back(alphanum[rand() / (RAND_MAX / (62) + 1)]);
    }
    return s;
}

double time_ms(string s)
{
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

const int fileslen = 1;
const string files[] = {"test2"};

const int wordsizeslen = 9;
const int wordsizes[] = {1, 2, 4, 8, 16, 32, 64, 128, 255};
const int repeat = 20;

int main()
{
    long uncompressed_sizes[fileslen];
    long compressed_sizes[fileslen];
    double index_times[fileslen];
    double search_times[fileslen][wordsizeslen];

    long l = CLOCKS_PER_SEC;

    for (int nnb = 0; nnb < repeat; ++nnb)
    {
        for (int k = 0; k < fileslen; ++k)
        {
            index_times[k] += time_ms("./ipmt index " + files[k]);
            for (int i = 0; i < wordsizeslen; ++i)
            {
                search_times[k][i] += time_ms("./ipmt search -c " + randword(wordsizes[i]) + " " + files[k] + ".idx");
            }
        }
    }

    for (int nnb = 0; nnb < repeat; ++nnb)
    {
        for (int k = 0; k < fileslen; ++k)
        {
            compressed_sizes[k] = filesize(files[k]);
            system(("./ipmt index --no-compression " + files[k]).c_str());
            uncompressed_sizes[k] = filesize(files[k]);
        }
    }

    cout.precision(5);
    cout.setf(ios::fixed, ios::floatfield);
    cout << "all timing results averaged over " << repeat << " random runs, all searches using -c\n";
    for (int k = 0; k < fileslen; ++k)
    {
        index_times[k] /= repeat;
        long comp_eff = (long) (100 * (double)compressed_sizes[k] / (double)uncompressed_sizes[k]);
        cout << ">> file " << files[k] << '\n';
        cout << "compressed from " << uncompressed_sizes[k] << " bytes to " << compressed_sizes[k] << " bytes (" << comp_eff << "\% of original size)\n";
        cout << "indexed in: " << index_times[k] << " ms\n";
        for (int i = 0; i < wordsizeslen; ++i)
        {
            search_times[k][i] /= repeat;
            cout << wordsizes[i] << "-length word searched for in: " << search_times[k][i]  << " ms\n";
        }
        cout << '\n';
    }

    return 0;
}