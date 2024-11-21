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

void _main_statemachine(openlcb_node_t* openlcb_node) {
    
    // Pop OpenLcb Messages and start dispatching.
    
    
    
}

void MainStatemachine_run() {
    
    openlcb_node_t* next_node = Node_get_first();   
    while (next_node) {
        
      _main_statemachine(next_node);
       next_node = Node_get_next();  
       
    }
    
}

void MainStatemachine_run_with_node(openlcb_node_t* openlcb_node) {
    
    _main_statemachine(openlcb_node);
    
}



