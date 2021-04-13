#include "microbit_discoverer.hpp"

#include <libserialport.h>
#include <string.h>
#include "microbit.hpp"

using namespace std;

MicrobitDiscoverer::MicrobitDiscoverer() {

}

vector<IThing*> MicrobitDiscoverer::discover() {

    vector<IThing*> things;
    int retval;
    struct sp_port **portList;

    char* portName;
    char* portDesc;

    retval = sp_list_ports(&portList);

    if (retval == SP_OK) {
         
         for(int i=0; portList[i] != NULL; i++) {
            portDesc = sp_get_port_description(portList[i]);

            //search in port description the keywords
            char* nameMatch = strstr(portDesc, "micro:bit");
            
            if (nameMatch != NULL) {
               //found
               portName = sp_get_port_name(portList[i]);

               Microbit *microbit = new Microbit(portName);
               things.push_back(microbit);
            }
        }
    }

    sp_free_port_list(portList);

    return things;
}