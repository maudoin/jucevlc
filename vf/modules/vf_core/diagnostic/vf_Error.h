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

#ifndef VF_ERROR_VFHEADER
#define VF_ERROR_VFHEADER

#include "vf_SafeBool.h"

/**
  A concise error report.

  This lightweight but flexible class records lets you record the file and
  line where a recoverable error occurred, along with some optional human
  readable text.

  A recoverable error can be passed along and turned into a non recoverable
  error by throwing the object: it's derivation from std::exception is
  fully compliant with the C++ exception interface.

  @ingroup vf_core
*/   
class Error
  : public std::exception
  , public SafeBool <Error>
{
public:
  /** Numeric code.

    This enumeration is useful when the caller needs to take different
    actions depending on the failure. For example, trying again later if
    a file is locked.
  */
  enum Code
  {
    success,        //!< "the operation was successful"

    general,        //!< "a general error occurred"

    canceled,       //!< "the operation was canceled"
    exception,      //!< "an exception was thrown"
    unexpected,     //!< "an unexpected result was encountered"
    platform,       //!< "a system exception was signaled"

    noMemory,       //!< "there was not enough memory"
    noMoreData,     //!< "the end of data was reached"
    invalidData,    //!< "the data is corrupt or invalid"
    bufferSpace,    //!< "the buffer is too small"
    badParameter,   //!< "one or more parameters were invalid"
    assertFailed,   //!< "an assertion failed"

    fileInUse,	    //!< "the file is in use"
    fileExists,	    //!< "the file exists"
    fileNoPerm,	    //!< "permission was denied" (file attributes conflict)
    fileIOError,    //!< "an I/O or device error occurred"
    fileNoSpace,    //!< "there is no space left on the device"
    fileNotFound,   //!< "the file was not found"
    fileNameInvalid //!< "the file name was illegal or malformed"
  };

  Error ();
  Error (const Error& other);
  Error& operator= (const Error& other);

  virtual ~Error() noexcept;

  Code code () const;
  bool failed () const;

  bool asBoolean () const;

  const String getReasonText () const;
  const String getSourceFilename () const;
  int getLineNumber () const;

  Error& fail (const char* sourceFileName,
    int lineNumber,
    const String reasonText,
    Code errorCode = general);

  Error& fail (const char* sourceFileName,
    int lineNumber,
    Code errorCode = general);

  // A function that is capable of recovering from an error (for
  // example, by performing a different action) can reset the
  // object so it can be passed up.
  void reset ();

  // Call this when reporting the error to clear the "checked" flag
  void willBeReported () const;

  // for std::exception. This lets you throw an Error that should
  // terminate the application. The what() message will be less
  // descriptive so ideally you should catch the Error object instead.
  const char* what() const noexcept;

  static const String getReasonTextForCode (Code code);

private:
  Code m_code;
  String m_reasonText;
  String m_sourceFileName;
  int m_lineNumber;
  mutable bool m_needsToBeChecked;

  mutable String m_what; // created on demand
  mutable const char* m_szWhat;
};

#endif
