
#ifndef FONTSERIALIZATION
#define FONTSERIALIZATION

#include "juce.h"

Typeface* loadFont( String inPath)
{
	File fontFile;
	if (File::isAbsolutePath (inPath))
	{
		fontFile = File(inPath);
	}
	else
	{
		fontFile = File::getCurrentWorkingDirectory().getChildFile(inPath);
	}

	FileInputStream ins(fontFile);
	return (new CustomTypeface (ins));

}

void serializeFont(String fontName, String out, uint32 glyphCount=256)
{
	
	File fontFile;
	if (File::isAbsolutePath (out))
	{
		fontFile = File(out);
	}
	else
	{
		fontFile = File::getCurrentWorkingDirectory().getChildFile(out);
	}
	fontFile.replaceWithData (0,0);
		

	if (!fontFile.hasWriteAccess ())
	{
		printf ("initialise ERROR can't write to destination file: %s\n", fontFile.getFullPathName().toUTF8());
		return;
	}

	if (fontName == String::empty)
	{
		printf ("initialise ERROR no font name given\n");
		return;
	}


	printf ("Fserialize::serializeFont looking for font in system list [%s]\n", fontName.toUTF8());
	
	Array <Font> systemFonts;
	Font::findFonts (systemFonts);
	for (int i=0; i<systemFonts.size(); i++)
	{
		if (systemFonts[i].getTypeface()->getName() == fontName)
		{
			CustomTypeface customTypefacePlain;
			customTypefacePlain.setCharacteristics(systemFonts[i].getTypefaceName(), systemFonts[i].getAscent(),
                                      systemFonts[i].isBold(), systemFonts[i].isItalic(), ' ');

			customTypefacePlain.addGlyphsFromOtherTypeface (*(systemFonts[i].getTypeface()), 0, glyphCount);
			
			FileOutputStream streamPlain(fontFile);
			customTypefacePlain.writeToStream (streamPlain);
		}
	}

	printf ("Fserialize::serializeFont finished\n");

}

#endif
