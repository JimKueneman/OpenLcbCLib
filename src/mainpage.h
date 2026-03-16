/** 
 * @mainpage OpenLcbCLib Documentation
 * 
 * 
 * @section Welcome
 * 
 * <div style="display: flex; align-items: center;">
 *   <img src="Diesel.png" alt="Image" align="left" style="margin-right: 15px; width: 128px;">
 *   <span>
 *     <h2>OpenLcbCLib - create OpenLcb (or NMRA LCC) node firmware in C easily.</h2>      
 *   </span>
 * 
 *   <p style="margin-left: 50px; margin-right: 50px;">
 *     <h4>The OpenLcbCLib library handles all low level interactions required to be compliant on the OpenLcb/LCC network for you to allow the developer to focus on writing application specific code.</h4>
 *   </p>
 * </div>
 * 
 * <hr>
 *
 * <h2 style="margin-left: 10px; margin-right: 10px;">Implementation Guide</h2>
 *   <p style="margin-left: 50px; margin-right: 50px;">
 *     <a href="../overviews/index.html">OpenLCB C Library Implementation Guide</a> — In-depth documentation covering architecture, state machines, protocol handlers, data flow, and extending the library.
 *   </p>
 *
 * <hr>
 *
 * <h2 style="margin-left: 10px; margin-right: 10px;">Getting Started Guides</h2>
 *   <p style="margin-left: 50px; margin-right: 50px;">
 *     <a href="../../QuickStartGuide.pdf">Quick Start Guide</a> — Get an OpenLCB/LCC node running on ESP32 with Arduino IDE in minutes. Covers hardware wiring, project setup from the BasicNode demo, Node IDs, and uploading your first sketch.
 *   </p>
 *   <p style="margin-left: 50px; margin-right: 50px;">
 *     <a href="../../DeveloperGuide.pdf">Developer Guide</a> — In-depth walkthrough of the Node Wizard, project file structure, driver implementation, callbacks, events, configuration memory, and all supported platforms.
 *   </p>
 *
 * <hr>
 *
 * <h2 style="margin-left: 10px; margin-right: 10px;">Node Wizard</h2>
 *   <p style="margin-left: 50px; margin-right: 50px;">
 *     <a href="../../../tools/node_wizard/node_wizard.html">Node Wizard</a> — Browser-based tool for configuring OpenLCB nodes. Generates the user configuration files, CDI/FDI descriptors, and application entry point for your target platform. No installation or internet connection required.
 *   </p>
 *
 * <hr>
 *
 * <h2 style="margin-left: 10px; margin-right: 10px;">CDI / FDI Editor</h2>
 *   <p style="margin-left: 50px; margin-right: 50px;">
 *     <a href="../../../tools/node_wizard/cdi_fdi_wizard.html">CDI / FDI Editor</a> — Standalone browser-based editor for creating and editing CDI (Configuration Description Information) and FDI (Function Description Information) XML descriptors. Useful for authoring or updating CDI/FDI on existing nodes without running the full Node Wizard.
 *   </p>
 *
 * <hr>

 * <h2 style="margin-left: 10px; margin-right: 10px;">License</h2>
 *
 *   <p style="margin-left: 50px; margin-right: 50px;">
 *       All rights reserved.
 *       Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:
 *   </p>
 *   <ul style="margin-left: 50px; margin-right: 50px;">
 *       <li>
 *           Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer. 
 *       </li>
 *       <li>
 *           Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
 *      </li>
 *  </ul>
 *  <p style="margin-left: 50px; margin-right: 50px;"> 
 *      THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" 
 *      AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 *      IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 *      ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
 *      LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 *      CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 *      SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 *      INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 *      CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 *      ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 *      POSSIBILITY OF SUCH DAMAGE.
 *   </p>
 *
 * 
 */
