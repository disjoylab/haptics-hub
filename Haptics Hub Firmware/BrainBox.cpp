#include "BrainBox.h"

#define AUDIO_INPUTS 2
#define AUDIO_OUTPUTS 7

// Definitions:

BrainBox::BrainBox() :
AudioStream(AUDIO_OUTPUTS, new audio_block_t*[AUDIO_OUTPUTS]),
clk(0)
{
  hold = new int16_t[AUDIO_OUTPUTS];     
  spill_a = new int16_t[AUDIO_OUTPUTS];
  spill_b = new int16_t[AUDIO_OUTPUTS]; 
  states = new int16_t[AUDIO_OUTPUTS];
  counters = new int16_t[AUDIO_OUTPUTS];
  counterhold = new int16_t[AUDIO_OUTPUTS];
  for (int i=0; i<AUDIO_OUTPUTS; i++){                         
    hold[i] = 0;
    spill_a[i] = 0;    
    spill_b[i] = 0;  
    states[i] = -1; 
    counters[i] = 0;    
    counterhold[i] = 0;                             
  }  
  prevSamples = new int16_t[8];
  for (int i=0; i<8; i++){                         
    prevSamples[i] = 0;                             
  }                          
}

BrainBox::~BrainBox(){}

// The Main Function:

void BrainBox::update(void) {
  uint16_t bigByte = 0;
  uint16_t littleByte = 0;
  // set up inputs/outputs and read inputs:
  audio_block_t* inBlock[AUDIO_INPUTS];
  audio_block_t* outBlock[AUDIO_OUTPUTS];
  inBlock[0] = receiveReadOnly(0);
  inBlock[1] = receiveReadOnly(1); 
  // check that inputs were read correctly:
  if (inBlock[0]&&inBlock[1]){ 
    for (int channel = 0; channel < AUDIO_OUTPUTS; channel++) {
      // for each output channel, allocate an audio block, and check that it was done properly:
      outBlock[channel] = allocate();
      if (outBlock[channel]) {
        for (int i = 0; i < AUDIO_BLOCK_SAMPLES; i++) {
          // for each input sample, update the clock variable
          clk = inBlock[1]->data[i];
          // now check that it's actually a clock high pulse
          if (clk > 17000) // 2^14 = 16384, 2^15-1 = 32767
          {
            // we treat channels 1-6 differently than 7 and 8 for hardware reasons:
            if (channel < 6)
            {
              if ((i+channel-8) < 0)
              {
                // look at prevSamples if the clock and channel tell us to look at a sample from the previous block
                hold[channel] = prevSamples[i+channel];
              }
              else
              {
                // otherwise, look at the sample that the clock and channel tell us to
                hold[channel] = inBlock[0]->data[i+channel-8];
              }
            }
            // now for channels 7 and 8, which need to be combined into one:
            else
            {
              if ((i+channel-8) < 0)
              {
                // if needed, look at the previous samples for ch 7 and 8, and do bitwise math to combine them
                bigByte = (32768 + prevSamples[i+channel])&65280;
                littleByte = (32768 + prevSamples[i+channel+1])>>8;
                hold[channel] = bigByte+littleByte-32768;
              }
              else
              {
                // otherwise, look at samples within the current block for ch 7 and 8, and again do bitwise math to combine them
                bigByte = (32768 + inBlock[0]->data[i+channel-8])&65280;
                littleByte = (32768 + inBlock[0]->data[i+channel-7])>>8;
                hold[channel] = bigByte+littleByte-32768;
              }
            }
          }
          // on the last round through the samples, store a new set of previous samples for the next block
          if ((channel==AUDIO_OUTPUTS-1) && (i+9>AUDIO_BLOCK_SAMPLES))
          {
            prevSamples[i+8-AUDIO_BLOCK_SAMPLES] = inBlock[0]->data[i];
          }
          // update the output at every sample
          outBlock[channel]->data[i] = hold[channel];
        }
        // send the output block for the current channel, and then release it
        transmit(outBlock[channel], channel);
        release(outBlock[channel]);
      }
    }
  }
  // if there was an input block, release that block too
  if(inBlock[0]) release(inBlock[0]);
  if(inBlock[1]) release(inBlock[1]);
}
