

#include "FontSerialization.h"



juce::Typeface* loadFont( const void* data, size_t size)
{

	juce::MemoryInputStream ins(data, size, false);
	return (new juce::CustomTypeface (ins));
	
}
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
juce::File beginWrite(juce::String const& destFilePath)
{
	juce::File fontFile;
	if (juce::File::isAbsolutePath (destFilePath))
	{
		fontFile = juce::File(destFilePath);
	}
	else
	{
		fontFile = juce::File::getCurrentWorkingDirectory().getChildFile(destFilePath);
	}

	if (fontFile.hasWriteAccess())
	{
		fontFile.replaceWithData (0,0);
	}
	
	return fontFile;
}
void outputBufferAsInlcudeFile(const void* data, size_t size, juce::String const& name, juce::String const& destFilePathCC, juce::String const& destFilePathH)
{
	juce::File fontFile = beginWrite(destFilePathCC);
	if (!fontFile.hasWriteAccess ())
	{
		printf ("initialise ERROR can't write to destination file: %s\n", fontFile.getFullPathName().toUTF8());
		return;
	}
	juce::File hFile = beginWrite(destFilePathH);
	if (!hFile.hasWriteAccess ())
	{
		printf ("initialise ERROR can't write to destination file: %s\n", hFile.getFullPathName().toUTF8());
		return;
	}
	
	juce::FileOutputStream cppStream(fontFile);
	cppStream << "#include \"" << hFile.getFileName() << "\"\n";
	juce::String line1;
    line1 << "const unsigned char " << name << "_resource[] = { ";
			
	cppStream << line1;

    juce::MemoryOutputStream out (65536);
    int charsOnLine = line1.length();

    for (size_t j = 0; j < size; ++j)
    {
        const int num = ((int) ((unsigned char*) data)[j]);
        out << num << ',';

        charsOnLine += 2;
        if (num >= 10)
            ++charsOnLine;
        if (num >= 100)
            ++charsOnLine;

        if (charsOnLine >= 200)
        {
            charsOnLine = 0;
            out << '\n';
        }
    }

    out << (char) 0;

    cppStream
        << (const char*) out.getData() << "0,0};\n"
		<< "const char *" << name << "Data = (const char*)"<< name<< "_resource;\n"
        << "const int " << name << "Size = " << (int) size<< ";\n\n";

	///////////////////////////////////////////////////
	juce::FileOutputStream hStream(hFile);
	hStream << "#ifndef _"<<name<<"_H_\n"
		<< "#define _"<<name<<"_H_\n\n"
		<< "extern const char * " << name << "Data;\n"
        << "extern const int " << name << "Size;\n\n"
		<< "#endif //_"<<name<<"_H_\n";
}
void serializeFont(juce::String const& fontName, juce::String const& outCPP, juce::String const& outH, juce::uint32 glyphCount)
{
	
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
			

			juce::MemoryOutputStream streamPlain(65536);
			customTypefacePlain.writeToStream (streamPlain);

			const void* data = streamPlain.getData();
			size_t size = streamPlain.getDataSize();
			juce::String name = fontName.replaceCharacter(' ', '_');

			outputBufferAsInlcudeFile(data, size, name, outCPP, outH);
		}
	}


}

