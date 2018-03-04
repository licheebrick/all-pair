#include "network.h"
#include "router.h"

using namespace std;

void write_rules()
{

}

int main(int argc, char* argv[])
{
    write_rules();
    Network network_example;
    network_example.init();
    network_example.all_pair_reachability();   

    return 0;
}