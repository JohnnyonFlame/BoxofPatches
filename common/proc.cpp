#include "proc.h"

#include <string>
#include <vector>
#include <fstream>
#include <sstream>
#include <sys/mman.h>

proc_maps_t::proc_maps_t(std::istream &stream)
{
    char ignored;
    std::string perms_str;
    stream >> std::hex >> start;
    stream >> ignored;
    stream >> std::hex >> end;
    stream >> perms_str;
    stream >> offset;
    stream >> dev;
    stream >> inode;
    stream >> path;

    perms = 0;
    if (perms_str.size() == 4 && perms_str[3] == 'p') {
        perms |= perms_str[0] == 'r' ? PROT_READ : 0;
        perms |= perms_str[1] == 'w' ? PROT_WRITE : 0;
        perms |= perms_str[2] == 'x' ? PROT_EXEC : 0;
    }
}

std::vector<proc_maps_t> get_proc_self_maps(const char *path_substr = "")
{
    std::vector<proc_maps_t> maps = {};
    if (auto proc_self_maps = std::ifstream("/proc/self/maps")) {
        for (std::string line; std::getline(proc_self_maps, line); ) {
            auto stream = std::stringstream(line);
            auto proc = proc_maps_t(stream);

            if (proc.path.find(path_substr) != std::string::npos)
                maps.push_back(proc);
        }
    }

    return maps;
}