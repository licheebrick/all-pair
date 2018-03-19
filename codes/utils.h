//
// Created by li danyang on 2018/3/17.
//

#ifndef WARSHALL_NO_PATH_UTILS_H
#define WARSHALL_NO_PATH_UTILS_H

#include <bdd.h>
#include <cstdint>
#include <list>
#include <json/json.h>

#define PACKED __attribute__ ((__packed__))

using namespace std;

struct PACKED List_t {
        uint32_t size;
        uint64_t *list;
        bool shared;
};

extern bool show_detail;

bdd match2bdd(string match, int hdr_len);
List_t make_sorted_list_from_array (uint32_t count, uint64_t elems[]);
List_t val_to_list(const Json::Value &val);
void allsatPrintHandler(char* varset, int size);

#endif //WARSHALL_NO_PATH_UTILS_H
