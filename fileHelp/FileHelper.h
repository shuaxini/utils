#ifndef __HDDL_FILE_HELPER__
#define __HDDL_FILE_HELPER__

#include <string>
#include <vector>
#include <boost/filesystem.hpp>

namespace FileHelper {
bool exist(const char* path);
bool exist(const std::string& path);

void* readFile(const char* filePath, size_t* length);
void* readFile(const std::string& filePath, size_t* length);

bool isFile(std::string filePath);
bool isDirectory(std::string filePath);
bool createDirectories(std::string directory);

bool isAbsolutePath(std::string filePath);

size_t getFileSize(std::string filePath);
size_t readFile(const char* filePath, size_t length, void* buffer);
size_t writeFile(const char* filePath, size_t length, void* buffer);

std::string getFileName(std::string filePath);
std::string getFileNameStem(std::string filePath);

std::string getTempDirectoryPath();
std::string getDirectoryOfExecuteFile();
std::vector<std::string> getFiles(std::string cate_dir);

bool changeFileDesOwner(int fd, const char* user, const char* group);
bool changeFileOwner(const char* file, const char* user, const char* group);
bool changeFileDesMode(int fd, int mode);
bool changeFileMode(const char* file, int mode);

template <typename T1, typename T2>
boost::filesystem::path _joinPath(T1 father, T2 son)
{
    boost::filesystem::path f(father);
    boost::filesystem::path s(son);
    return f / s;
}

template <typename T, typename... Ts>
boost::filesystem::path _joinPath(T p1, Ts... p2)
{
    return boost::filesystem::path(p1) / _joinPath(p2...);
}

template <typename T, typename... Ts>
std::string joinPath(T p1, Ts... p2)
{
    auto r = _joinPath(p1, p2...);

    return r.string();
}

bool updateAccessAttribute(int fd, std::string& group, std::string& user, int mode);
bool updateAccessAttribute(const char* file, std::string& group, std::string& user, int mode);
bool updateAccessAttribute(std::string& file, std::string& group, std::string& user, int mode);

char* getHome();
}
#endif
