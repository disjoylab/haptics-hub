/* Audio Library for Teensy 3.X
 * Copyright (c) 2014, Paul Stoffregen, paul@pjrc.com
 *
 * Development of this audio library was funded by PJRC.COM, LLC by sales of
 * Teensy and Audio Adaptor boards.  Please support PJRC's efforts to develop
 * open source software by purchasing Teensy or other PJRC products.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice, development funding notice, and this permission
 * notice shall be included in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 */

#ifndef output_noiseshaped_pwm_h_
#define output_noiseshaped_pwm_h_

#include "Arduino.h"

#include "AudioStream.h"

#include "DMAChannel.h"

#if defined(__IMXRT1062__)

struct _nspwm_pin_info_struct
{
  uint8_t type ;    // 0=no pwm, 1=flexpwm, 2=quad
  uint8_t module ;  // 0-3, 0-3
  uint8_t channel ; // 0=X, 1=A, 2=B
  uint8_t muxval ;  //
};

struct _nsaudio_info_flexpwm
{
  IMXRT_FLEXPWM_t * flexpwm ;
  _nspwm_pin_info_struct info ;
  uint8_t pin ;
};


class AudioOutputNoiseShapedPWM : public AudioStream
{
 public:
  AudioOutputNoiseShapedPWM (void) : AudioStream(1, inputQueueArray)
  {
    begin (2);
  }
  AudioOutputNoiseShapedPWM (uint8_t pin) : AudioStream(1, inputQueueArray)
  {
    begin (pin);
  }

  virtual void update(void);

 private:
  audio_block_t * inputQueueArray [1] ; 
  bool active ;   // used to inhibit update() before begin has succeeded
  bool update_responsibility ;
  uint16_t * pwm_dma_buffer ;  // allocated in DMAMEM

  void begin (uint8_t pin); //FlexPWM pin only

  // ring buffer needed for interpolation when ISR and update() rates are exactly the same
  static const uint32_t bufwrap = 4 * AUDIO_BLOCK_SAMPLES ;
  volatile int16_t buffer [bufwrap] ;
  volatile uint32_t inptr, outptr ;
  // DMA and pin stuff
  DMAChannel dma ;
  _nsaudio_info_flexpwm apin ;

  // noise shaping integrator accumulators
  volatile int32_t acc1, acc2, acc3, acc4 ;

  // interpolation state
  int32_t M, N;   // M and N are exact integer ratio between AUDIO_SAMPLE_RATE_EXACT and PWM frequency
  int32_t t;      // variable used for interpolation (Bressenhams style)
  int16_t u, v;   // u, v, w are latest 3 samples for interpolation
        
  void expand (audio_block_t * inbuf);

  volatile bool before_update ;

  static AudioOutputNoiseShapedPWM * instances[4] ;
  static int units ;               // used to allocate ISRs, upto 4 instances allowed
  static bool pins_in_use [64] ;   // bit mask of pins in use by all instances
  int allocate_unit (void) ;
  static void isr0 (void) ;        // statically avilable ISR trampolines
  static void isr1 (void) ;
  static void isr2 (void) ;
  static void isr3 (void) ;
  void handle_interrupt (void) ;   // generic ISR
        
  // set if any of the instances of AudioOutputNoiseShapedPWM have update responsibility
  static bool we_are_updater ;
};

#endif
#endif
