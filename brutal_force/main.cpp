#include "network.h"
#include "router.h"
#include <json/json.h>
#include <fstream>
#include <string.h>

//#pragma comment(dylib, "libjsoncpp.dylib")

using namespace std;

string json_file_path = "./jsonfile";

void write_rules()
{

}

int main(int argc, char* argv[])
{
    write_rules();
    Network network_example;

    //read topology
    ifstream jsfile;
    Json::Value root;
    Json::Reader reader;
    string file_name = json_file_path + "/" + "topology.json";
    jsfile.open(file_name.c_str());
    if (!jsfile.good()) {
        cout << "Error opening the file " << file_name << endl;
        // stringstream err_msg;
        // LOG4CXX_ERROR(rlogger, err_msg.str()); //fail to use log4cxx
        // return t_list;
    }
    reader.parse(jsfile, root, false);
    Json::Value topology = root["topology"];
    for (unsigned i = 0; i < topology.size(); i++) 
    {
        network_example.add_link(topology[i]["src"].asUInt64(), topology[i]["dst"].asUInt64());
    }
    //LOG4CXX_DEBUG(flogger, "miaomiaomiao");
    jsfile.close();

    network_example.print_topology();
    network_example.init();
    //network_example.print_topology();
    network_example.all_pair_reachability();

    return 0;
}