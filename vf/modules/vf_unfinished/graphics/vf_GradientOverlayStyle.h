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

#ifndef VF_GRADIENTOVERLAYSTYLE_VFHEADER
#define VF_GRADIENTOVERLAYSTYLE_VFHEADER

/** Computation of gradient blends.

    @ingroup vf_gui
*/
struct GradientOverlayStyle
{
  enum Kind
  {
    kindLinear = 1,
    kindRadial,
    kindAngle,
    kindReflected,
    kindDiamond
  };

  GradientOverlayStyle () : active (false)
  {
  }

  bool            active;
  BlendMode::Type mode;
  double          opacity;
  GradientColours colours;
  bool            reverse;
  Kind            kind;
  double          angle;
  double          scale;
  Point <int>     startPoint;
  Point <int>     endPoint;

  void operator() (Pixels destPixels);

  //----------------------------------------------------------------------------

  struct RenderDebug
  {
    explicit RenderDebug (Pixels dest, SharedTable <Colour>)
      : m_dest (dest)
    {
    }

    template <class T>
    inline void operator() (int const x, int const y, T const t) const noexcept
    {
      PixelARGB& dest (*((PixelARGB*)m_dest.getPixelPointer (x, y)));
      dest.getRed ()    = uint8 (t);
      dest.getGreen ()  = uint8 (t);
      dest.getBlue ()   = uint8 (t);
    }

  private:
    Pixels m_dest;
  };

  //----------------------------------------------------------------------------

  struct Render
  {
    explicit Render (Pixels dest, SharedTable <Colour> table)
      : m_dest (dest)
      , m_table (table)
    {
    }

    template <class T>
    inline void operator() (int const x, int const y, T const t) const noexcept
    {
      PixelRGB& dest (*((PixelRGB*)m_dest.getPixelPointer (x, y)));

      dest.blend (m_table [int (t)].getPixelARGB ());
    }

  private:
    Pixels m_dest;
    SharedTable <Colour> m_table;
  };

  //----------------------------------------------------------------------------

  struct DDA
  {
    DDA (int rise, int run, int dir_ = 1)
      : mod (run)
      , step (rise % run)
      , skip (rise / run)
      , dir (dir_)
    {
    }

    inline void advance (int steps)
    {
      if (mod)
      {
        if (steps >= 0)
        {
          value += dir * skip * steps;
          frac  += step * steps;
          value += dir * frac / mod;
          frac   = frac % mod;
        }
        else
        {
          value += dir * skip * steps;
          frac  += step * steps;
          value += dir * (frac - (mod - 1)) / mod;
          frac  = frac % mod;
          if (frac < 0)
            frac += mod;
        }
      }
    }

    inline int operator() ()
    {
      int const result = value;

      value += dir * skip;
      frac += step;

      if (frac >= mod)
      {
        frac -= mod;
        value += dir;
      }

      return result;
    }

    int mod;
    int step;
    int skip;
    int dir;

    int frac;
    int value;
  };

  /** Linear gradient calculator.

      Uses an all-integer digital differential analyzer.
  */
  struct Linear
  {
    template <class Functor>
    void operator() (
      int rows,
      int cols,
      int x0, int y0, // normal start pt
      int x1, int y1, // normal end pt
      int const scale,
      Functor f)
    {
      int v0;
      int v1;
      int dir;

      if (x0 != x1)
      {
        // normalize coordinates
        if (x0 <= x1)
        {
          v0 = 0;
          v1 = scale;
          dir = 1;
        }
        else
        {
          v0 = scale;
          v1 = 0;
          dir = - 1;

          std::swap (x0, x1);
          std::swap (y0, y1);
        }

        if (y0 != y1)
        {
          // line slope     m  = (y1 - y0) / (x1 - x0)
          //
          // gradient slope m' = (x0 - x1) / (y1 - y0)
          //
          // width  = floor (m  * (y1 - y0)) + x1 - x0
          //
          // height = floor (m' * (x0 - x1)) + y1 - y0
          //
          int const w = x1 - x0 + ((y1 - y0) * (y1 - y0)) / (x1 - x0);
          int const h = y1 - y0 + ((x0 - x1) * (x0 - x1)) / (y1 - y0);

          DDA ddx (scale, w, dir);

          DDA ddy (scale, h, dir);
          ddy.frac = 0;
          ddy.value = v0;
          ddy.advance (-y0);

          for (int y = 0; y < rows; ++y)
          {
            ddx.frac = (ddy.frac * ddx.mod) / ddy.mod;
            ddx.value = ddy.value;
            ddx.advance (-x0);
 
            ddy ();

            for (int x = 0; x < cols; ++x)
            {
              int const value = ddx ();
          
              if (value < 0)
              {
                f (x, y, 0);
              }
              else if (value > scale)
              {
                f (x, y, scale);
              }
              else
              {
                f (x, y, value);
              }
            }
          }
        }
        else
        {
          // Special case for horizontal blend
          int const w = x1 - x0;

          for (int y = 0; y < rows; ++y)
          {
            DDA ddx (scale, w, dir);
            ddx.frac = ddx.mod / 2;
            ddx.value = v0;
            ddx.advance (-x0);

            for (int x = 0; x < cols; ++x)
            {
              int const value = ddx ();
          
              if (value < 0)
              {
                f (x, y, 0);
              }
              else if (value > scale)
              {
                f (x, y, scale);
              }
              else
              {
                f (x, y, value);
              }
            }
          }
        }
      }
      else
      {
        // Special case for vertical blend
        if (y0 <= y1)
        {
          v0 = 0;
          v1 = scale;
          dir = 1;
        }
        else
        {
          v0 = scale;
          v1 = 0;
          dir = -1;

          std::swap (y0, y1);
        }

        int const h = y1 - y0;

        DDA ddy (scale, h, dir);
        ddy.frac = 0;
        ddy.value = v0;
        ddy.advance (-y0);

        for (int y = 0; y < rows; ++y)
        {
          int value = ddy ();
          
          if (value < 0)
            value = 0;
          else if (value > scale)
            value = scale;

          for (int x = 0; x < cols; ++x)
            f (x, y, value);
        }
      }
    }
  };

  //----------------------------------------------------------------------------

  /** Linear gradient calculator.

      This uses fixed point arithmetic.
  */
  struct LinearFixed
  {
    template <class Functor>
    void operator() (
      int rows,
      int cols,
      int x0, int y0, // normal start pt
      int x1, int y1, // normal end pt
      int const scale,
      Functor f)
    {
      struct FDDA
      {
        int value;
        int step;
      };

      int const w = 1 * (x1 - x0) + (1 * (y1 - y0) * (y1 - y0)) / (x1 - x0);
      int const h = 1 * (y1 - y0) + (1 * (x0 - x1) * (x0 - x1)) / (y1 - y0);

      FDDA ddx;
      ddx.step = 65536 * scale / w;
      ddx.value = 0;

      FDDA ddy;
      ddy.step = 65536 * scale / h;
      ddy.value = - ddy.step * y0;

      for (int y = 0; y < rows; ++y)
      {
        ddx.value = ddy.value - ddx.step * x0;
        ddy.value += ddy.step;

        for (int x = 0; x < cols; ++x)
        {
          int const value = ddx.value >> 16;
          ddx.value += ddx.step;
          
          if (value < 0)
          {
            f (x, y, 0);
          }
          else if (value > scale)
          {
            f (x, y, scale);
          }
          else
          {
            f (x, y, value);
          }
        }
      }
    }
  };

  //----------------------------------------------------------------------------

  /** Produce a linear gradient.

      This implementation uses float values.
  */
  struct LinearFloat
  {
    template <class Functor>
    void operator() (
      int rows,
      int cols,
      int x0_, int y0_,
      int x1_, int y1_,
      int const scale,
      Functor f)
    {
      float x0 = float (x0_);
      float x1 = float (x1_);
      float y0 = float (y0_);
      float y1 = float (y1_);

      int v0;
      int v1;
      
      if (x0 <= x1)
      {
        v0 = 0;
        v1 = scale;
      }
      else
      {
        v0 = scale;
        v1 = 0;
      }

      if (x0 != x1)
      {
        // 1/m'
        float const m = (y1 - y0) / (x0 - x1);
        float const w =  floor (x1 - x0 - m * (y1 - y0));

        for (int y = 0; y < rows; ++y)
        {
          float const p0 = m * (y - y0) + x0;
          float const p1 = p0 + w;
          float const d = scale / w;

          for (int x = 0; x < cols; ++x)
          {
            if (x <= p0)
            {
              f (x, y, 0);
            }
            else if (x >= p1)
            {
              f (x, y, scale);
            }
            else
            {
              f (x, y, d * (x - p0) + 0.5f);
            }
          }
        }
      }
      else
      {
        // Special case for horizontal lines.
      }
    }
  };

  //----------------------------------------------------------------------------

  static inline float piFloat ()
  {
    return 3.14159265f;
  }

  static inline float piBy2Float ()
  {
    return 1.570796325f;
  }
  
  // |error| < 0.005
  static inline float fast_atan2f (float y, float x)
  {
    if ( x == 0.0f )
    {
      if ( y > 0.0f ) return piBy2Float ();
      if ( y == 0.0f ) return 0.0f;
      return -piBy2Float ();
    }
    float atan;
    float z = y/x;
    if ( fabsf( z ) < 1.0f )
    {
      atan = z/(1.0f + 0.28f*z*z);
      if ( x < 0.0f )
      {
        if ( y < 0.0f ) return atan - piFloat ();
        return atan + piFloat ();
      }
    }
    else
    {
      atan = piBy2Float () - z/(z*z + 0.28f);
      if ( y < 0.0f ) return atan - piFloat ();
    }
    return atan;
  }

  //----------------------------------------------------------------------------
  
  /** Angle gradient generator.
  */
  struct AngleGradientIterator
  {
    /** Create an angle gradient iterator.

        @param maxValue Multiplier to put value in the range [0, maxValue]
    */
    AngleGradientIterator (
      Pixels destPixels,
      Point <int> const& centerPoint,
      int maxValue)
      : m_destPixels (destPixels)
      , m_centerPoint (centerPoint)
      , m_maxValue (maxValue)
    {
    }

    /** Call functor for each pixel.
    */
    template <class Functor>
    void operator () (Functor f)
    {
      float inv = float (m_maxValue) / (2.f * piFloat ());

      for (int y = 0; y < m_destPixels.getRows (); ++y)
      {
        float const dy = float (m_centerPoint.getY () - y);

        uint8* dest = m_destPixels.getLinePointer (y);

        for (int x = 0; x < m_destPixels.getCols (); ++x)
        {
          float const rad = fast_atan2f (dy, float (m_centerPoint.getX () - x)) + piFloat ();
          int const value = static_cast <int> (inv * rad + 0.5);

          f (dest, value);

          dest = addBytesToPointer (dest, m_destPixels.getColBytes ());
        }
      }
    }

  private:
    AngleGradientIterator& operator= (AngleGradientIterator const&);

    Pixels m_destPixels;
    Point <int> const m_centerPoint;
    int const m_maxValue;
  };

  //----------------------------------------------------------------------------
  
  /** Diamond gradient generator.
  */
  struct DiamondGradientIterator
  {
    /** Create a diamond gradient iterator.

        @param maxValue Multiplier to put value in the range [0, maxValue]
    */
    DiamondGradientIterator (
      Pixels destPixels,
      Point <int> const& centerPoint,
      int maxValue)
      : m_destPixels (destPixels)
      , m_centerPoint (centerPoint)
      , m_maxValue (maxValue)
    {
    }

    /** Call functor for each pixel.
    */
    template <class Functor>
    void operator () (Functor f)
    {
      for (int y = 0; y < m_destPixels.getRows (); ++y)
      {
        float const dy = abs (float (m_centerPoint.getY () - y));

        uint8* dest = m_destPixels.getLinePointer (y);

        for (int x = 0; x < m_destPixels.getCols (); ++x)
        {
          float dist = dy + abs (m_centerPoint.getX () - x);

          int value = static_cast <int> (dist);

          if (value > m_maxValue)
            value = m_maxValue;

          f (dest, value);

          dest = addBytesToPointer (dest, m_destPixels.getColBytes ());
        }
      }
    }

  private:
    DiamondGradientIterator& operator= (DiamondGradientIterator const&);

    Pixels m_destPixels;
    Point <int> const m_centerPoint;
    int const m_maxValue;
  };
};

#endif
