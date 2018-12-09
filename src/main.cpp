#include <getopt.h>
#include <string.h>
#include <iostream>
#include <fstream>
#include <locale>
#include <string_view>
#include <suffix-array.hpp>
#include <lz77.hpp>

using namespace std;

bool help_printed = false;
static const int STRING_SIZE_LESS = 4096;

static const char *helpful_string = "ipmt index [options] textfile\noptions:\n-n, --no-compression: if set, do not compress output file\n--ll: set ll value for compression algorithm\n--ls: set ls value for compression algorithm\n-h, --help: display this message\n\nipmt search [options] pattern textfile\noptions:\n-p, --pattern patternfile: search for all patterns in patternfile. patterns must be separated by a line break. if this option is used, then the program's arguments are instead read as:\nipmt search [options] textfile\n-n, --no-compression: if set, do not decompress input file (i.e. assume it is not compressed)\n--ll: set ll value for decompression algorithm\n--ls: set ls value for decompression algorithm\n-c, --count: if set, display only the amount of occurences of each pattern in the text\n-h, --help: display this message\n";

static const char alphabet[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnoprqstuvwxyz1234567890 .,;:<>)([]{}~^/?\\!@#$\%\'\"&*_+-=|\r\n\t";
static const int ablen = sizeof(alphabet);

void print_occs(vector<int> *occ, char *txt, int n, bool count_mode);

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
    int ll = 4;
    int ls = 5;

    int c;
    int option_index = 0;
    static struct option long_options[] = {
        {"pattern", required_argument, 0, 'p'},
        {"ll", required_argument, 0, 'l'},
        {"ls", required_argument, 0, 's'},
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
            break;

        case 'l':
            ll = atoi(optarg);
            break;

        case 's':
            ls = atoi(optarg);
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
    if(optind >= argc)
    {
        print_help();
        return -1;
    }

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

    char *text;
    int *sa_info;
    int textlen;
    int fullsize;
    if (strcmp(mode, "search") == 0)
    {
        ifstream text_file(text_path, ios::binary | ios::ate);
        if (!text_file.good())
        {
            cout << "could not open text file " << text_path << "\n";
            return 6;
        }

        if (no_compression)
        {
            fullsize = text_file.tellg();
            textlen = fullsize / (1 + 3 * sizeof(int) / sizeof(char));
            text_file.seekg(0);
            text = new char[fullsize];
            text_file.read(text, fullsize);
            text_file.close();
        }
        else
        {
            int filelen = text_file.tellg();
            text_file.seekg(0);
            vector<char> input;
            input.resize(filelen);
            text_file.read(&input[0], filelen);
            vector<char> *vec = lz77_decode(&input, ll, ls, alphabet, ablen);
            text = vec->data();
            fullsize = vec->size();
            textlen = fullsize / (1 + 3 * sizeof(int) / sizeof(char));
            text_file.close();
        }
        sa_info = (int *)(text + textlen);

        vector<int> occ;
        if (!use_pattern_path)
        {
            search(text, textlen, pattern, strlen(pattern), sa_info, sa_info + textlen, sa_info + textlen * 2, &occ);
            print_occs(&occ, text, textlen, count_mode);
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
                print_occs(&occ, text, textlen, count_mode);
                printf("\n");
                occ.erase(occ.begin(), occ.end());
            }
            pattern_file.close();
        }
    }
    else
    {
        ifstream text_file(text_path, ios::ate | ios::binary);
        if (!text_file.good())
        {
            cout << "could not open text file " << text_path << "\n";
            return 6;
        }

        textlen = (int)(text_file.tellg());
        fullsize = textlen + (textlen * 3) * sizeof(int) / sizeof(char);
        text_file.seekg(0);

        text = new char[fullsize];
        sa_info = (int *)(text + textlen);
        text_file.read(text, textlen);
        text_file.close();

        construct(text, textlen, sa_info, sa_info + textlen, sa_info + textlen * 2);

        char *text_dest = new char[strlen(text_path) + 4];
        strcpy(text_dest, text_path);
        strcat(text_dest, ".idx");
        ofstream text_file_dest(text_dest, ios_base::trunc | ios_base::binary);
        if (no_compression)
        {
            text_file_dest.write(text, fullsize);
            text_file_dest.close();
        }
        else
        {
            vector<char> input(text, text + fullsize);
            vector<char> *output = lz77_encode(text, fullsize, ll, ls, alphabet, ablen);
            text_file_dest.write(output->data(), output->size());
            text_file_dest.close();
        }
    }
    delete[] text;

    return 0;
}

void print_occs(vector<int> *occ, char *txt, int n, bool count_mode)
{
    int pos = 0;
    string_view s(txt);
    if (count_mode)
        cout << occ->size() << '\n';
    else
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