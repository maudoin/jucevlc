
#ifndef FONTSERIALIZATION
#define FONTSERIALIZATION

#include "juce.h"



juce::Typeface* loadFont( const void* data, size_t size);
juce::Typeface* loadFont( juce::String inPath);
juce::File beginWrite(juce::String const& destFilePath);
void outputBufferAsInlcudeFile(const void* data, size_t size, juce::String const& name, juce::String const& destFilePathCC, juce::String const& destFilePathH);
void serializeFont(juce::String const& fontName, juce::String const& outCPP, juce::String const& outH, juce::uint32 glyphCount=256);

#endif
