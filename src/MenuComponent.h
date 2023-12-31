#ifndef MENU_COMPONENT_H
#define MENU_COMPONENT_H


#include "AbstractMenu.h"
#include "SettingSlider.h"

class MenuItem;
class MenuItemList;
class RecentMenuItemList;
//==============================================================================
class MenuComponent : public virtual juce::Component,
					public virtual AbstractMenu,
					private juce::ChangeListener,
					private juce::Slider::Listener
{
public:
	using Value = std::variant<juce::Colour, int>;
	using UpdateBoundsCallback = std::function<void()>;

private:
	std::unique_ptr<MenuItemList> menuList;
	std::unique_ptr<RecentMenuItemList> recentList;
	std::vector<std::unique_ptr<juce::Drawable>> m_iconImages;
	juce::ColourSelector m_colourSelector;
	SettingSlider m_slider;
	UpdateBoundsCallback m_updateBoundsCallback;

public:
	MenuComponent();
	MenuComponent(UpdateBoundsCallback const& updateBoundsCallback);
	virtual ~MenuComponent();

	void resized() override;
	void forceMenuRefresh() override;
	void changeListenerCallback (ChangeBroadcaster*) override;
	void sliderValueChanged (juce::Slider* slider) override;

	juce::Component* asComponent() final {return this;}
	juce::Component const* asComponent() const final {return this;}
	bool isShown() const final;
	void setShown(bool show) final;

	void menuItemSelected(int /*lastRowselected*/);
	void recentItemSelected(int /*lastRowselected*/);
	int preferredWidth()const final;
	int preferredHeight()const final;

	void addMenuItem(juce::String const& name, AbstractMenuItem::ActionEffect actionEffect, AbstractAction const& action, AbstractMenuItem::Icon icon, MenuComponentParams const& params = {}) override;
	void addRecentMenuItem(juce::String const& name, AbstractMenuItem::ActionEffect actionEffect, AbstractAction const& action, AbstractMenuItem::Icon icon) override;

	void setScaleComponent(juce::Component* scaleComponent) override;

protected:
	enum class Mode{LIST, COLOR, SLIDER};
	void setMode(Mode mode);

	juce::Drawable const* getImage(AbstractMenuItem::Icon icon)const;

	void activateItem(MenuItem const& item, bool isRecent);

};

#endif //MENU_COMPONENT_H