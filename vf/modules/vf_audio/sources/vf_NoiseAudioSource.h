/*============================================================================*/
/*
  VFLib: https://github.com/vinniefalco/VFLib

  Copyright (C) 2008 by Vinnie Falco <vinnie.falco@gmail.com>

  This library contains portions of other open source products covered by
  separate licenses. Please see the corresponding source files for specific
  terms.
  
  VFLib is provided under the terms of The MIT License (MIT):

  Permission is hereby granted, free of charge, to any person obtaining a copy
  of this software and associated documentation files (the "Software"), to deal
  in the Software without restriction, including without limitation the rights
  to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
  copies of the Software, and to permit persons to whom the Software is
  furnished to do so, subject to the following conditions:

  The above copyright notice and this permission notice shall be included in
  all copies or substantial portions of the Software.

  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
  IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
  FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
  AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
  LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
  FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS
  IN THE SOFTWARE.
*/
/*============================================================================*/

#ifndef VF_NOISEAUDIOSOURCE_VFHEADER
#define VF_NOISEAUDIOSOURCE_VFHEADER

/*============================================================================*/
/**
  @internal
 
  @brief Filter to make pink noise from white.

  From http://www.musicdsp.org/files/pink.txt

  @ingroup vf_audio internal
*/
struct pink_noise_filter
{
  // +/-0.05dB above 9.2Hz @ 44,100 Hz Fs
  pink_noise_filter ()
  {
    b0 = 0;
    b1 = 0;
    b2 = 0;
    b3 = 0;
    b4 = 0;
    b5 = 0;
    b6 = 0;
  }

  template <typename Sample>
  inline Sample process (const Sample white)
  {
    b0 = 0.99886 * b0 + white * 0.0555179;
    b1 = 0.99332 * b1 + white * 0.0750759;
    b2 = 0.96900 * b2 + white * 0.1538520;
    b3 = 0.86650 * b3 + white * 0.3104856;
    b4 = 0.55000 * b4 + white * 0.5329522;
    b5 = -0.7616 * b5 - white * 0.0168980;
    const double pink = b0 + b1 + b2 + b3 + b4 + b5 + b6 + white * 0.5362;
    b6 = white * 0.115926;
    return static_cast <Sample> (pink);
  }

private:
  double b0, b1, b2, b3, b4, b5, b6;
};

/*============================================================================*/
/**
  @internal

  @brief A faster but less accurate pink noise filter.

  @ingroup vf_audio internal
*/
struct fast_pink_noise_filter
{
  // +/-0.5dB above 9.2Hz @ 44,100 Hz Fs
  fast_pink_noise_filter ()
  {
    b0 = 0;
    b1 = 0;
    b2 = 0;
  }

  template <typename Sample>
  inline Sample process (const Sample white)
  {
    b0 = 0.99765 * b0 + white * 0.0990460;
    b1 = 0.96300 * b1 + white * 0.2965164;
    b2 = 0.57000 * b2 + white * 1.0526913;
    const double pink = b0 + b1 + b2 + white * 0.1848;
    return static_cast <Sample> (pink);
  }

private:
  double b0, b1, b2, b3, b4, b5, b6;
};

/*============================================================================*/
/**
  An AudioSource to produce noise.

  The noise can be pink or white.

  @todo Refactor to produce only white noise. Create a new AudioSource called
        PinkNoiseFilter that can be chained onto another AudioSource. Create
        PinkNoiseAudioSource as a composition of NoiseAudioSource and
        PinkNoiseFilter.

  @ingroup vf_audio
*/
class NoiseAudioSource : public AudioSource
{
public:
  /** @param pink `true` for pink noise. */
  explicit NoiseAudioSource (bool pink = false);
  
  ~NoiseAudioSource ();

  void prepareToPlay (int samplesPerBlockExpected,
                      double sampleRate);

  void releaseResources();

  void getNextAudioBlock (const AudioSourceChannelInfo& bufferToFill);
  
private:
  void white_noise (int numSamples, float* dest);
  void pink_noise (int numSamples, float* dest);

  bool m_pink;
  Random m_random;
  pink_noise_filter m_filter;
};

#endif
