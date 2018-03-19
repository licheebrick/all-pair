#include "network.h"
#include "router.h"
#include <json/json.h>
#include <iostream>
#include <fstream>
#include <string.h>
#include<time.h>    

using namespace std;

const int router_num = 4;
string json_file_path = "./jsonfile";

int main(int argc, char* argv[]) 
{
    int flag = 0;
    printf("Which one?\n");
    cin >> flag;
    
    clock_t startTime,endTime;  
    startTime = clock();

    Network network_example(router_num);

    //read topology
    ifstream jsfile;
    Json::Value root;
    Json::Reader reader;
    string file_name = json_file_path + "/" + "topology.json";
    jsfile.open(file_name.c_str());
    if (!jsfile.good()) 
    {
        cout << "Error opening the file " << file_name << endl;
    }
    reader.parse(jsfile, root, false);
    Json::Value topology = root["topology"];
    for (unsigned i = 0; i < topology.size(); i++) 
    {
        network_example.add_link(topology[i]["src"].asUInt64(), topology[i]["dst"].asUInt64());
    }
    jsfile.close();

    network_example.print_topology();
    network_example.init();
    if(flag == 1)
        network_example.brutal_force();
    else if(flag == 2)
        network_example.warshall_with_path();
    else if(flag == 3)
        network_example.segment_based();
    else if(flag == 4)
        network_example.rule_based();
    else
        printf("1: brutal_force; 2: warshall_record_path\n");
    
    endTime = clock();  
    printf("Totle Time : %f s \n", (double)(endTime - startTime) / CLOCKS_PER_SEC);

    return 0;
}