#ifndef OP_BattleTimes_h
#define OP_BattleTimes_h

// This is kind of silly, but due to complicated include rules, we need the definition of REPAIR_TIME_mS to be in a separate h file. 
// This file needs to be included in both OP_Tank.h and OP_TBS.h (and other future sound header files). 
// It would make most sense to put these definitions in OP_Tank.h, but we can't include OP_Tank.h in OP_TBS.h


// Repairs take 15 seconds
#define REPAIR_TIME_mS                  15000   // How long does a repair operation take. During this time the tank can still receive hits, but 
                                                // it can't move. 

// Tanks is dead for 15 seconds 
#define DESTROYED_INOPERATIVE_TIME_mS   15000   // How long is the vehicle immobilized after being destroyed. 15 seconds is the Tamiya spec. After this,
                                                // the vehicle will automatically re-generate with full health restored. 
                                            


#endif 
