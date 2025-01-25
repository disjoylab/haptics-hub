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

#include <Arduino.h>

#include "output_noiseshaped_pwm.h"


#if defined(__IMXRT1062__)

#define PWM_RATIO 6
#define BUFSIZE  (PWM_RATIO * AUDIO_BLOCK_SAMPLES)
#define PWM_RATE (PWM_RATIO * AUDIO_SAMPLE_RATE_EXACT)
extern uint8_t analog_write_res;

extern const struct _nspwm_pin_info_struct pwm_pin_info[];


AudioOutputNoiseShapedPWM * AudioOutputNoiseShapedPWM::instances[4] ;
int AudioOutputNoiseShapedPWM::units = 0 ;
bool AudioOutputNoiseShapedPWM::pins_in_use[64] = { false };
bool AudioOutputNoiseShapedPWM::we_are_updater = false ;


void AudioOutputNoiseShapedPWM::begin (uint8_t pin)
{
  active = false ;
  update_responsibility = false ;

  if (pin >= 64)          // sanity check
    return ;
  if (pins_in_use[pin])   // already have an instance for that pin.
    return ;
        
  int unit = allocate_unit () ;  // allocate an ISR (upto 4 of them)
  if (unit == -1)
    return ;
  pins_in_use [pin] = true ;
        
  u = 0 ; v = 0 ;         // interpolation state
  inptr = bufwrap - int (2 * AUDIO_BLOCK_SAMPLES) ;
  outptr = 0 ;
  for (int i = 0 ; i < inptr ; i++)
    buffer[i] = 0 ;
  before_update = true ;

  pwm_dma_buffer = (uint16_t *) malloc (2 * BUFSIZE * sizeof (uint16_t)) ; // in DMAMEM, double buffer
  if (pwm_dma_buffer == NULL)
    return ;

  // noise shaping state
  acc1 = acc2 = acc3 = acc4 = 0 ;

  analogWriteResolution(8);
  analogWriteFrequency (pin, PWM_RATE);
  analogWrite (pin, 0x80);

  apin.pin = pin;
  apin.info = pwm_pin_info[apin.pin];

  uint8_t dmamux_source;

  if (apin.info.type == 1)
  { //only for valid flexPWM pin:
    unsigned module = (apin.info.module >> 4) & 3;
    unsigned submodule = apin.info.module & 3;
    switch (module)
    {
    case 0:
      {
        apin.flexpwm = &IMXRT_FLEXPWM1;
        switch (submodule)
        {
        case 0: dmamux_source = DMAMUX_SOURCE_FLEXPWM1_WRITE0; break;
        case 1: dmamux_source = DMAMUX_SOURCE_FLEXPWM1_WRITE1; break;
        case 2: dmamux_source = DMAMUX_SOURCE_FLEXPWM1_WRITE2; break;
        default: dmamux_source = DMAMUX_SOURCE_FLEXPWM1_WRITE3;
        }
        break;
      }
    case 1:
      {
        apin.flexpwm = &IMXRT_FLEXPWM2;
        switch (submodule)
        {
        case 0: dmamux_source = DMAMUX_SOURCE_FLEXPWM2_WRITE0; break;
        case 1: dmamux_source = DMAMUX_SOURCE_FLEXPWM2_WRITE1; break;
        case 2: dmamux_source = DMAMUX_SOURCE_FLEXPWM2_WRITE2; break;
        default: dmamux_source = DMAMUX_SOURCE_FLEXPWM2_WRITE3;
        }
        break;
      }
    case 2:
      {
        apin.flexpwm = &IMXRT_FLEXPWM3;
        switch (submodule)
        {
        case 0: dmamux_source = DMAMUX_SOURCE_FLEXPWM3_WRITE0; break;
        case 1: dmamux_source = DMAMUX_SOURCE_FLEXPWM3_WRITE1; break;
        case 2: dmamux_source = DMAMUX_SOURCE_FLEXPWM3_WRITE2; break;
        default: dmamux_source = DMAMUX_SOURCE_FLEXPWM3_WRITE3;
        }
        break;
      }
    default:
      {
        apin.flexpwm = &IMXRT_FLEXPWM4;
        switch (submodule)
        {
        case 0: dmamux_source = DMAMUX_SOURCE_FLEXPWM4_WRITE0; break;
        case 1: dmamux_source = DMAMUX_SOURCE_FLEXPWM4_WRITE1; break;
        case 2: dmamux_source = DMAMUX_SOURCE_FLEXPWM4_WRITE2; break;
        default: dmamux_source = DMAMUX_SOURCE_FLEXPWM4_WRITE3;
        }
      }
    }

    volatile uint16_t * valReg ;
        
    switch (apin.info.channel)
    {
    case 0:  valReg = &apin.flexpwm->SM[submodule].VAL0; break;
    case 1:  valReg = &apin.flexpwm->SM[submodule].VAL3; break;
    default: valReg = &apin.flexpwm->SM[submodule].VAL5; break;
    }

    dma.begin(true);
    dma.sourceBuffer (pwm_dma_buffer, 2 * BUFSIZE * sizeof(uint16_t));
    dma.destination ((uint16_t &)valReg);
    dma.TCD->DADDR = valReg;    // fix for the above which doesn't pass the address right

    dma.triggerAtHardwareEvent(dmamux_source);
    dma.TCD->CSR = DMA_TCD_CSR_INTHALF | DMA_TCD_CSR_INTMAJOR;
    dma.attachInterrupt(unit == 0 ? isr0 :
                        unit == 1 ? isr1 :
                        unit == 2 ? isr2 :
                        unit == 3 ? isr3 : NULL) ;

    //set PWM-DMA-Enable
    apin.flexpwm->SM[submodule].DMAEN = FLEXPWM_SMDMAEN_VALDE;

    //clear inital dma data:
    int16_t modulo = apin.flexpwm->SM[apin.info.module & 3].VAL1;
    for (unsigned j = 0 ; j < 2*BUFSIZE ; j++)
    {
      pwm_dma_buffer[j] = modulo / 2;
    }
    arm_dcache_flush_delete (pwm_dma_buffer, 2 * BUFSIZE * sizeof (int16_t));
                        
    update_responsibility = update_setup();
    if (update_responsibility)
      we_are_updater = true ;   // so we know the audio lib is clocking at our PWM ISR frequency

    // M and N define the ratio in ISR frequency v. update() frequency  M / N = update rate / ISR rate
    // if we have update_responsibility then N == M for 1:1 ratio in the interpolator
    M = AUDIO_SAMPLE_RATE_EXACT * (modulo+1) * PWM_RATIO ;
    N = we_are_updater ? M : F_BUS_ACTUAL ;
    t = N-M ;

    dma.enable();
    active = true ;
  }
}

int AudioOutputNoiseShapedPWM::allocate_unit (void)
{
  __disable_irq() ;  // is this needed?
  if (units >= 4)
  {
    __enable_irq() ;
    return -1 ;
  }
  int res = units ;
  instances [res] = this ;
  units += 1 ;
  __enable_irq() ;
  return res ;
}


void AudioOutputNoiseShapedPWM::isr0 (void) { if (instances[0]) instances[0]->handle_interrupt () ; }
void AudioOutputNoiseShapedPWM::isr1 (void) { if (instances[1]) instances[1]->handle_interrupt () ; }
void AudioOutputNoiseShapedPWM::isr2 (void) { if (instances[2]) instances[2]->handle_interrupt () ; }
void AudioOutputNoiseShapedPWM::isr3 (void) { if (instances[3]) instances[3]->handle_interrupt () ; }

void AudioOutputNoiseShapedPWM::handle_interrupt (void)
{
  // capture DMA progress
  uint16_t * saddr = (uint16_t *)(dma.TCD->SADDR);
  uint16_t * midpoint = pwm_dma_buffer + BUFSIZE ;

  // let other interrupts progress
  dma.clearInterrupt();

  uint16_t * dest = (uint32_t) saddr < (uint32_t) midpoint ? midpoint : pwm_dma_buffer ;

  const uint32_t modulo = apin.flexpwm->SM[apin.info.module & 3].VAL1 ;

  if (before_update)
  {
    for (unsigned i = 0 ; i < BUFSIZE ; i++)
      dest[i] = modulo / 2 ;
  }
  else
  {
    int mult = 2 ;
    switch (F_BUS_ACTUAL)
    {
    case 24000000:   mult = 2 ; break ;
    case 75600000:   mult = 1 ; break ;
    case 132000000:  mult = 2 ; break ;
    case 150000000:  mult = 2 ; break ;
    }
    for (unsigned i = 0 ; i < AUDIO_BLOCK_SAMPLES ; i++)
    {
      // current don't interpolate between samples for the 6 PWM cycles
      for (unsigned j = 0 ; j < PWM_RATIO ; j++)
      { // 4th order noise shaping filter, 4 integrators with feedback
        acc1 += buffer [outptr] ;
        acc2 += acc1 ;
        acc3 += acc2 ;
        acc4 += acc3 ;
        int16_t topbits = acc4 >> 8 ;   // use top 8 bits for PWM which allows fast enough PWM rate for easy filtering
        int32_t feedback = topbits << 8 ;
        acc1 -= feedback ;
        acc2 -= feedback ;
        acc3 -= feedback ;
        acc4 -= feedback ;
        dest [PWM_RATIO*i+j] = mult * topbits + modulo / 2 ;
      }
      // step the ring buffer
      outptr += 1 ;
      if (outptr >= bufwrap) outptr = 0 ;
    }
  }
  arm_dcache_flush_delete (dest, BUFSIZE * sizeof (uint16_t));
        
  if (update_responsibility)
    update_all();
}

void AudioOutputNoiseShapedPWM::update (void)
{
  if (!active)
    return ;
  audio_block_t * new_block = receiveReadOnly();
        
  // clocking update()s at the same rate as ISRs if an instance of AudioOutputNoiseShapedPWM has update responsibility
  if (we_are_updater)
    N = M ;

  expand (new_block) ;
  before_update = false ;
  if (new_block)
    release (new_block) ;
}


void AudioOutputNoiseShapedPWM::expand (audio_block_t * inbuf)
{
  float mult = 1.0;
  switch (F_BUS_ACTUAL)
  {
  case  24000000:  mult = 1.0 ; break ;
  case  75600000:  mult = 1.0 ; break ;
  case 132000000:  mult = 0.92 ; break ;
  case 150000000:  mult = 1.0 ; break ;
  }
  // short cut if no interpolation needed:
  if (M == N)
  {
    for (int j = 0 ; j < AUDIO_BLOCK_SAMPLES ; j++)
    {
      int16_t w = inbuf ? inbuf->data[j] : 0 ;
      // write to ring buffer
      buffer [inptr++] = round (mult * w) ;
      if (inptr >= bufwrap) inptr = 0 ;
    }
    return ;
  }
  // interpolator using three latest samples u,v,w and quadratic interpolation formula
  for (int j = 0 ; j < AUDIO_BLOCK_SAMPLES ; j++)
  {
    int16_t w = inbuf ? inbuf->data[j] : 0 ;
    t -= N ;              //   M / N = update rate / ISR rate 
    while (t < N-M)       //   so t changes by -N per update sample, +M per ISR sample, balancing 
    {                     //   the equation  N * update rate == M * ISR rate
      t += M ;
      float d = (float) t / N ;   // d is the intersample fraction relative to incoming
      int16_t out = (int16_t) (round (mult * (((u+w)/2-v) * d*d + (w-u)/2 * d + v))) ;  // quadratic interp
      buffer [inptr++] = out ;
      if (inptr >= bufwrap) inptr = 0 ;
    }
    u = v ;
    v = w ;
  }
}
                        
#endif // __IMXRT1062__

