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

#ifndef VF_RESIZABLELAYOUT_VFHEADER
#define VF_RESIZABLELAYOUT_VFHEADER

// Subclasses have size information
//
class ResizableChild
{
public:
  ResizableChild() : m_minW(0), m_maxW(0x3fffffff), m_minH(0), m_maxH(0x3fffffff) { }

  virtual ~ResizableChild () { }

  void setMinimumWidth (int minimumWidth) { m_minW = minimumWidth; }
  void setMaximumWidth (int maximumWidth) { m_maxW = maximumWidth; }
  void setMinimumHeight (int minimumHeight) { m_minH = minimumHeight; }
  void setMaximumHeight (int maximumHeight) { m_maxH = maximumHeight; }

  void setFixedHeight (int height) { m_minH = height; m_maxH = height; }

  int getMinimumWidth() const { return m_minW; }
  int getMaximumWidth() const { return m_maxW; }
  int getMinimumHeight() const { return m_minH; }
  int getMaximumHeight() const { return m_maxH; }

  void setMinimumSize (int minimumWidth, int minimumHeight)
  {
    setMinimumWidth (minimumWidth);
    setMinimumHeight (minimumHeight);
  }

  void setMaximumSize (int maximumWidth, int maximumHeight)
  {
    setMaximumWidth (maximumWidth);
    setMaximumHeight (maximumHeight);
  }

  void setSizeLimits (
    int minimumWidth,
    int minimumHeight,
    int maximumWidth,
    int maximumHeight)
  {
    setMinimumWidth (minimumWidth);
    setMaximumWidth (maximumWidth);
    setMinimumHeight (minimumHeight);
    setMaximumHeight (maximumHeight);
  }

  virtual void resizeStart() {}

private:
  int m_minW;
  int m_maxW;
  int m_minH;
  int m_maxH;
};

//------------------------------------------------------------------------------

// Allows assignment of anchor points to determine resizing behavior.
//
class ResizableLayout
  : public ResizableChild
  , private ComponentListener
  , private AsyncUpdater
{
public:
  enum
  {
    anchorUnit=100
  };

  enum Style
  {
    styleStretch,
    styleFixedAspect
  };

  static const Point <int> anchorNone;
  static const Point <int> anchorTopLeft;
  static const Point <int> anchorTopCenter;
  static const Point <int> anchorTopRight;
  static const Point <int> anchorMidLeft;
  static const Point <int> anchorMidCenter;
  static const Point <int> anchorMidRight;
  static const Point <int> anchorBottomLeft;
  static const Point <int> anchorBottomCenter;
  static const Point <int> anchorBottomRight;

public:
  explicit ResizableLayout (Component* owner);
  ~ResizableLayout ();

  // Add a Component to the Layout.
  // topLeft and bottomRight are the percentages that the top left and bottom right of
  // the Component should move by, when the layout is resized.
  // So if you wanted to have the control take on the full width of the parent, and
  // half the height, you would use bottomRight.x=100, bottomRight.y=50. or
  // use the constant anchorMidRight
  void addToLayout (
    Component *component,
    const Point <int> &topLeft,
    const Point <int> &bottomRight = anchorNone,
    Style style = styleStretch );

  // Remove a Component from the Layout.
  void removeFromLayout (Component* component);

  // Activate (or deactivate) the Layout. The Layout is initially inactive,
  // to prevent spurious recalculation while a Component and its children are being
  // constructed (causing resized() messages). Activating the Layout for the
  // first time will cause an Update().
  void activateLayout (bool isActive=true);

  // Call this to manually update the state information for a single control
  // after it has been moved or resized from elsewhere.
  // UNFINISHED API
  void updateLayoutFor (Component *component);

  // Convenience function
  static Rectangle <int> calcBoundsOfChildren (Component* parent);

private:
  void handleAsyncUpdate ();

  // Update the state information for all items. This is used on the first Activate(),
  // and can also be used if multiple controls are moved or resized from elsewhere.
  // UNFINISHED API
  void updateLayout ();

  void resizeStart ();

private:
  struct Rect
  {
    Rect() {}
    Rect (int top0, int left0, int bottom0, int right0) { top=top0; left=left0; bottom=bottom0; right=right0; }
    Rect (const Rectangle<int> &r) { top=int(r.getY()); left=int(r.getX()); bottom=int(r.getBottom()); right=int(r.getRight()); }
    operator Rectangle<int>() const { return Rectangle<int>( left, top, getWidth(), getHeight() ); }
    int getHeight () const { return bottom-top; }
    int getWidth () const { return right-left; }
    void reduce (int dx, int dy) { top+=dy; left+=dx; bottom-=dy; right-=dx; }

    int top;
    int left;
    int bottom;
    int right;
  };

  struct Anchor
  {
    Style	style;
    Component* component;
    ResizableChild* child;
    Point<int> topLeft;
    Point<int> bottomRight;

    Anchor (Component* component=0);
    bool operator== (Anchor const& rhs) const;
    bool operator>= (Anchor const& rhs) const;
    bool operator<  (Anchor const& rhs) const;
  };

  struct State
  {
    Component* component;
    double aspect;
    Rect margin;

    State (Component* component=0);
    bool operator== (const State& rhs) const;
    bool operator>= (const State& rhs) const;
    bool operator<  (const State& rhs) const;
  };

  void addStateFor (const Anchor& anchor);

public:
  void recalculateLayout ();

private:
  void componentMovedOrResized (Component& component,
    bool wasMoved,
    bool wasResized);

  void componentBeingDeleted (Component& component);

private:
  Component* m_owner;

  SortedSet <Anchor> m_anchors;
  SortedSet <State> m_states;

  bool m_bFirstTime;
  bool m_isActive;
};

//------------------------------------------------------------------------------

// Constrains a top level window based on recursively calculated limits on children.
//
class TopLevelConstrainer
{
public:
  explicit TopLevelConstrainer (ResizableChild* owner);

  void setAsConstrainerFor (ResizableWindow* window);

private:
  class Constrainer : public ComponentBoundsConstrainer
  {
  public:
    Constrainer (ResizableChild* owner);
    void resizeStart();

  private:
    ResizableChild& m_owner;
  };

  Constrainer m_constrainer;
};

#endif
