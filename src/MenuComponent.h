#ifndef MENU_COMPONENT_H
#define MENU_COMPONENT_H


#include "AbstractMenu.h"

class MenuItem;
class MenuItemList;
class RecentMenuItemList;
//==============================================================================
class MenuComponent : public virtual juce::Component,
					public virtual AbstractMenu,
					private juce::ChangeListener,
					private juce::Slider::Listener
{
	std::unique_ptr<MenuItemList> menuList;
	std::unique_ptr<RecentMenuItemList> recentList;
	std::vector<std::unique_ptr<juce::Drawable>> m_iconImages;
	bool m_gradient;
	juce::ColourSelector m_colourSelector;
	juce::Slider m_slider;
public:
	using Value = std::variant<juce::Colour, int>;

	MenuComponent(bool gradient = true);
	virtual ~MenuComponent();

	void resized() override;
	void forceMenuRefresh() override;
	void  changeListenerCallback (ChangeBroadcaster*) override;
	void sliderValueChanged (Slider* slider) override;

	void paint (juce::Graphics& g) final;
	juce::Component* asComponent() final {return this;}
	juce::Component const* asComponent() const final {return this;}
	bool isShown() const final;
	void setShown(bool show) final;

	void menuItemSelected(int /*lastRowselected*/);
	void recentItemSelected(int /*lastRowselected*/);
	int preferredHeight()const final;

	void addMenuItem(juce::String const& name, AbstractMenuItem::ActionEffect actionEffect, AbstractAction action, AbstractMenuItem::Icon icon, MenuComponentParams const& params = {}) override;
	void addRecentMenuItem(juce::String const& name, AbstractMenuItem::ActionEffect actionEffect, AbstractAction action, AbstractMenuItem::Icon icon) override;

protected:
	enum class Mode{LIST, COLOR, SLIDER};
	void setMode(Mode mode);

	juce::Drawable const* getImage(AbstractMenuItem::Icon icon)const;

	void activateItem(MenuItem& item, bool isRecent);

};

#endif //MENU_COMPONENT_H