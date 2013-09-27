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
#include <stdio.h>
#include <dirent.h>

bool force = false;
bool contentOnly = false;
bool dryRun = false;
bool verbose = false;

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
        "-c\tremove only the content, keep the directory\n"
        "-v\tverbose\n"
        "-n\tdon't remove anything, say what'd be done\n";
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

int myUnlink(std::string const& path)
{
    if (dryRun || verbose) {
        std::cout << "Removing " << path << '\n';
    }
    if (!dryRun) {
        return unlink(path.c_str());
    }
    return 0;
}

int myRmdir(std::string const& path)
{
    if (dryRun || verbose) {
        std::cout << "Removing " << path << '\n';
    }
    if (!dryRun) {
        return rmdir(path.c_str());
    }
    return 0;
}

// ----------------------------------------------------------------------------
bool removeEntry(std::string const& path, bool contentOnly)
{
    struct stat buf;
    if (lstat(path.c_str(), &buf) != 0) {
        perror(path.c_str());
        return false;
    }
    if (!S_ISDIR(buf.st_mode)) {
        if (force || S_ISLNK(buf.st_mode) || access(path.c_str(), W_OK) == 0) {
            if (myUnlink(path.c_str()) != 0) {
                int error = errno;
                std::cerr << "Can't remove " << path << ":" << strerror(error) << '\n';
                return false;
            } else {
                return true;
            }
        } else {
            if (!force) {
                perror(path.c_str());
            }
            return false;
        }
    } else {
        if (access(path.c_str(), W_OK|X_OK|R_OK) != 0) {
            if (!force) {
                int error = errno;
                std::cerr << "Can't remove " << path << ":" << strerror(error) << '\n';
                return false;
            } else {
                if (verbose | dryRun) {
                    std::cerr << "Changing rights on " << path.c_str() << '\n';
                }
                if (dryRun) {
                    if (access(path.c_str(), X_OK|R_OK) != 0) {
                        int error = errno;
                        std::cerr << "Lack of access rights (" << strerror(error) << ") prevent listing actions which would be done on "
                                  << path.c_str() << '\n';
                    }
                    return false;
                } else {
                    if (chmod(path.c_str(), S_IRUSR|S_IWUSR|S_IXUSR|buf.st_mode) != 0) {
                        int error = errno;
                        std::cerr << "Can't change rights on " << path << ":" << strerror(error) << '\n';
                        return false;
                    }
                }
            }            
        }
        DIR* dp = opendir(path.c_str());
        if (dp == NULL) {
            int error = errno;
            std::cerr << "Can't remove " << path << ":" << strerror(error) << '\n';
            return false;
        } else {
            bool status = true;
            while (struct dirent* ep = readdir(dp)) {
                if (strcmp(ep->d_name, ".") != 0
                    && strcmp(ep->d_name, "..") != 0)
                {
                    status &= removeEntry(path+"/" +ep->d_name, false);
                }
            }
            closedir(dp);
            if (!contentOnly) {
                if (myRmdir(path.c_str()) != 0) {
                    int error = errno;
                    std::cerr << "Can't remove dir " << path << ": " << strerror(error) << '\n';
                    return false;
                } else {
                    return status;
                }
            } else {
                return status;
            }
        }
    }
} // removeEntry

// ----------------------------------------------------------------------------
/// The main
int main(int argc, char* argv[])
{
    int status = EXIT_SUCCESS;

    try {
        int c, errcnt = 0;
        
        std::locale::global(std::locale(""));
        
        while (c = getopt(argc, argv, "fchvn"), c != -1) {
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
            case 'v':
                verbose=true;
                break;
            case 'n':
                dryRun=true;
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
            throw 2;

        int notRemoved = 0;
        
        for (int i = optind; i < argc; ++i) {
            if (!removeEntry(argv[i], contentOnly))
                ++notRemoved;
        }

        if (notRemoved > 0)
            status = 1;
        else
            status = 0;
    } catch (std::exception& e) {
        std::cerr << "Error: " << e.what() << '\n';
        status = 3;
    } catch (int s) {
        status = s;
    } catch (...) {
        std::cerr << "Unexpected exception.\n";
        status = 3;
    }

    return status;
} // main
