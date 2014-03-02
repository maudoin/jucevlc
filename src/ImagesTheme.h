#ifndef IMAGES_THEME_H
#define IMAGES_THEME_H


namespace juce
{
	class Drawable;
}
juce::Drawable* buildFolderBackImage(float hue);
juce::Drawable* buildFolderFrontImage(float hue);
juce::Drawable* buildBackImage(float hue);

#endif //IMAGES_THEME_H