#ifndef brainbox_h_
#define brainbox_h_

#include "Arduino.h"
#include "AudioStream.h"
#include "Audio.h"

class BrainBox : public AudioStream
{
  public:
    BrainBox();
    ~BrainBox();
    
    virtual void update(void);
    
  private:
    int16_t clk;
    int16_t* hold;
    int16_t* spill_a;
    int16_t* spill_b;
    int16_t* states;
    int16_t* counters;
    int16_t* counterhold;
    int16_t* prevSamples;
};

#endif
