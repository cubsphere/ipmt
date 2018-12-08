#include <getopt.h>
#include <string.h>
#include <iostream>
#include <fstream>
#include <locale>
#include <suffix-array.hpp>

using namespace std;

bool help_printed = false;
const char *helpful_string = "after reading this helpful string, you learn how to use this program properly.\n";
const int STRING_SIZE_LESS = 4096;

void print_occs(vector<int> *occ, char *txt, int n);

void print_help()
{
    if (!help_printed)
    {
        cout << helpful_string;
        help_printed = true;
    }
}

int main(int argc, char **argv)
{
    bool count_mode = false;
    bool no_compression = false;

    char *mode;
    char *pattern_path;
    char *pattern;
    bool use_pattern_path = false;
    bool pattern_defined = false;

    char *text_path;

    int c;
    int option_index = 0;
    static struct option long_options[] = {
        {"pattern", required_argument, 0, 'p'},
        {"count", no_argument, 0, 'c'},
        {"no-compression", no_argument, 0, 'n'},
        {"help", no_argument, 0, 'h'}};

    bool stop = false;

    while (1)
    {
        c = getopt_long(argc, argv, "p:cnh", long_options, &option_index);

        if (c == -1 || stop)
            break;

        switch (c)
        {
        case 'p':
            pattern_defined = true;
            use_pattern_path = true;
            pattern_path = optarg;

        case 'c':
            count_mode = true;
            break;

        case 'h':
            print_help();
            break;

        case 'n':
            no_compression = true;
            break;

        case '?':
        default:
            return 1;
        }
    }

    mode = argv[optind];
    ++optind;
    if (strcmp(mode, "index") == 0)
    {
        if (count_mode && pattern_defined)
            cout << "pattern and count mode are not used for indexing\n";
        else if (count_mode)
            cout << "count mode is not used for indexing\n";
        else if (pattern_defined)
            cout << "pattern is not used for indexing\n";

        if (optind == argc)
        {
            cout << "text file not specified\n";
            return 2;
        }
        text_path = argv[optind];
        ++optind;
    }
    else if (strcmp(mode, "search") == 0)
    {
        if (!pattern_defined)
        {
            if (optind == argc)
            {
                cout << "pattern not specified\n";
                return 3;
            }
            pattern = argv[optind];
            ++optind;
        }

        if (optind == argc)
        {
            cout << "text file not specified\n";
            return 4;
        }
        text_path = argv[optind];
        ++optind;
    }
    else
    {
        print_help();
        return 5;
    }

    ifstream text_file(text_path, ios::ate);
    if (!text_file.is_open())
    {
        cout << "could not open text file " << text_path << "\n";
        return 6;
    }

    char *text;
    int *sa_info;
    int textlen;
    int fullsize;
    if (strcmp(mode, "search") == 0)
    {
        if (no_compression | !no_compression)
        {
            fullsize = (int)(text_file.tellg()) - 1;
            textlen = fullsize / (3 * sizeof(int) / sizeof(char) + 1);
            text_file.seekg(0);

            text = new char[fullsize];
            sa_info = (int *)(text + textlen);
            text_file.read(text, fullsize);
            text_file.close();

        }
        else
        {
        }

        vector<int> occ;
        if (!use_pattern_path)
        {
            search(text, textlen, pattern, strlen(pattern), sa_info, sa_info + textlen, sa_info + textlen * 2, &occ);
            print_occs(&occ, text, textlen);
        }
        else
        {
            char pat[STRING_SIZE_LESS];
            ifstream pattern_file;
            pattern_file.open(pattern_path);
            if (!pattern_file.is_open())
            {
                cout << "could not open pattern file " << pattern_path << "\n";
                return 7;
            }
            while (!pattern_file.eof())
            {
                pattern_file.getline(pat, STRING_SIZE_LESS);
                search(text, textlen, pat, pattern_file.gcount() - 1, sa_info, sa_info + textlen, sa_info + textlen * 2, &occ);
                printf(">> occurences for %s:\n", pat);
                print_occs(&occ, text, textlen);
                printf("\n");
                occ.erase(occ.begin(), occ.end());
            }
            pattern_file.close();
        }
    }
    else
    {
        if (no_compression | !no_compression)
        {
            textlen = (int)(text_file.tellg()) - 1;
            fullsize = textlen + (textlen * 3) * sizeof(int) / sizeof(char);
            text_file.seekg(0);

            text = new char[fullsize];
            sa_info = (int *)(text + textlen);
            text_file.read(text, textlen);
            text_file.close();
        }
        else
        {
        }

        construct(text, textlen, sa_info, sa_info + textlen, sa_info + textlen * 2);

        char *text_dest = new char[strlen(text_path) + 4];
        strcpy(text_dest, text_path);
        strcat(text_dest, ".idx");
        ofstream text_file_dest(text_dest, ios_base::trunc);
        text_file_dest.write(text, fullsize);
        text_file_dest.close();
        delete[] text_dest;
    }
    delete[] text;

    return 0;
}

void print_occs(vector<int> *occ, char *txt, int n)
{
    int pos = 0;
    string s = string(txt);
    while (pos < occ->size())
    {
        int hind = s.find_last_of('\n', occ->at(pos)) + 1;
        int fore = s.find('\n', occ->at(pos));

        if (hind == -1)
            hind = 0;
        if (fore == -1)
            fore = n;

        printf("%.*s\n", fore - hind, txt + hind);
        while (pos < occ->size() && occ->at(pos) < fore)
            ++pos;
    }
}