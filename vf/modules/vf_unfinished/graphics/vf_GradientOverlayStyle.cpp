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

void GradientOverlayStyle::operator () (Pixels destPixels)
{
  jassert (destPixels.isRGB ());

  if (!active)
    return;

  SharedTable <Colour> table;

  if (reverse)
    table = colours.withMultipliedAlpha (float (opacity)).withReversedStops().createLookupTable ();
  else
    table = colours.withMultipliedAlpha (float (opacity)).createLookupTable ();

#if 1
  Point <int> p0 (int (destPixels.getWidth () * .4), int (destPixels.getHeight () * .4));
  Point <int> p1 (int (destPixels.getWidth () * .5), int (destPixels.getHeight () * .55));
#else
  Point <int> p0 (0, 0);
  Point <int> p1 (200, 40);
#endif

  if (reverse)
  {
    std::swap (p0, p1);
  }

  switch (kind)
  {
  case kindLinear:
    {
    Linear() (
      destPixels.getRows (),
      destPixels.getCols (),
      p0.getX(), p0.getY(), p1.getX(), p1.getY (),
      table.getNumEntries () - 1,
      Render (destPixels, table));
    }
    break;

  case kindRadial:
    // TEMPORARY, for testing purposes
    LinearFloat() (
      destPixels.getRows (),
      destPixels.getCols (),
      p0.getX(), p0.getY(), p1.getX(), p1.getY (),
      table.getNumEntries () - 1,
      Render (destPixels, table));
    //jassertfalse;
    break;

  case kindAngle:
    BlendMode::apply (
      mode,
      AngleGradientIterator (destPixels, destPixels.getBounds().getCentre (), table.getNumEntries () - 1),
      BlendProc::ARGB::CopyTable (table));
    break;

  case kindReflected:
    jassertfalse;
    break;

  case kindDiamond:
    BlendMode::apply (
      mode,
      DiamondGradientIterator (destPixels, destPixels.getBounds().getCentre (), table.getNumEntries () - 1),
      BlendProc::ARGB::CopyTable (table));
    break;

  default:
    jassertfalse;
    break;
  }
}
