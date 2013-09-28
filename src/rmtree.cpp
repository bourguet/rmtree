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


// ----------------------------------------------------------------------------
// Global variables
bool force = false;
bool contentOnly = false;
bool keepTree = false;
bool dryRun = false;
bool verbose = false;
bool traverseMountPoints = false;
bool checkOwner = true;
bool forceRecurse = false;
uid_t me;

// ----------------------------------------------------------------------------
/// unlink(2) wrapper taking dryRun and verbose into account 
int doUnlink(std::string const& path)
{
    if (dryRun || verbose) {
        std::cout << "Removing " << path << '\n';
    }
    if (!dryRun) {
        return unlink(path.c_str());
    }
    return 0;
}

// ----------------------------------------------------------------------------
/// rmdir(2) wrapper taking dryRun and verbose into account
int doRmdir(std::string const& path)
{
    if (dryRun || verbose) {
        std::cout << "Removing directory " << path << '\n';
    }
    if (!dryRun) {
        return rmdir(path.c_str());
    }
    return 0;
}

// ----------------------------------------------------------------------------
/// Display simple usage information
void usage()
{
    std::cout << "Usage: rmtree [-hvnctFROM] paths...\n";
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
        "-h\tthis help\n"
        "-v\tverbose, say what is done\n"
        "-n\tdon't do anything, just say what will be done\n"
        "-c\tremove only the content, keep the top directories\n"
        "-t\tremove only the content, keep the directory tree\n"
        "-F\tforce (change permissions on removed entries, remove read only entries)\n"
        "-R\trecurse in directories which one know beforehand won't be removed\n"
        "-O\tdon't check ownership\n"
        "-M\tremove in different file systems\n"
        ;
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
                std::cerr << "Can't remove the current working directory.\n";
                return false;
            }
            std::string parent(path+"/..");
            if (lstat(parent.c_str(), &buf) != 0) {
                int error = errno;
                std::cerr << "Unable to stat " << path << ": " << strerror(error) << '\n';
                return false;
            }
            if (buf.st_dev != cur.first) {
                std::cerr << "Can't remove " << path << " as it is a mount point.\n";
                return false;
            }
        }
    }
    return true;
} // isValidForRemoval

// ----------------------------------------------------------------------------
/// remove a directory entry.
bool removeEntry(std::string const& path, bool contentOnly,
                 bool topDir, dev_t parentDevice)
{
    bool result = true;
    struct stat buf;
    if (lstat(path.c_str(), &buf) != 0) {
        perror(path.c_str());
        return false;
    }
    if (!traverseMountPoints && !topDir && buf.st_dev != parentDevice) {
        std::cerr << "Can't remove " << path << ": is on a different filesystem\n";
        return false;
    }
    if (!S_ISDIR(buf.st_mode)) {
        if (checkOwner && buf.st_uid != me) {
            std::cerr << "Can't remove " << path << ": not owner\n";
            return false;
        } else if (force || S_ISLNK(buf.st_mode) || access(path.c_str(), W_OK) == 0) {
            if (doUnlink(path.c_str()) != 0) {
                int error = errno;
                std::cerr << "Can't remove " << path << ": " << strerror(error) << '\n';
                return false;
            } else {
                return true;
            }
        } else {
            int error = errno;
            std::cerr << "Can't remove " << path << ": " << strerror(error) << '\n';
            return false;
        }
    } else {
        if (checkOwner && buf.st_uid != me) {
            std::cerr << "Can't remove " << path << ": not owner\n";
            if (forceRecurse) {
                result = false;
            } else {
                return false;
            }
        } else if (access(path.c_str(), W_OK|X_OK|R_OK) != 0) {
            if (!force) {
                int error = errno;
                std::cerr << "Can't remove " << path << ": " << strerror(error) << '\n';
                if (forceRecurse)
                    result = false;
                else
                    return false;
           } else {
                if (verbose | dryRun) {
                    std::cerr << "Changing rights on " << path.c_str() << '\n';
                }
                if (dryRun) {
                    if (access(path.c_str(), X_OK|R_OK) != 0) {
                        std::cerr << "Lack of access rights prevent listing actions which would be done on "
                                  << path.c_str() << '\n';
                        return false;
                    }
                } else {
                    if (chmod(path.c_str(), S_IRUSR|S_IWUSR|S_IXUSR|buf.st_mode) != 0) {
                        int error = errno;
                        std::cerr << "Can't change rights on " << path << ": " << strerror(error) << '\n';
                        if (forceRecurse)
                            result = false;
                        else
                            return false;
                    }
                }
            }
        }
        DIR* dp = opendir(path.c_str());
        if (dp == NULL) {
            int error = errno;
            std::cerr << "Can't list the content of " << path << ": " << strerror(error) << '\n';
            return false;
        } else {
            while (struct dirent* ep = readdir(dp)) {
                if (strcmp(ep->d_name, ".") != 0 && strcmp(ep->d_name, "..") != 0)
                {
                    result &= removeEntry(path+"/"+ep->d_name, keepTree, false, buf.st_dev);
                }
            }
            closedir(dp);
            if (!contentOnly && (!checkOwner || buf.st_uid == me)) {
                if (doRmdir(path.c_str()) != 0) {
                    int error = errno;
                    std::cerr << "Can't remove dir " << path << ": " << strerror(error) << '\n';
                    return false;
                } else {
                    return result;
                }
            } else {
                return result;
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
        
        while (c = getopt(argc, argv, "hvnctFROM"), c != -1) {
            switch (c) {
            case 'h':
                help();
                throw 0;
            case 'v':
                verbose=true;
                break;
            case 'n':
                dryRun=true;
                break;
            case 'c':
                contentOnly=true;
                break;
            case 't':
                keepTree=true;
                break;
            case 'F':
                force=true;
                break;
            case 'M':
                traverseMountPoints=true;
                break;
            case 'R':
                forceRecurse=true;
                break;
            case 'O':
                checkOwner=false;
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
            if (!isValidForRemoval(argv[i], contentOnly||keepTree))
                ++errcnt;
        }
        if (optind == argc) {
            usage();
            ++errcnt;
        }

        if (errcnt > 0)
            throw 2;

        me = geteuid();
        int notRemoved = 0;
        
        for (int i = optind; i < argc; ++i) {
            if (!removeEntry(argv[i], contentOnly, true, 0))
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
