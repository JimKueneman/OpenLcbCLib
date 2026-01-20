This document outlines the coding style used in the OpenLcbCLib library.

# Use of types:

This library uses typedef liberally to allow better type checking does not encourage the use of type casting unless absolutely necessary.

# File Names and Folders:

Filenames are all lower case with underscores between words and are descriptive.  The file structure is as follows

~~~
 -src
    |
    | - drivers     // Where drivers are located that take the physical
    |    |          // layer IO and converts it to raw openlcb message types for
    |    |          // use in the openlcb core library
    |    |
    |    | - canbus      // Where the can interface files live to support 
    |                    //  another transport layer (such as TCP/IP) create another
    |                    //  folder at this same lever and name it something more appropriate.  
    |                           
    | - openlcb    // Where core files that function on the openlcb messages in using full NodeIDs
~~~

# C Coding Style:

**Indentation**

Indentation of lines of code shall be 4 spaces, no tabs allowed, unless otherwise stated.   If the spec and examples are in conflict the value here shall be used.

**Line Length**

Any function that has a large number or long identifier names shall be split up using these rules:

1) If the line is longer than 120 characters (converting tabs equivalent number of space in the count) do the following
2) Break the parameter list up with a parameter on each line (a parameter may be the return from another function recursively in which the same rule applies).  The indentation shall be as follows:

~~~

OpenLcbUtilities_load_openlcb_message(
            statemachine_info->outgoing_msg_info.msg_ptr,
            statemachine_info->openlcb_node->alias,
            statemachine_info->openlcb_node->id,
            statemachine_info->incoming_msg_info.msg_ptr->source_alias,
            statemachine_info->incoming_msg_info.msg_ptr->source_id,
            ProtocolEventTransport_extract_consumer_event_status_mti(
                        statemachine_info->openlcb_node, 
                        statemachine_info->openlcb_node->consumers.enumerator.enum_index)
            );
            
~~~

The indentation shall be 3 times the normal indentation value.  If the resulting line is still longer than max limit above allow it.

3) For logical operator lines the same applies of the max line length above.  The line shall be broken up into multiple lines at the operator as in the example:

~~~

return (
            ((uint64_t) (*buffer)[0 + index] << 40) |
            ((uint64_t) (*buffer)[1 + index] << 32) |
            ((uint64_t) (*buffer)[2 + index] << 24) |
            ((uint64_t) (*buffer)[3 + index] << 16) |
            ((uint64_t) (*buffer)[4 + index] << 8) |
            ((uint64_t) (*buffer)[5 + index])
            );
                      
~~~

4) Pointers "*" shall be placed as shown in the example for the buffer:

~~~

int16_t OpenLcbUtilities_extract_word_from_config_mem_buffer(configuration_memory_buffer_t *buffer, uint8_t index)

~~~


## Header Files:

Header file functions use the name of the file in PascalCaseStyle case appended with a descriptive name of what the function does in lower case with words separated by underscores.  For instance in the can_buffer_store.h file to allocate a new CAN buffer you would call 

~~~
  extern can_msg_t* CanBufferStore_allocate_buffer(void);
~~~

Guards are named with 2 leading and trailing underscores with the path (starting at the level under ./src)of the module in capitals with underscores between the words.

For example, all CAN prefixed files are in the 

**/src/drivers/canbus folder** 

All openlcb prefixed files are in the 

**/src/openlcb folder**

(from above diagram) so the guard text for the can_buffer_store.h header would be

~~~
// This is a guard condition so that contents of this file are not included
// more than once.  
#ifndef __DRIVERS_CANBUS_CAN_BUFFER_STORE__
#define __DRIVERS_CANBUS_CAN_BUFFER_STORE__

#ifdef	__cplusplus
  extern "C" {
#endif /* __cplusplus */


    extern void SomeFunction(void);
    
#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif	/* __DRIVERS_CANBUS_CAN_BUFFER_STORE__ */

~~~

## Type Definitions:

Any constant that can not be changed is written in capitals with underscores between the letter this includes #defines that define a constant.

~~~

#define USER_DEFINED_NODE_BUFFER_DEPTH 1 

~~~

All #defines with mathematical operators shall be enclosed with parenthesis to ensure the math is done in the preprocessing stage.

~~~

#define USER_DEFINED_NODE_BUFFER_DEPTH (1 + 5 * 8) 

~~~

Enumerations definitions use a *trailing _enum* to signify it is a type as in the previous example below.

~~~
typedef enum {

        BASIC,
        DATAGRAM,
        SNIP,
        STREAM

    } payload_type_enum;
    
~~~

 Type definitions use a *trailing _t* to signify it is a type as in the previous example below.

~~~

typedef struct {

        openlcb_msg_state_t state;
        uint16_t mti;
        uint16_t source_alias;
        
    } openlcb_msg_t;
    
~~~
## For loop and If statements and Functions:

All for loops and If statements will use full brackets regardless of the number of statements in the following format examples, except for single line return statement as in the following example.  There will be blank space inserted as shown.

Insert one blank line after opening braces { and one blank line before closing braces } in all code blocks.

### For Loops
~~~

if (_interface->handle_try_enumerate_next_node()) { return; }
    
for (int i = 0; i < USER_DEFINED_CONSUMER_COUNT; i++) {

    openlcb_node->consumers.list[i] = 0;
    statements....

} 

if (openlcb_nodes.count == 0) {
        
    statements....
    return NULL;
        
} else {

    statements......

}
~~~

### Else/If will use the following style

~~~
if (openlcb_nodes.count == 0) {
        
    statements......
    return NULL;
        
 } else if (openlcb_nodes.count == 100) {
 
    statements......
    statements......

 } else {

    statements......
    statements......

}
~~~

### Switch statements

~~~

switch (openlcb_msg->mti) {

    case MTI_SIMPLE_NODE_INFO_REQUEST:

        statements......
        statements......

        break;

    case MTI_PROTOCOL_SUPPORT_INQUIRY:

        statements......
        statements......
            
        break;

    case MTI_VERIFY_NODE_ID_ADDRESSED:

        statements......
        statements......

        break;

    default:

        statements......
        statements......

        break;

}

~~~

### Functions and Variable Formats and Naming 

~~~

void _generate_event_ids(openlcb_node_t* openlcb_node) {

    statements......
    statements......

}

~~~

Module function naming

Within a module the following function and variable naming convention is used. 

Any function/variable that is accessed outside the module is accessed through the header file and using the name of the module in PascalCaseStyle case and lower case with underscores between words.  The following is in a module named:

> can_buffer_store.h

~~~

can_msg_t* CanBufferStore_allocate_buffer(void);

~~~

Variables used as function parameters or global variable are lower case and separated by underscores.  If the function or variable is local to module then it also begins with an underscore as in the example.  All internally used variables and functions will be defined as *static*.

~~~

static openlcb_nodes_t _openlcb_nodes;

static void _generate_event_ids(openlcb_node_t* openlcb_node) {


  }
  
~~~

