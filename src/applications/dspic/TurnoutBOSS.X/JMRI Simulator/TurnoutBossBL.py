# This is a simulation of a TurnoutBoss left-end board (BL)
#  Bob Jacobsen   2025

import jmri

# set default states
sensors.getSensor("OML").state = INACTIVE
sensors.getSensor("OTL").state = INACTIVE
sensors.getSensor("OMC").state = INACTIVE
sensors.getSensor("OSC").state = INACTIVE
sensors.getSensor("OTR").state = INACTIVE
sensors.getSensor("OMR").state = INACTIVE

turnouts.getTurnout("TL").state = CLOSED  # eventually needs to handle TLC & TLO
        
sensors.getSensor("ScdBALstop").state = INACTIVE
sensors.getSensor("SaBRstop").state = INACTIVE
sensors.getSensor("SbBRstop").state = INACTIVE
        
sensors.getSensor("SHDBL").state = INACTIVE
sensors.getSensor("SCLBL").state = INACTIVE
sensors.getSensor("SCRBL").state = INACTIVE
sensors.getSensor("SCBBL").state = ACTIVE  # default is ACTIVE

# define the simulation class
class TurnoutBossBL(jmri.jmrit.automat.AbstractAutomaton) :
    def handle(self) :
        self.waitMsec(50)  # instead of waiting for changes
        
        OML  = sensors.getSensor("OML").state == ACTIVE
        OTL  = sensors.getSensor("OTL").state == ACTIVE
        OMC  = sensors.getSensor("OMC").state == ACTIVE
        OSC  = sensors.getSensor("OSC").state == ACTIVE
        OTR  = sensors.getSensor("OTR").state == ACTIVE
        OMR  = sensors.getSensor("OMR").state == ACTIVE

        TLO  = turnouts.getTurnout("TL").state == CLOSED  # eventually needs to handle TLC & TLO
        
        ScdBALstop = sensors.getSensor("ScdBALstop").state == ACTIVE
        SaBRstop   = sensors.getSensor("SaBRstop").state == ACTIVE
        SbBRstop   = sensors.getSensor("SbBRstop").state == ACTIVE
        
        SHD     = sensors.getSensor("SHDBL").state == ACTIVE
        SCL     = sensors.getSensor("SCLBL").state == ACTIVE
        SCR     = sensors.getSensor("SCRBL").state == ACTIVE
        SCB     = sensors.getSensor("SCBBL").state == ACTIVE
        
        SaBL = "Stop"  # Red
        SaBLstop = True
        if ( (SCB or SCL) and TLO and not OML and not OTL and not ScdBALstop) :
            SaBL = "Slow Clear"     # Green
            SaBLstop = False   
        if ((SCB or SCL) and TLO and not OML and not OTL and ScdBALstop) :
            SaBL = "Restricting"    # Yellow
            SaBLstop = False   
        
        SbBL = "Stop"  # Red
        SbBLstop = True
        if ((SCB or SCL) and not TLO and not OML and not OTL and not ScdBALstop) :
            SbBL = "Slow Clear"     # Green
            SbBLstop = False   
        if ((SCB or SCL) and not TLO and not OML and not OTL and ScdBALstop) :
            SbBL = "Restricting"    # Yellow
            SbBLstop = False   
        
        # Note: This only implements double head signaling at present
        ScBL = "Stop"  # Red
        if ((SCB or SCR) and TLO and not OTL and not OMC and not SaBRstop) :
            ScBL = "Slow Clear"     # Green 
        if ((SCB or SCR) and TLO and not OTL and not OMC and SaBRstop) :
            ScBL = "Restricting"     # Yellow
            
        SdBL = "Stop"  # Red
        if ((SCB or SCR) and not TLO and not OTL and not OSC and not SbBRstop) :
            SdBL = "Slow Clear"     # Green 
        if ((SCB or SCR) and not TLO and not OTL and not OSC and SbBRstop) :
            SdBL = "Restricting"     # Yellow
        
        ScdBLstop = (ScBL == "Stop") and (SdBL == "Stop")
 
        
        # drive the signals for output
        self.setMast(SaBL, "SaBL")
        self.setSensor(SaBLstop, "SaBLstop")
        
        self.setMast(SbBL, "SbBL")
        self.setSensor(SbBLstop, "SbBLstop")
        
        self.setMast(ScBL, "ScBL")
        self.setMast(SdBL, "SdBL")
        self.setSensor(ScdBLstop, "ScdBLstop")
        
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
    
TurnoutBossBL().start()
