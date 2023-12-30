#ifndef MENU_COMPONENT_VALUE_H
#define MENU_COMPONENT_VALUE_H


#include "SettingSlider.h"
#include <JuceHeader.h>
#include <variant>

struct MenuComponentBack{};
using MenuComponentValue = std::variant<std::monostate, juce::Colour, double, MenuComponentBack>;
using MenuComponentParams = std::variant<std::monostate, juce::Colour, SettingSlider::Params>;

#endif //MENU_COMPONENT_VALUE_H