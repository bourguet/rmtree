// -*- C++ -*-

#include <exception>
#include <iostream>
#include <stdlib.h>
#include <unistd.h>

/// Display simple usage information
void usage()
{
    std::cout << "Usage: rmtree\n";
}

/// Display a full help message
void help()
{
    usage();
}

/// The main
int main(int argc, char* argv[])
{
    int status = EXIT_SUCCESS;

    try {
        int c, errcnt = 0;

        std::locale::global(std::locale(""));
        
        while (c = getopt(argc, argv, "h"), c != -1) {
            switch (c) {
            case 'h':
                help();
                throw 0;
            case '?':
                errcnt++;
                break;
            default:
                std::cerr << "Unexpected result from getopts: " << c << '\n';
                errcnt++;
            }
        }

        if (optind < argc) {
        }

        if (errcnt > 0)
            throw EXIT_FAILURE;
        
    } catch (std::exception& e) {
        std::cerr << "Error: " << e.what() << '\n';
        status = EXIT_FAILURE;
    } catch (int s) {
        status = s;
    } catch (...) {
        std::cerr << "Unexpected exception.\n";
        status = EXIT_FAILURE;
    }

    return status;
}
