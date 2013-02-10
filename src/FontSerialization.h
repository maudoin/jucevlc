
#ifndef FONTSERIALIZATION
#define FONTSERIALIZATION

#include "juce.h"

juce::Typeface* loadFont( juce::String inPath)
{
	juce::File fontFile;
	if (juce::File::isAbsolutePath (inPath))
	{
		fontFile = juce::File(inPath);
	}
	else
	{
		fontFile = juce::File::getCurrentWorkingDirectory().getChildFile(inPath);
	}

	juce::FileInputStream ins(fontFile);
	return (new juce::CustomTypeface (ins));

}

void serializeFont(juce::String fontName, juce::String out, juce::uint32 glyphCount=256)
{
	
	juce::File fontFile;
	if (juce::File::isAbsolutePath (out))
	{
		fontFile = juce::File(out);
	}
	else
	{
		fontFile = juce::File::getCurrentWorkingDirectory().getChildFile(out);
	}
	fontFile.replaceWithData (0,0);
		

	if (!fontFile.hasWriteAccess ())
	{
		printf ("initialise ERROR can't write to destination file: %s\n", fontFile.getFullPathName().toUTF8());
		return;
	}

	if (fontName == juce::String::empty)
	{
		printf ("initialise ERROR no font name given\n");
		return;
	}


	printf ("Fserialize::serializeFont looking for font in system list [%s]\n", fontName.toUTF8());
	
	juce::Array <juce::Font> systemFonts;
	juce::Font::findFonts (systemFonts);
	for (int i=0; i<systemFonts.size(); i++)
	{
		if (systemFonts[i].getTypeface()->getName() == fontName)
		{
			juce::CustomTypeface customTypefacePlain;
			customTypefacePlain.setCharacteristics(systemFonts[i].getTypefaceName(), systemFonts[i].getAscent(),
                                      systemFonts[i].isBold(), systemFonts[i].isItalic(), ' ');

			customTypefacePlain.addGlyphsFromOtherTypeface (*(systemFonts[i].getTypeface()), 0, glyphCount);
			
			juce::FileOutputStream streamPlain(fontFile);
			customTypefacePlain.writeToStream (streamPlain);
		}
	}

	printf ("Fserialize::serializeFont finished\n");

}

#endif
