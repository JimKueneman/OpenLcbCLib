/*
 * File:   openlcb_main_statemachine.c
 * Author: jimkueneman
 *
 * Created on November 20, 2024, 6:18 PM
 */


#include "xc.h"
#include "openlcb_buffer_store.h"
#include "openlcb_types.h"
#include "openlcb_defines.h"
#include "openlcb_node.h"
#include "openlcb_buffer_fifo.h"
#include "openlcb_utilities.h"


void MainStatemachine_initialize() {
    
    
}

void MainStatemachine_run() {
    
    openlcb_node_t* next_node = Node_get_first();   
    while (next_node) {
        
      
       next_node = Node_get_next();  
       
    }
    
}



