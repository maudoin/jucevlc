#ifndef MENU_COMPONENT_VALUE_H
#define MENU_COMPONENT_VALUE_H


#include "SettingSlider.h"
#include <JuceHeader.h>
#include <variant>

class AbstractMenu;
struct MenuComponentValue
{
    struct Back{};
    MenuComponentValue(AbstractMenu& menu, std::variant<std::monostate, juce::Colour, double, Back>&& value)
     : m_menu(menu)
     , m_value(std::move(value))
    {}
    AbstractMenu& menu()const{return m_menu;}
    template<typename T>
    T const* value() const{return std::get_if<T>(&m_value);}
private:
    AbstractMenu& m_menu;
    std::variant<std::monostate, juce::Colour, double, Back> m_value;
};
using MenuComponentParams = std::variant<std::monostate, juce::Colour, SettingSlider::Params>;

#endif //MENU_COMPONENT_VALUE_H