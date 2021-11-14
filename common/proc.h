#ifndef __PROC_H__
#define __PROC_H__

#include <string>
#include <vector>
#include <istream>

struct proc_maps_t {
    uint64_t start;
    uint64_t end;
    int perms;
    uint64_t offset;
    std::string dev;
    int inode;
    std::string path;

    proc_maps_t(std::istream &stream);
};

extern std::vector<proc_maps_t> get_proc_self_maps(const char *path_substr);

#endif /* __PROC_H__ */