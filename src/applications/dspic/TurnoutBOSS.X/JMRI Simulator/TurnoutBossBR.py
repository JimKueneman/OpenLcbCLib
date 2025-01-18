# This is a simulation of a TurnoutBoss right-end board (BL)
#  Bob Jacobsen   2025

import jmri

# set default states
sensors.getSensor("OML").state = INACTIVE
sensors.getSensor("OTL").state = INACTIVE
sensors.getSensor("OMC").state = INACTIVE
sensors.getSensor("OSC").state = INACTIVE
sensors.getSensor("OTR").state = INACTIVE
sensors.getSensor("OMR").state = INACTIVE

turnouts.getTurnout("TR").state = CLOSED  # eventually needs to handle TRC & TRO
        
sensors.getSensor("ScdBARstop").state = INACTIVE
sensors.getSensor("SaBRstop").state = INACTIVE
sensors.getSensor("SbBRstop").state = INACTIVE
        
sensors.getSensor("SHDBR").state = INACTIVE
sensors.getSensor("SCLBR").state = INACTIVE
sensors.getSensor("SCRBR").state = INACTIVE
sensors.getSensor("SCBBR").state = ACTIVE  # default is ACTIVE

# define the simulation class
class TurnoutBossBR(jmri.jmrit.automat.AbstractAutomaton) :
    def handle(self) :
        self.waitMsec(50)  # instead of waiting for changes
        
        OML  = sensors.getSensor("OML").state == ACTIVE
        OTL  = sensors.getSensor("OTL").state == ACTIVE
        OMC  = sensors.getSensor("OMC").state == ACTIVE
        OSC  = sensors.getSensor("OSC").state == ACTIVE
        OTR  = sensors.getSensor("OTR").state == ACTIVE
        OMR  = sensors.getSensor("OMR").state == ACTIVE

        TRO  = turnouts.getTurnout("TR").state == CLOSED  # eventually needs to handle TLC & TLO
        
        ScdBARstop = sensors.getSensor("ScdBARstop").state == ACTIVE
        SaBLstop   = sensors.getSensor("SaBLstop").state == ACTIVE
        SbBLstop   = sensors.getSensor("SbBLstop").state == ACTIVE

        SHD     = sensors.getSensor("SHDBR").state == ACTIVE
        SCL     = sensors.getSensor("SCLBR").state == ACTIVE
        SCR     = sensors.getSensor("SCRBR").state == ACTIVE
        SCB     = sensors.getSensor("SCBBR").state == ACTIVE

        SaBR = "Stop"  # Red
        SaBRstop = True
        if ( (SCB or SCL) and TRO and not OMR and not OTR and not ScdBARstop) :
            SaBR = "Slow Clear"     # Green
            SaBRstop = False   
        if ((SCB or SCR) and TRO and not OMR and not OTR and ScdBARstop) :
            SaBR = "Restricting"    # Yellow
            SaBRstop = False   
        
        SbBR = "Stop"  # Red
        SbBRstop = True
        if ((SCB or SCR) and not TRO and not OMR and not OTR and not ScdBARstop) :
            SbBR = "Slow Clear"     # Green
            SbBRstop = False   
        if ((SCB or SCR) and not TRO and not OMR and not OTR and ScdBARstop) :
            SbBR = "Restricting"    # Yellow
            SbBRstop = False   
        
        # Note: This only implements double head signaling at present
        ScBR = "Stop"  # Red
        if ((SCB or SCL) and TRO and not OTR and not OMC and not SaBLstop) :
            ScBR = "Slow Clear"     # Green 
        if ((SCB or SCL) and TRO and not OTR and not OMC and SaBLstop) :
            ScBR = "Restricting"     # Yellow
            
        SdBR = "Stop"  # Red
        if ((SCB or SCL) and not TRO and not OTR and not OSC and not SbBLstop) :
            SdBR = "Slow Clear"     # Green 
        if ((SCB or SCL) and not TRO and not OTR and not OSC and SbBLstop) :
            SdBR = "Restricting"     # Yellow
        
        ScdBRstop = (ScBR == "Stop") and (SdBR == "Stop")
 
        
        # drive the signals for output
        self.setMast(SaBR, "SaBR")
        self.setSensor(SaBRstop, "SaBRstop")
        
        self.setMast(SbBR, "SbBR")
        self.setSensor(SbBRstop, "SbBRstop")
        
        self.setMast(ScBR, "ScBR")
        self.setMast(SdBR, "SdBR")
        self.setSensor(ScdBRstop, "ScdBRstop")
        
        return True

    # set a mast to an aspect if it's not already at that aspect
    def setMast(self, aspect, name) :
        mast = masts.getSignalMast(name)
        if mast.getAspect() != aspect :
            mast.setAspect(aspect)
 
    # set a sensor to a state if not already at that state
    def setSensor(self, state, name) :
        sensor = sensors.getSensor(name)
        setting = ACTIVE if state else INACTIVE
        if sensor.state != setting :
            sensor.state = setting
    
TurnoutBossBR().start()
