// -*- C++ -*-

#include <errno.h>
#include <exception>
#include <iostream>
#include <set>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

// ----------------------------------------------------------------------------
/// Display simple usage information
void usage()
{
    std::cout << "Usage: rmtree [-fc] dirs...\n";
} // usage

// ----------------------------------------------------------------------------
/// Display a full help message
void help()
{
    usage();
    std::cout <<
        "Remove directories and their content\n"
        "\n"
        "Options:\n"
        "-f\tforce (change permissions on removed entries, remove read only entries)\n"
        "-c\tremove only the content, keep the directory\n";
} // help

// ----------------------------------------------------------------------------
/// check if a path can be removed.
/// Must exists.  Don't remove anything from .. to /.
/// Remove . and mount point only if contentOnly.
/// path must be a directory if contentOnly.
bool isValidForRemoval(std::string const& path, bool contentOnly)
{
    bool result = true;
    static std::set<std::pair<dev_t, ino_t> > pathToRoot;
    static std::pair<dev_t, ino_t> cwd;
    if (pathToRoot.empty()) {
        struct stat buf;
        std::string cur = ".";
        if (lstat(cur.c_str(), &buf) != 0) {
            int error = errno;
            std::cerr << "Unable to stat " << cur << ": "
                      << strerror(error) << '\n';
            result = false;
        }
        cwd = std::make_pair(buf.st_dev, buf.st_ino);
        do {
            cur += "/..";
            if (lstat(cur.c_str(), &buf) != 0) {
                int error = errno;
                std::cerr << "Unable to stat " << cur << ": "
                          << strerror(error) << '\n';
                result = false;
            }
        } while (pathToRoot.insert(std::make_pair(buf.st_dev, buf.st_ino)).second);
    }
    struct stat buf;
    if (lstat(path.c_str(), &buf) != 0) {
        int error = errno;
        std::cerr << "Unable to stat " << path << ": " << strerror(error) << '\n';
        return false;
    }
    if (pathToRoot.find(std::make_pair(buf.st_dev, buf.st_ino)) != pathToRoot.end()) {
        std::cerr << "Can't remove " << path << " as it is a parent.\n";
        return false;
    }
    if (contentOnly) {
        if (!S_ISDIR(buf.st_mode)) {
            std::cerr << path << " is not a directory.\n";
            return false;
        }
    } else {
        if (S_ISDIR(buf.st_mode)) {
            std::pair<dev_t, ino_t> cur = std::make_pair(buf.st_dev, buf.st_ino);
            if (cur == cwd) {
                std::cerr << "Can't remove .\n";
                return false;
            }
            std::string parent(path+"/..");
            if (lstat(parent.c_str(), &buf) != 0) {
                int error = errno;
                std::cerr << "Unable to stat " << path << ": " << strerror(error) << '\n';
                return false;
            }
            if (buf.st_dev != cur.first) {
                std::cerr << "Can't remove " << path << " as it is a mount point\n";
                return false;
            }
        }
    }
    return true;
} // isValidForRemoval

// ----------------------------------------------------------------------------
/// The main
int main(int argc, char* argv[])
{
    int status = EXIT_SUCCESS;

    try {
        int c, errcnt = 0;
        bool force = false;
        bool contentOnly = false;
        
        std::locale::global(std::locale(""));
        
        while (c = getopt(argc, argv, "fch"), c != -1) {
            switch (c) {
            case 'h':
                help();
                throw 0;
            case 'f':
                force=true;
                break;
            case 'c':
                contentOnly=true;
                break;
            case '?':
                ++errcnt;
                break;
            default:
                std::cerr << "Unexpected result from getopts: " << c << '\n';
                ++errcnt;
            }
        }
        
        for (int i = optind; i < argc; ++i) {
            if (!isValidForRemoval(argv[i], contentOnly))
                ++errcnt;
        }
        if (optind == argc) {
            usage();
            ++errcnt;
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
} // main
