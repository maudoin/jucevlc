#ifndef IMAGES_THEME_H
#define IMAGES_THEME_H


namespace juce
{
	class Drawable;
	class Colour;
}
juce::Drawable* buildFolderBackImage(float hue);
juce::Drawable* buildFolderFrontImage(float hue);
juce::Drawable* buildBackImage(float hue);

juce::Colour getThemeBaseColor(float hue);
#endif //IMAGES_THEME_H