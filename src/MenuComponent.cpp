#include "MenuComponent.h"
#include "FileSorter.h"
#include "Extensions.h"
#include "Icons.h"

#define SHORTCUTS_FILE "shortcuts.list"

using namespace std::placeholders;

void NullAbstractAction(MenuComponentValue const&){};


class MenuItem
{
protected:
	juce::String text;
	const juce::Drawable* icon;
	AbstractAction action;
	AbstractMenuItem::ActionEffect actionEffect;
	MenuComponentParams params;
public:

    MenuItem (juce::String const& text = {},
				AbstractMenuItem::ActionEffect actionEffect = AbstractMenuItem::EXECUTE_ONLY,
				AbstractAction const& action = NullAbstractAction,
				const juce::Drawable* icon = nullptr,
				MenuComponentParams const& params = {})
	: text(text)
	, icon(icon)
	, action(action)
	, actionEffect(actionEffect)
	, params(params)
	{}

	const juce::Drawable* getIcon()const{ return icon;}
	juce::String const& getText() const { return text;}
	void execute(MenuComponentValue const& value)const{ action(value);}
	AbstractMenuItem::ActionEffect getActionEffect()const{ return actionEffect; }
	AbstractAction const& getAction()const{ return action; }
	MenuComponentParams const& getParams()const{ return params; }

};



class TransparentListBox : public juce::ListBox, public AppProportionnalComponent
{
public:
	TransparentListBox(const juce::String& componentName = {},
             juce::ListBoxModel* model = nullptr) : juce::ListBox(componentName, model)
	{
		setColour(backgroundColourId, juce::Colour());
		addComponentListener(this);
	}
	void paint (juce::Graphics&) final
	{
	}
	float getFontItemScaleRatio() const final {return 1.1f;}
};


class MenuItemList : public juce::ListBoxModel
{
public:
	typedef std::function<void (int)> SelectionCallback;
private:
	TransparentListBox box;
	juce::Array<MenuItem> items;
	SelectionCallback listBoxSelectionCallback;
	int m_preferredWidth;
public:

	MenuItemList(juce::String const& name,SelectionCallback listBoxSelectionCallback)
		: box(name, this)
		, listBoxSelectionCallback(listBoxSelectionCallback)
		, m_preferredWidth(0)
	{
	}
	virtual ~MenuItemList() = default;

	TransparentListBox& getListBox()
	{
		return box;
	}

    // implements the ListBoxModel method
    int getNumRows() final
    {
        return items.size();
    }

    // implements the ListBoxModel method
    void paintListBoxItem (int rowNumber,
                           juce::Graphics& g,
                           int width, int height,
                           bool /*rowIsSelected*/)
    {
        if (rowNumber >= 0 && rowNumber < items.size())
		{
			MenuItem& item = items.getReference(rowNumber);
			paintMenuItem(g, width, height, item.getText(), item.getIcon(), box.getFont());
		}
    }
	static float iconSizeFor(int height){return height;}
	static float hborderFor(int height){return height/8.f;}
	static float iconhborderFor(int height){return  height/4.f;}
	static int getWidth(int height, juce::Font const& font, juce::String const& text)
	{
		return iconSizeFor(height)+iconSizeFor(height)+iconSizeFor(height)+font.getStringWidth(text);
	}

	static void paintMenuItem(juce::Graphics& g, int width, int height,
		juce::String const& text, const juce::Drawable* d, juce::Font const& font)
	{


		float hborder = hborderFor(height);

		float iconhborder = iconhborderFor(height);
		float iconSize = iconSizeFor(height);


		float xBounds = iconSize + iconhborder;

		juce::Rectangle<float> borderBounds(xBounds, hborder, width-xBounds, height-2.f*hborder);

		g.setColour (juce::Colours::black);

		if (d != nullptr)
		{
				d->drawWithin (g, juce::Rectangle<float> (iconhborder, 0.0f, iconSize, iconSize),
								juce::RectanglePlacement::centred | juce::RectanglePlacement::onlyReduceInSize, 1.0f);
		}

		g.setFont(font);

		g.setColour (juce::Colours::white);

		int xText = iconSize + 2*(int)iconhborder;
		g.drawFittedText (text,
							xText, 0, width - xText, height,
							juce::Justification::centredLeft,
							1, //1 line
							1.f//no h scale
							);
	}

    void selectedRowsChanged(int lastRowselected)
	{
		listBoxSelectionCallback(lastRowselected);
	}


    virtual void add(juce::String const& name, AbstractMenuItem::ActionEffect actionEffect,
					AbstractAction const& action, const juce::Drawable* icon, MenuComponentParams const& params)
	{
		items.add(MenuItem(name, actionEffect, action, icon, params));
        contentChanged();
	}

	MenuItem const* getItem(int rowNumber) const
	{
		if (rowNumber < 0 || rowNumber>= items.size())
		{
			return nullptr;
		}
		return &(items.getReference(rowNumber));
	}

	MenuItem const* getSelectedItem() const
	{
		return getItem(box.getSelectedRow());
	}
	void clear()
	{
		items.clear();
		contentChanged();
	}
	void contentChanged()
	{
		m_preferredWidth = 0;
		for(auto const& item: items)
		{
			m_preferredWidth = std::max(m_preferredWidth, getWidth(box.getRowHeight(), box.getFont(), item.getText()));
		}
        box.updateContent();
	}
	int preferredWidth()const
	{
		return m_preferredWidth;
	}

};

class RecentMenuItemList : public juce::ListBoxModel
{
public:
	typedef std::function<void (int)> SelectionCallback;
private:
	TransparentListBox box;
	juce::Array<MenuItem> items;
	SelectionCallback listBoxSelectionCallback;
	int m_preferredWidth;
public:

	RecentMenuItemList(juce::String const& name,SelectionCallback listBoxSelectionCallback)
		: box(name, this)
		, listBoxSelectionCallback(listBoxSelectionCallback)
		, m_preferredWidth(0)
	{
	}
	virtual ~RecentMenuItemList() = default;


	TransparentListBox& getListBox()
	{
		return box;
	}

    // implements the ListBoxModel method
    int getNumRows() override
    {
        return items.size()<=1?0:1;
    }

	juce::String getText()const
	{
		juce::String path;
        if (!items.isEmpty())
		{
			for(int i=std::max(1, items.size()-2);i<items.size();++i)
			{
				if(!path.isEmpty())
				{
					path = path+" > "+items[i].getText();
				}
				else
				{
					path = items[i].getText();
				}
			}
		}
		return path;
	}

    // implements the ListBoxModel method
    void paintListBoxItem (int /*rowNumber*/,
                           juce::Graphics& g,
                           int width, int height,
                           bool /*rowIsSelected*/) override
    {
        if (MenuItem const* item = getLastItem())
		{
	 		g.setGradientFill (juce::ColourGradient(juce::Colours::lightgrey,
	 											0, 0.f,
	 											juce::Colours::black.withAlpha(0.f),
	 											width, 0.f,
	 											true));

	 		g.drawLine(0, height, (float)width, height, 2.f);

			MenuItemList::paintMenuItem(g, width, height, getText(), item->getIcon(), box.getFont());
		}
    }

    void selectedRowsChanged (int lastRowselected) override
	{
		listBoxSelectionCallback(lastRowselected);
	}

    virtual void add(juce::String const& name, AbstractMenuItem::ActionEffect actionEffect,
	                 AbstractAction action, const juce::Drawable* icon,
				     MenuComponentParams const& params = {})
	{
		items.add(MenuItem(name, actionEffect, action, icon, params));
        contentChanged();
	}

	void contentChanged()
	{
		m_preferredWidth = box.getRowHeight() + (items.isEmpty() ? 0 : MenuItemList::getWidth(box.getRowHeight(), box.getFont(), getText()));
        box.updateContent();
	}
	int preferredWidth()const
	{
		return m_preferredWidth;
	}

	MenuItem const* getLastItem() const
	{
		return (items.isEmpty()) ? nullptr : &(items.getReference(items.size()-1));
	}
	void unstack()
	{
		if(items.size()>1)
		{
			items.removeLast(1);
		}
		contentChanged();
	}
};

MenuComponent::MenuComponent()
 : MenuComponent([this]{this->resized();})
{}

MenuComponent::MenuComponent(UpdateBoundsCallback const& updateBoundsCallback)
	: menuList(std::make_unique<MenuItemList>("MenuList", std::bind(&MenuComponent::menuItemSelected, this, _1)))
	, recentList(std::make_unique<RecentMenuItemList>("RecentList", std::bind(&MenuComponent::recentItemSelected, this, _1)))
	, m_iconImages(EnumSize<AbstractMenuItem::Icon>::value)
	, m_colourSelector(juce::ColourSelector::showColourspace, 0, 0)
	, m_updateBoundsCallback(updateBoundsCallback)
{
    m_iconImages[AbstractMenuItem::Icon::None]                  = nullptr;
    m_iconImages[AbstractMenuItem::Icon::Check]                 = juce::Drawable::createFromImageData (Icons::check_svg, Icons::check_svgSize);
    m_iconImages[AbstractMenuItem::Icon::Folder]                = juce::Drawable::createFromImageData (Icons::layers_svg, Icons::layers_svgSize);
    m_iconImages[AbstractMenuItem::Icon::FolderShortcut]        = juce::Drawable::createFromImageData (Icons::star_svg, Icons::star_svgSize);
    m_iconImages[AbstractMenuItem::Icon::FolderShortcutOutline] = juce::Drawable::createFromImageData (Icons::star_outline_svg, Icons::star_outline_svgSize);
    m_iconImages[AbstractMenuItem::Icon::Audio]                 = juce::Drawable::createFromImageData (Icons::audio_svg, Icons::audio_svgSize);
    m_iconImages[AbstractMenuItem::Icon::Display]               = juce::Drawable::createFromImageData (Icons::video_svg, Icons::video_svgSize);
    m_iconImages[AbstractMenuItem::Icon::Subtitles]             = juce::Drawable::createFromImageData (Icons::subtitles_svg, Icons::subtitles_svgSize);
	m_iconImages[AbstractMenuItem::Icon::Back]                  = juce::Drawable::createFromImageData (Icons::cancel_svg, Icons::cancel_svgSize);
	m_iconImages[AbstractMenuItem::Icon::AddAll]                = juce::Drawable::createFromImageData (Icons::layers_svg, Icons::layers_svgSize);
	m_iconImages[AbstractMenuItem::Icon::PlayAll]               = juce::Drawable::createFromImageData (Icons::play_svg, Icons::play_svgSize);
	m_iconImages[AbstractMenuItem::Icon::Settings]              = juce::Drawable::createFromImageData (Icons::settings_svg, Icons::settings_svgSize);
	m_iconImages[AbstractMenuItem::Icon::Sliders]               = juce::Drawable::createFromImageData (Icons::sliders_svg, Icons::sliders_svgSize);
	m_iconImages[AbstractMenuItem::Icon::Download]              = juce::Drawable::createFromImageData (Icons::download_svg, Icons::download_svgSize);
	m_iconImages[AbstractMenuItem::Icon::Exit]            	    = juce::Drawable::createFromImageData (Icons::exit_svg, Icons::exit_svgSize);

    addAndMakeVisible (recentList->getListBox());
    addAndMakeVisible (menuList->getListBox());
    addChildComponent (m_colourSelector);
    addChildComponent (m_slider);
	setOpaque(true);
	m_colourSelector.addChangeListener (this);
	m_slider.addListener(this);

}
MenuComponent::~MenuComponent()
{
}

void MenuComponent::setScaleComponent(juce::Component* scaleComponent)
{
	AppProportionnalComponent::setScaleComponent(scaleComponent);
	m_slider.setScaleComponent(scaleComponent);
	menuList->getListBox().setScaleComponent(scaleComponent);
	recentList->getListBox().setScaleComponent(scaleComponent);
}

juce::Drawable const* MenuComponent::getImage(AbstractMenuItem::Icon icon)const
{
	std::unique_ptr<juce::Drawable> const&ptr = m_iconImages[icon];
	return ptr?ptr.get():nullptr;
}

bool MenuComponent::isShown() const
{
	return isVisible();
}
void MenuComponent::setShown(bool show)
{
	setVisible(show);
}

void MenuComponent::resized()
{
	menuList->getListBox().setRowHeight((int)getItemHeight());
	recentList->getListBox().setRowHeight((int)getItemHeight());

	int recentSize = recentList->getNumRows()*(int)getItemHeight()+2;
	if(recentSize > getHeight()/2)
	{
		recentSize = getHeight()/2;
	}

	recentList->getListBox().setBounds(0, 0, getWidth(), recentSize);
	menuList->getListBox().setBounds(0, recentSize, getWidth(), getHeight()-recentSize);
	m_colourSelector.setBounds(0, recentSize, getWidth(), getHeight()-recentSize);
	m_slider.setBounds((int)getItemHeight(), recentSize, getWidth()-2*(int)getItemHeight(), (int)getItemHeight());
	Component::resized();
}

void MenuComponent::activateItem(MenuItem const& item, bool isRecent)
{

	switch(item.getActionEffect())
	{
		case AbstractMenuItem::STORE_AND_OPEN_CHILDREN:
		{
			MenuItem copy = item;
			setMode(Mode::LIST);

			if(!isRecent)
			{
				recentList->add(item.getText(), item.getActionEffect(), item.getAction(), getImage(AbstractMenuItem::Icon::Back));
			}

			menuList->clear();
			copy.execute(isRecent?MenuComponentValue(*this, MenuComponentValue::Back{}):MenuComponentValue(*this, {}));

			m_updateBoundsCallback();
			break;
		}
		case AbstractMenuItem::STORE_AND_OPEN_COLOR:
		{
			setMode(Mode::COLOR);
			if(!isRecent)
			{
				recentList->add(item.getText(), item.getActionEffect(), item.getAction(), getImage(AbstractMenuItem::Icon::Back));
			}

			menuList->clear();

			m_updateBoundsCallback();
			break;
		}
		case AbstractMenuItem::STORE_AND_OPEN_SLIDER:
		{
			setMode(Mode::SLIDER);
			m_slider.setup(std::get<SettingSlider::Params>(item.getParams()));
			if(!isRecent)
			{
				recentList->add(item.getText(), item.getActionEffect(), item.getAction(), getImage(AbstractMenuItem::Icon::Back));
			}

			menuList->clear();

			m_updateBoundsCallback();
			if(auto* parent = getParentComponent())parent->resized();
			break;
		}
		case AbstractMenuItem::EXECUTE_ONLY:
		{
			item.execute({*this, {}});

			m_updateBoundsCallback();
			break;
		}
		case AbstractMenuItem::REFRESH_MENU:
		{
			item.execute({*this, {}});

			forceMenuRefresh();
			break;
		}
	}
	menuList->getListBox().setSelectedRows(juce::SparseSet<int>());
}
void MenuComponent::recentItemSelected(int lastRowselected)
{
	if(lastRowselected >= 0)
	{
		recentList->unstack();
		if(auto const*item = recentList->getLastItem())
		{
			activateItem(*item, true);
			recentList->getListBox().setSelectedRows(juce::SparseSet<int>());
		}
	}
}

void MenuComponent::setMode(Mode mode)
{
	m_colourSelector.setVisible(mode == Mode::COLOR);
	m_slider.setVisible(mode == Mode::SLIDER);
	menuList->getListBox().setVisible(mode == Mode::LIST);
}

void MenuComponent::changeListenerCallback (ChangeBroadcaster* broadcaster)
{
	if( MenuItem const* item = recentList->getLastItem() )
	{
		if( item->getActionEffect() == AbstractMenuItem::STORE_AND_OPEN_COLOR &&  m_colourSelector.isVisible() && broadcaster == &m_colourSelector )
		{
			item->execute({*this, m_colourSelector.getCurrentColour()});
		}
	}
}

void MenuComponent::sliderValueChanged (Slider* slider)
{
	if( MenuItem const* item = recentList->getLastItem() )
	{
		if( item->getActionEffect() == AbstractMenuItem::STORE_AND_OPEN_SLIDER && m_slider.isVisible() && m_slider.is(slider))
		{
			item->execute({*this, m_slider.getValue()});
		}
	}
}

void MenuComponent::menuItemSelected (int /*lastRowselected*/)
{
	MenuItem const* item = menuList->getSelectedItem();
    if (item != nullptr)
	{
		activateItem(*item, false);
	}
}

void MenuComponent::forceMenuRefresh()
{
	MenuItem const* item = recentList->getLastItem();
    if (item != nullptr)
	{
		MenuItem itemCopy = *item;
		recentList->getListBox().setSelectedRows(juce::SparseSet<int>());
		activateItem(itemCopy, true);
	}
}

void MenuComponent::addMenuItem(juce::String const& name,
	AbstractMenuItem::ActionEffect actionEffect, AbstractAction const& action,
	AbstractMenuItem::Icon icon, MenuComponentParams const& params)
{
	menuList->add(name, actionEffect, action, getImage(icon), params);
	menuList->getListBox().setSelectedRows(juce::SparseSet<int>());
}
void MenuComponent::addRecentMenuItem(juce::String const& name, AbstractMenuItem::ActionEffect actionEffect, AbstractAction const& action, AbstractMenuItem::Icon icon)
{
	recentList->add(name, actionEffect, action, getImage(icon));
	recentList->getListBox().setSelectedRows(juce::SparseSet<int>());
}


int MenuComponent::preferredWidth()const
{
	return std::max(recentList->preferredWidth(),
				menuList->preferredWidth());
}


int MenuComponent::preferredHeight()const
{
	float rowHeight = getItemHeight();

	//margin
	int h = rowHeight/4;

	//recent
	h += recentList->getNumRows()*rowHeight;

	if(auto *item = recentList->getLastItem())
	{
		//current content
		switch(item->getActionEffect())
		{
			case AbstractMenuItem::STORE_AND_OPEN_CHILDREN:
			{
	 			h += menuList->getNumRows()*rowHeight;
				break;
			}
			case AbstractMenuItem::STORE_AND_OPEN_COLOR:
			{
	 			h += 4*rowHeight;
				break;
			}
			case AbstractMenuItem::STORE_AND_OPEN_SLIDER:
			{
	 			h += rowHeight;
				break;
			}
			case AbstractMenuItem::EXECUTE_ONLY:
			case AbstractMenuItem::REFRESH_MENU:
				break;
		}
	}
	return h;
}