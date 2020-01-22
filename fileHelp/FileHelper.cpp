#include <iostream>
#include <string>
#include <string.h>
#include <grp.h>
#include <sys/stat.h>
#include <sys/resource.h>

#include "FileHelper.h"


namespace FileHelper {
std::string getEnvironmentVariable(const char* var)
{
    std::string ret;
    char* envvar = NULL;

    envvar = getenv(var);

    if (envvar != NULL) {
        ret = envvar;
    }

    return ret;
}

std::string getHddlInstallDir()
{
    return getEnvironmentVariable("HDDL_INSTALL_DIR");
}

std::string getAbsPath(const char* relpath)
{
    std::string r;
    char* rp = nullptr;

    rp = realpath(relpath, NULL);

    if (rp != nullptr) {
        r = std::string(rp);
        free(rp);
    }

    return r;
}

bool liftMaxOpenFileLimit()
{
    struct rlimit fdLimit;

    if (getrlimit(RLIMIT_NOFILE, &fdLimit) < 0) {
        return false;
    }

    fdLimit.rlim_cur = fdLimit.rlim_max;
    if (setrlimit(RLIMIT_NOFILE, &fdLimit) < 0) {
        return false;
    }

    return true;
}


bool exist(const char* path)
{
    struct stat buffer;
    return (stat (path, &buffer) == 0);
}

bool exist(const std::string& path)
{
    if (path.empty()) {
        return false;
    }

    return exist(path.c_str());
}

bool isFile(std::string filePath)
{
    try {
        return boost::filesystem::is_regular_file(filePath);
    } catch (boost::filesystem::filesystem_error& e) {
        printf("Error: isFile() failed: %s", e.what());
    }

    return false;
}

bool isDirectory(std::string filePath)
{
    try {
        return boost::filesystem::is_directory(filePath);
    } catch (boost::filesystem::filesystem_error& e) {
        printf("Error: checkIfDirectory() failed: %s", e.what());
    }

    return false;
}

bool createDirectories(std::string directory)
{
    try {
        return boost::filesystem::create_directories(directory.c_str());
    } catch (boost::filesystem::filesystem_error& e) {
        printf("Error: createDirectories() failed: %s", e.what());
    }

    return false;
}

bool isAbsolutePath(std::string filePath)
{
    try {
        boost::filesystem::path path(filePath);
        return path.is_absolute();
    } catch (boost::filesystem::filesystem_error& e) {
        printf("Error: IsAbsolutePath() failed: %s", e.what());
    }

    return false;
}

size_t readFile(const char* filePath, size_t length, void* buffer)
{
    if (!filePath || !length || !buffer) {
        return 0;
    }

    FILE* fp = fopen(filePath, "rb");
    if (!fp) {
        return 0;
    }

    auto readBytes = fread(buffer, 1, length, fp);
    if (readBytes != length) {
        std::cerr << "readFile() failed, readBytes=" << readBytes << " expectBytes=" << length << std::endl;
    }

    fclose(fp);

    return readBytes;
}

void* readFile(const char* filePath, size_t* length)
{
    FILE *fp = NULL;
    char *buffer = NULL;
    size_t bufSize;

    if (!(fp = fopen(filePath, "rb"))) {
        printf("failed to open file %s", filePath);
        return NULL;
    }

    fseek(fp, 0, SEEK_END);
    bufSize = ftell(fp);
    rewind(fp);

    buffer = (char*) malloc(bufSize);
    if (!buffer) {
        printf("failed to malloc buffer with %d bytes", *(int *)length);
        fclose(fp);
        return NULL;
    }

    if (fread(buffer, 1, bufSize, fp) != bufSize) {
        printf("failed to read file %s with %d bytes", filePath, *(int *)length);
        fclose(fp);
        free(buffer);
        return NULL;
    }

    fclose(fp);

    *length = bufSize;

    return buffer;
}

void* readFile(const std::string& filePath, size_t* length)
{
    if (filePath.empty()) {
        return nullptr;
    }

    return readFile(filePath.c_str(), length);
}

size_t writeFile(const char* filePath, size_t length, void* buffer)
{
    if (!filePath || !length || !buffer) {
        return 0;
    }

    FILE* fp = fopen(filePath, "wb");
    if (!fp) {
        return 0;
    }

    auto writeBytes = fwrite(buffer, 1, length, fp);
    if (writeBytes != length) {
        std::cerr << "writeFile() failed, writeBytes=" << writeBytes << " expectBytes=" << length << std::endl;
    }

    fclose(fp);

    return writeBytes;
}

std::string getTempDirectoryPath()
{
    auto tempPath = boost::filesystem::temp_directory_path();

    return boost::filesystem::canonical(tempPath).string();;
}

std::string getFileName(std::string filePath)
{
    return boost::filesystem::path(filePath).filename().string();
}

std::string getFileNameStem(std::string filePath)
{
    return boost::filesystem::path(filePath).stem().string();
}

size_t getFileSize(std::string filePath)
{
    FILE* fp = fopen(filePath.c_str(), "rb");
    if (!fp) {
        return 0;
    }

    fseek(fp, 0, SEEK_END);

    size_t fileSize = ftell(fp);

    fclose(fp);

    return fileSize;
}

std::vector<std::string> getFiles(std::string path)
{
    std::vector<std::string> files;
    if (!path.empty()) {
        namespace fs = boost::filesystem;
        if (fs::exists(path)) {
            if (fs::is_regular_file(path)) {
                files.push_back("\0");
            } else if (fs::is_directory(path)) {
                fs::path apk_path(path);
                fs::directory_iterator end;
                for (fs::directory_iterator i(apk_path); i != end; ++i) {
                    if (fs::is_regular_file(i->status())) {
                        std::string fname = i->path().filename().string();
                        if (fname == "." || fname == "..") {
                            continue;
                        }
                        files.push_back(fname);
                    }
                }
            }
        }
    }
    std::sort(files.begin(), files.end());
    return files;
}

bool updateAccessAttribute(int fd, std::string& group, std::string& user, int mode)
{
    bool ret = false;

    ret = changeFileDesOwner(fd, user.empty() ? NULL : user.c_str(),
        group.empty() ? NULL : group.c_str());
    if (ret == false) {
        printf("Error: Failed to set owner to fd: %d", fd);
        return false;
    }

    ret = changeFileDesMode(fd, mode);
    if (ret == false) {
        printf("Error: Failed to set mode to fd: %d", fd);
        return false;
    }

    printf("Set fd:%d owner: user-'%s', group-'%s', mode-'0%o'"
        , fd
        , user.empty() ? "no_change" : user.c_str()
        , group.empty() ? "no_change" : group.c_str()
        , mode);

    return true;
}

bool updateAccessAttribute(std::string& file, std::string& group, std::string& user, int mode)
{
    return updateAccessAttribute(file.c_str(), group, user, mode);
}

bool updateAccessAttribute(const char *file, std::string& group, std::string& user, int mode)
{
    bool ret = false;

    ret = changeFileOwner(file, user.empty() ? NULL : user.c_str(),
        group.empty() ? NULL : group.c_str());
    if (ret == false) {
        printf("Error: Failed to set owner to file: %s", file);
        return false;
    }

    ret = changeFileMode(file, mode);
    if (ret == false) {
        printf("Error: Failed to set owner and mode to file: %s", file);
        return false;
    }

    printf("Set file:%s owner: user-'%s', group-'%s', mode-'0%o'"
        , file
        , user.empty() ? "no_change" : user.c_str()
        , group.empty() ? "no_change" : group.c_str()
        , mode);

    return true;
}

std::string getDirectoryOfExecuteFile()
{
    char filePath[512];

    if (readlink("/proc/self/exe", filePath, 512) <= 0) {
        return std::string();
    }

    auto pCh = strrchr(filePath, '/');
    if (!pCh) {
        return std::string();
    }

    *pCh = '\0';

    return filePath;
}

bool changeFileDesOwner(int fd, const char* user, const char* group)
{
    gid_t gid = -1;
    uid_t uid = -1;

    if (fd <= 0) {
        errno = EINVAL;
        return false;
    }

    if (user != NULL) {
        // TODO:
    }

    if (group != NULL) {
        struct group* g = NULL;
        g = getgrnam(group);
        if (g == NULL) {
            printf("Error: Cannot get group id of group: %s\n", group);
            return false;
        }
        gid = g->gr_gid;

    }

    if (fchown(fd, uid, gid) < 0) {
        return false;
    }

    return true;
}

bool changeFileOwner(const char* file, const char* user, const char* group)
{
    gid_t gid = -1;
    uid_t uid = -1;

    if (file == NULL) {
        errno = EINVAL;
        return false;
    }

    if (!exist(file)) {
        printf("Error: file %s doesn't exist.", file);
        errno = EINVAL;
        return false;
    }

    if (user != NULL) {
        // TODO:
    }

    if (group != NULL) {
        struct group* g = NULL;
        g = getgrnam(group);
        if (g == NULL) {
            printf("Error: Cannot get group id of group: %s\n", group);
            return false;
        }
        gid = g->gr_gid;
    }

    if (chown(file, uid, gid) < 0) {
    	std::cout << "chown failed." << std::endl;
    	perror(file);
        return false;
    }

    return true;
}

bool changeFileDesMode(int fd, int mode)
{
    static_assert(sizeof (mode) >= sizeof (mode_t), "int is small than mode_t");
    if (fd <= 0) {
        errno = EINVAL;
        return false;
    }

    if (mode >= 0) {
        if (fchmod(fd, (mode_t)mode) < 0) {
            return false;
        }
    }

    return true;
}

bool changeFileMode(const char* file, int mode)
{
    static_assert(sizeof (mode) >= sizeof (mode_t), "int is small than mode_t");
    if (file == NULL) {
        errno = EINVAL;
        return false;
    }

    if (!exist(file)) {
        printf("Error: file %s doesn't exist.", file);
        errno = EINVAL;
        return false;
    }

    if (mode >= 0) {
        if (chmod(file, (mode_t)mode) < 0) {
            return false;
        }
    }

    return true;
}

char* getHome()
{
    return getenv("HOME");
}

}// End of namespace
