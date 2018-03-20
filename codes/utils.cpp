//
// Created by li danyang on 2018/3/17.
//
#include <sstream>
#include "utils.h"

bdd match2bdd(string match, int hdr_len) {
    bdd result = bddtrue;
    int match_len = 8;
    if (hdr_len != 1) {
        match_len = 8 * hdr_len + hdr_len - 1;
    }
    int idx = 0;
    for (int i = 0; i < match_len; i++) {
        if (match[i] == 'x') {
            idx++;
            continue;
        } else {
            if (match[i] == '1') {
                result &= bdd_ithvar(idx);
                idx++;
            } else {
                if (match[i] == '0') {
                    result &= bdd_nithvar(idx);
                    idx++;
                }
            }
        }
    }
    return result;
}

int compare (const void * a, const void * b)
{
    return ( *(uint64_t*)a - *(uint64_t*)b );
}

List_t make_sorted_list_from_array (uint32_t count, uint64_t elems[]) {
    List_t result;
    result.size = count;
    result.shared = false;
    if (count > 0)
        result.list = (uint64_t *)malloc(count * sizeof(uint64_t));
    else
        result.list = NULL;
    for (uint32_t i = 0; i < count; i++) {
        result.list[i] = elems[i];
    }
    qsort(result.list, result.size, sizeof(uint64_t), compare);
    return result;
}

List_t val_to_list(const Json::Value &val) {
    uint64_t elems[val.size()];
    for (unsigned i = 0; i < val.size(); i++)
        elems[i] = val[i].asUInt();
    return make_sorted_list_from_array(val.size(),elems);
}

std::string list_to_string(List_t p) {
    std::stringstream result;
    result << "( ";
    for (uint32_t i = 0; i < p.size; i++) {
        result << p.list[i] << " ";
    }
    result << ")";
    return result.str();
}

void allsatPrintHandler(char* varset, int size)
{
    for (int v=0; v<size; ++v)
    {
        cout << (varset[v] < 0 ? 'X' : (char)('0' + varset[v]));
    }
    cout << endl;
}