#include <getopt.h>
#include <string.h>
#include <iostream>
#include <fstream>
#include <locale>
#include <string>
#include <suffix-array.hpp>
#include <suffix-array-lcp.hpp>
#include <lz77.hpp>
#include <lz78.hpp>

using namespace std;

bool help_printed = false;
static const int STRING_SIZE_LESS = 4096;

static const char *helpful_string =
    "ipmt index [options] textfile\n"
    "options:\n"
    "-n, --no-compression: if set, do not compress output file\n"
    "-l, --lcp: use LCP - drastically increases RAM usage\n"
    "-h, --help: display this message\n"
    "ipmt search [options] pattern textfile\n"
    "options:\n"
    "-p, --pattern patternfile: search for all patterns in patternfile. patterns must be separated by a line break. if this option is used, then the program's arguments are instead read as:\n"
    "ipmt search [options] textfile\n"
    "-c, --count: if set, display only the amount of occurences of each pattern in the text\n"
    "-h, --help: display this message\n";

static const char alphabet[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnoprqstuvwxyz1234567890 .,;:<>)([]{}~^/?\\!@#$\%\'\"&*_+-=|\r\n\t";
static const int ablen = sizeof(alphabet);

static const int ll = 4;
static const int ls = 6;

static const char flag_uses_lcp = 1;
static const char flag_uncompressed = 2;
static const char flag_uses_lz78 = 4;

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
    bool use_lcp = false;
    bool use_lz78 = false;
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
        {"lcp", no_argument, 0, 'l'},
        {"lz78", no_argument, 0, '8'},
        {"count", no_argument, 0, 'c'},
        {"no-compression", no_argument, 0, 'n'},
        {"help", no_argument, 0, 'h'}};

    bool stop = false;

    while (1)
    {
        c = getopt_long(argc, argv, "p:clnh", long_options, &option_index);

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
            use_lcp = true;
            break;

        case '8':
            use_lz78 = true;
            break;

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

        int filelen = text_file.tellg();
        text_file.seekg(0);
        char flags;
        text_file.read(&flags, 1);
        no_compression = flags & flag_uncompressed;
        use_lcp = flags & flag_uses_lcp;
        use_lz78 = flags & flag_uses_lz78;

        if (no_compression)
        {
            fullsize = filelen - 1;
            text = new char[fullsize];
            text_file.read(text, fullsize);
            text_file.close();
        }
        else
        {
            if (use_lz78)
            {
                deque<char> input(filelen - 1, '\0');
                text_file.read(&input[0], filelen - 1);
                deque<char> *dek = decode(&input, alphabet, ablen);
                text = &(*dek)[0];
                fullsize = dek->size();
                text_file.close();
            }
            else
            {
                vector<char> input;
                input.resize(filelen - 1);
                text_file.read(&input[0], filelen - 1);
                vector<char> *vec = lz77_decode(&input, ll, ls, alphabet, ablen);
                text = vec->data();
                fullsize = vec->size();
                text_file.close();
            }
        }
        if (use_lcp)
            textlen = fullsize / (1 + 3 * sizeof(int) / sizeof(char));
        else
            textlen = fullsize / (1 + 1 * sizeof(int) / sizeof(char));
        sa_info = (int *)(text + textlen);

        vector<int> occ;
        if (!use_pattern_path)
        {
            if (use_lcp)
                search(text, textlen, pattern, strlen(pattern), sa_info, sa_info + textlen, sa_info + textlen * 2, &occ);
            else
                search(text, textlen, pattern, strlen(pattern), sa_info, &occ);
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
                if (use_lcp)
                    search(text, textlen, pat, pattern_file.gcount() - 1, sa_info, sa_info + textlen, sa_info + textlen * 2, &occ);
                else
                    search(text, textlen, pat, pattern_file.gcount() - 1, sa_info, &occ);

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
        text_file.seekg(0);

        if (use_lcp)
            fullsize = textlen + (textlen * 3) * sizeof(int) / sizeof(char);
        else
            fullsize = textlen + (textlen) * sizeof(int);

        text = new char[fullsize];
        sa_info = (int *)(text + textlen);
        text_file.read(text, textlen);
        text_file.close();

        if (use_lcp)
            construct(text, textlen, sa_info, sa_info + textlen, sa_info + textlen * 2);
        else
            construct(text, textlen, sa_info);

        char *text_dest = new char[strlen(text_path) + 5];
	int first_dot = idx_77(text_path, strlen(text_path), '.');
	if (first_dot == -1) first_dot = strlen(text_path);
	int fd;
	for (fd = 0; fd < first_dot; fd++) {
	  text_dest[fd] = text_path[fd];
	}
	text_dest[fd] = '\0';
        strcat(text_dest, ".idx");
        ofstream text_file_dest(text_dest, ios_base::trunc | ios_base::binary);

        char flags;
        if (use_lcp)
            flags |= flag_uses_lcp;
        if (no_compression)
            flags |= flag_uncompressed;
        if (use_lz78)
            flags |= flag_uses_lz78;

        text_file_dest.write(&flags, 1);

        if (no_compression)
        {
            text_file_dest.write(text, fullsize);
            text_file_dest.close();
        }
        else
        {
            if (use_lz78)
            {
                string input(text, fullsize);
                deque<char> *output = encode(input, alphabet, ablen);
                text_file_dest.write(&(*output)[0], output->size());
                text_file_dest.close();
            }
            else
            {

                vector<char> input(text, text + fullsize);
                vector<char> *output = lz77_encode(text, fullsize, ll, ls, alphabet, ablen);
                text_file_dest.write(&(*output)[0], output->size());
                text_file_dest.close();
            }
        }
    }
    delete[] text;

    return 0;
}

void print_occs(vector<int> *occ, char *txt, int n, bool count_mode)
{
    int pos = 0;
    string s(txt);
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
