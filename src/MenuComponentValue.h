#ifndef MENU_COMPONENT_VALUE_H
#define MENU_COMPONENT_VALUE_H


#include <JuceHeader.h>
#include <variant>


using MenuComponentValue = std::variant<std::monostate, juce::Colour, double>;
struct SliderParams{double init, min, max;};
using MenuComponentParams = std::variant<std::monostate, juce::Colour, SliderParams>;

#endif //MENU_COMPONENT_VALUE_H