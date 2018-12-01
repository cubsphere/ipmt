#include <getopt.h>
#include <string.h>
#include <iostream>
#include <fstream>
#include <locale>

using namespace std;

bool help_printed = false;
const char *helpful_string = "after reading this helpful string, you learn how to use this program properly.\n";

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
        {"help", no_argument, 0, 'h'}};

    bool stop = false;

    while (1)
    {
        c = getopt_long(argc, argv, "p:ch", long_options, &option_index);

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

    long textlen = text_file.tellg();
    --textlen;
    text_file.seekg(0);
    char *text = new char[textlen];
    text_file.read(text, textlen);
    text_file.close();

    if (strcmp(mode, "search") == 0)
    {
        if (!use_pattern_path)
        {
        }
        else
        {
            int STRING_SIZE_LESS = 1024; //TODO
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
                pattern_file.gcount();
            }
            pattern_file.close();
        }
    }
    else
    {
    }

    text_file.close();

    return 0;
}
