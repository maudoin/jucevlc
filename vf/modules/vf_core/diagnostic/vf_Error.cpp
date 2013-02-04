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

Error::Error()
  : m_code (success)
  , m_lineNumber (0)
  , m_needsToBeChecked (true)
  , m_szWhat (0)
{
}

Error::Error (const Error& other)
  : m_code (other.m_code)
  , m_reasonText (other.m_reasonText)
  , m_sourceFileName (other.m_sourceFileName)
  , m_lineNumber (other.m_lineNumber)
  , m_needsToBeChecked (true)
  , m_szWhat (0)
{
  other.m_needsToBeChecked = false;
}

Error::~Error() noexcept
{
  /* If this goes off it means an error object was created but never tested */
  jassert (!m_needsToBeChecked);
}

Error& Error::operator= (const Error& other)
{
  m_code = other.m_code;
  m_reasonText = other.m_reasonText;
  m_sourceFileName = other.m_sourceFileName;
  m_lineNumber = other.m_lineNumber;
  m_needsToBeChecked = true;
  m_what = String::empty;
  m_szWhat = 0;

  other.m_needsToBeChecked = false;

  return *this;
}

Error::Code Error::code () const
{
  m_needsToBeChecked = false;
  return m_code;
}

bool Error::failed () const
{
  return code () != success;
}

bool Error::asBoolean () const
{
  return code () != success;
}

const String Error::getReasonText () const
{
  return m_reasonText;
}

const String Error::getSourceFilename () const
{
  return m_sourceFileName;
}

int Error::getLineNumber () const
{
  return m_lineNumber;
}

Error& Error::fail (const char* sourceFileName,
                    int lineNumber,
                    const String reasonText,
                    Code errorCode)
{
  jassert (m_code == success);
  jassert (errorCode != success);

  m_code = errorCode;
  m_reasonText = reasonText;
  m_sourceFileName = Debug::getFileNameFromPath (sourceFileName);
  m_lineNumber = lineNumber;
  m_needsToBeChecked = true;

  return *this;
}

Error& Error::fail (const char* sourceFileName,
                    int lineNumber,
                    Code errorCode)
{
  return fail (sourceFileName,
               lineNumber,
               getReasonTextForCode (errorCode), 
               errorCode);
}

void Error::reset ()
{
  m_code = success;
  m_reasonText = String::empty;
  m_sourceFileName = String::empty;
  m_lineNumber = 0;
  m_needsToBeChecked = true;
  m_what = String::empty;
  m_szWhat = 0;
}

void Error::willBeReported () const
{
  m_needsToBeChecked = false;
}

const char* Error::what () const noexcept
{
  if (!m_szWhat)
  {
// The application could not be initialized because sqlite was denied access permission
// The application unexpectedly quit because the exception 'sqlite was denied access permission at file ' was thrown
    m_what <<
      m_reasonText << " " <<
      TRANS("at file") << " '" <<
      m_sourceFileName << "' " <<
      TRANS("line") << " " <<
      String (m_lineNumber) << " " <<
      TRANS("with code") << " = " <<
      String (m_code);

    m_szWhat = (const char*)m_what.toUTF8();
  }

  return m_szWhat;
}

const String Error::getReasonTextForCode (Code code)
{
  String s;

  switch (code)
  {
  case success:         s=TRANS("the operation was successful"); break;

  case general:         s=TRANS("a general error occurred"); break;

  case canceled:        s=TRANS("the operation was canceled"); break;
  case exception:       s=TRANS("an exception was thrown"); break;
  case unexpected:      s=TRANS("an unexpected result was encountered"); break;
  case platform:        s=TRANS("a system exception was signaled"); break;

  case noMemory:        s=TRANS("there was not enough memory"); break;
  case noMoreData:      s=TRANS("the end of data was reached"); break;
  case invalidData:     s=TRANS("the data is corrupt or invalid"); break;
  case bufferSpace:     s=TRANS("the buffer is too small"); break;
  case badParameter:    s=TRANS("one or more parameters were invalid"); break;
  case assertFailed:    s=TRANS("an assertion failed"); break;

  case fileInUse:       s=TRANS("the file is in use"); break;
  case fileExists:      s=TRANS("the file exists"); break;
  case fileNoPerm:      s=TRANS("permission was denied"); break;
  case fileIOError:     s=TRANS("an I/O or device error occurred"); break;
  case fileNoSpace:     s=TRANS("there is no space left on the device"); break;
  case fileNotFound:    s=TRANS("the file was not found"); break;
  case fileNameInvalid: s=TRANS("the file name was illegal or malformed"); break;

  default:              s=TRANS("an unknown error code was received"); break;
  }

  return s;
}
