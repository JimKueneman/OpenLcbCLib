CdiToArray.py:

     Converts an XML file into the minimal array of ASCII values to us in the node parameters structure including reporting the number of characters in th conversion.
     
     Usage:  python CdiToArray.py My_Cdi_File.xml
     
     the resulting file will the named My_Cdi_File.xml.c but it is not a valid C file it is just the values that can be cut and pasted into the node parameters structure initializer in the user application to embed the CDI into the Flash or RAM depending on the target CPU/System.


