#include "MenuComponent.h"
#include "FileSorter.h"
#include "Extensions.h"
#include "Icons.h"

#define SHORTCUTS_FILE "shortcuts.list"

using namespace std::placeholders;

void NullAbstractAction(AbstractMenuItem&){};


class MenuItem : public virtual AbstractMenuItem
{
protected:
	juce::String name;
	const juce::Drawable* icon;
	AbstractAction action;
	AbstractMenuItem::ActionEffect actionEffect;
	bool isShortcut;
public:

    MenuItem (juce::String const& name = {}, AbstractMenuItem::ActionEffect actionEffect = AbstractMenuItem::EXECUTE_ONLY, AbstractAction action = NullAbstractAction, const juce::Drawable* icon = nullptr, bool isShortcut = false)
	:name(name)
	,icon(icon)
	,action(action)
	,actionEffect(actionEffect)
	,isShortcut(isShortcut)
	{
	}

	virtual ~MenuItem()
	{
	}

	virtual const juce::Drawable* getIcon(){ return icon;}
	virtual juce::String const& getName(){ return name;}
	virtual void execute(AbstractMenuItem & m){ action(m);}
	virtual AbstractMenuItem::ActionEffect getActionEffect()const{ return actionEffect; }
	virtual AbstractAction const& getAction()const{ return action; }
	virtual bool isMenuShortcut(){ return isShortcut ;}

};



class TransparentListBox : public virtual juce::ListBox
{
public:
	TransparentListBox(const juce::String& componentName = {},
             juce::ListBoxModel* model = nullptr) : juce::ListBox(componentName, model)
	{
		setColour(backgroundColourId, juce::Colour());
	}
	void paint (juce::Graphics& /*g*/)
	{
	}
};


class MenuItemList : public juce::ListBoxModel
{
public:
	typedef std::function<void (int)> SelectionCallback;
private:
	TransparentListBox box;
	juce::Array<MenuItem> items;
	bool isShortcut;
	SelectionCallback listBoxSelectionCallback;
public:

	MenuItemList(juce::String const& name,SelectionCallback listBoxSelectionCallback, bool isShortcut)
		:box(name, this)
		,isShortcut(isShortcut)
		,listBoxSelectionCallback(listBoxSelectionCallback)
	{
	}
	virtual ~MenuItemList()
	{
	}


	juce::ListBox * getListBox()
	{
		return &box;
	}

    // implements the ListBoxModel method
    int getNumRows()
    {
        return items.size();
    }

    // implements the ListBoxModel method
    void paintListBoxItem (int rowNumber,
                           juce::Graphics& g,
                           int width, int height,
                           bool rowIsSelected)
    {
        if (rowNumber < 0 || rowNumber>= items.size())
		{
			return;
		}
		MenuItem& item = items.getReference(rowNumber);
		paintMenuItem(g, width, height, rowIsSelected, item.getName(), item.getIcon(), isShortcut);
    }

	static void paintMenuItem(juce::Graphics& g, int width, int height,
		bool isItemSelected, juce::String const& name, const juce::Drawable* d, bool isShortcut)
	{

		float fontSize =  1.1f*height;

		float hborder = height/8.f;

		float iconhborder = height/4.f;
		int iconSize = height;


		float xBounds = iconSize + iconhborder;

		juce::Rectangle<float> borderBounds(xBounds, hborder, width-xBounds, height-2.f*hborder);

		g.setColour (juce::Colours::black);

		if (d != nullptr)
		{
				d->drawWithin (g, juce::Rectangle<float> (iconhborder, 0.0f, (float)iconSize, (float)iconSize),
								juce::RectanglePlacement::centred | juce::RectanglePlacement::onlyReduceInSize, 1.0f);
		}

		juce::Font f = g.getCurrentFont().withHeight(fontSize);
		//f.setTypefaceName(/*"Forgotten Futurist Shadow"*/"Times New Roman");
		f.setStyleFlags(juce::Font::plain);
		g.setFont(f);

		g.setColour (juce::Colours::white);

		int xText = iconSize + 2*(int)iconhborder;
		g.drawFittedText (name,
							xText, 0, width - xText, height,
							juce::Justification::centredLeft,
							1, //1 line
							1.f//no h scale
							);
	}

    void selectedRowsChanged (int lastRowselected)
	{
		listBoxSelectionCallback(lastRowselected);
	}


    virtual void add(juce::String const& name, AbstractMenuItem::ActionEffect actionEffect, AbstractAction action, const juce::Drawable* icon)
	{
		items.add(MenuItem(name, actionEffect, action, icon, isShortcut));
        box.updateContent();
	}

	MenuItem* getItem(int rowNumber)
	{
		if (rowNumber < 0 || rowNumber>= getNumRows())
		{
			return nullptr;
		}
		return &(items.getReference(rowNumber));
	}

	MenuItem* getSelectedItem()
	{
		return getItem(box.getSelectedRow());
	}
	void removeItemsAfter(int rowNumber)
	{
		//menu is likely to have changed
		items.removeRange(rowNumber +1, items.size()-1-rowNumber);
		box.updateContent();
	}
	void clear()
	{
		items.clear();
		box.updateContent();
	}

	void selectLastItem(juce::NotificationType type)
	{
		int index = getNumRows()-1;
		juce::SparseSet<int> set;
		set.addRange (juce::Range<int>(index, index));
		getListBox()->setSelectedRows(set, type);
	}
};
MenuComponent::MenuComponent(bool const gradient)
	: menuList(new MenuItemList("MenuList", std::bind(&MenuComponent::menuItemSelected, this, _1),false))
	, recentList(new MenuItemList("RecentList", std::bind(&MenuComponent::recentItemSelected, this, _1),true))
    , itemImage               (juce::Drawable::createFromImageData (Icons::atom_svg, Icons::atom_svgSize))
    , folderImage             (juce::Drawable::createFromImageData (Icons::openmenu_svg, Icons::openmenu_svgSize))
    , playlistImage           (juce::Drawable::createFromImageData (Icons::playlist_svg, Icons::playlist_svgSize))
    , folderShortcutImage     (juce::Drawable::createFromImageData (Icons::openshort_svg, Icons::openshort_svgSize))
    , hideFolderShortcutImage (juce::Drawable::createFromImageData (Icons::hideopen_svg, Icons::hideopen_svgSize))
    , audioImage              (juce::Drawable::createFromImageData (Icons::soundon_svg, Icons::soundon_svgSize))
    , displayImage            (juce::Drawable::createFromImageData (Icons::image_svg, Icons::image_svgSize))
    , subtitlesImage          (juce::Drawable::createFromImageData (Icons::subtitles_svg, Icons::subtitles_svgSize))
    , likeAddImage            (juce::Drawable::createFromImageData (Icons::likeadd_svg, Icons::likeadd_svgSize))
	, likeRemoveImage         (juce::Drawable::createFromImageData (Icons::likeremove_svg, Icons::likeremove_svgSize))
	, backImage               (juce::Drawable::createFromImageData (Icons::backCircle_svg, Icons::backCircle_svgSize))
	, m_gradient(gradient)
{
    addAndMakeVisible (recentList->getListBox());
    addAndMakeVisible (menuList->getListBox());
	setOpaque(true);

}
MenuComponent::~MenuComponent()
{
}

void MenuComponent::paint (juce::Graphics& g)
{
	float w = (float)asComponent()->getWidth();
	float h = (float)asComponent()->getHeight();
	if(m_gradient)
	{
		float const roundness = 0.01f*asComponent()->getParentWidth();
		static const juce::Colour color = juce::Colours::darkgrey.darker().darker().withAlpha(0.9f);
		g.setColour (color);
		g.fillRoundedRectangle(0, 0, w, h, roundness);
	}
	else
	{
		g.fillRect(0.f, 0.f, w, h);
	}

}
void MenuComponent::resized()
{
	menuList->getListBox()->setRowHeight((int)getItemHeight());
	recentList->getListBox()->setRowHeight((int)getItemHeight());

	int recentSize = recentList->getNumRows()*(int)getItemHeight()+2;
	if(recentSize > getHeight()/2)
	{
		recentSize = getHeight()/2;
	}

	recentList->getListBox()->setBounds(0, 0, getWidth(), recentSize);
	menuList->getListBox()->setBounds((int)getItemHeight(), recentSize, getWidth()-(int)getItemHeight(), getHeight()-recentSize);
	Component::resized();
}

void MenuComponent::forceRecentItem(int row)
{
	MenuItem* item = recentList->getItem(row);
    if (item != nullptr)
	{
		MenuItem selected(*item);
		recentList->removeItemsAfter(row);
		recentList->getListBox()->setSelectedRows(juce::SparseSet<int>());

		menuList->clear();
		selected.execute(selected);

		resized();
	}
}
void MenuComponent::recentItemSelected(int /*lastRowselected*/)
{
	forceRecentItem(recentList->getListBox()->getSelectedRow());
}

void MenuComponent::menuItemSelected (int /*lastRowselected*/)
{
	MenuItem* item = menuList->getSelectedItem();
    if (item != nullptr)
	{
		switch(item->getActionEffect())
		{
			case AbstractMenuItem::STORE_AND_OPEN_CHILDREN:
			{
				recentList->add(item->getName(), item->getActionEffect(), item->getAction(), getBackImage());

				MenuItem copy = *item;
				menuList->clear();
				copy.execute(copy);

				resized();
				break;
			}
			case AbstractMenuItem::EXECUTE_ONLY:
			{
				item->execute(*item);

				resized();
				break;
			}
			case AbstractMenuItem::REFRESH_MENU:
			{
				item->execute(*item);

				forceMenuRefresh();
				break;
			}
		}
		menuList->getListBox()->setSelectedRows(juce::SparseSet<int>());
	}
}

void MenuComponent::forceMenuRefresh()
{
	forceRecentItem(recentList->getNumRows()-1);
}

void MenuComponent::addMenuItem(juce::String const& name, AbstractMenuItem::ActionEffect actionEffect, AbstractAction action, const juce::Drawable* icon, bool shortcut)
{
	this->addMenuItem(shortcut?recentList.get():menuList.get(), name, actionEffect, action, icon);
}
void MenuComponent::addMenuItem(MenuItemList* target, juce::String const& name, AbstractMenuItem::ActionEffect actionEffect, AbstractAction action, const juce::Drawable* icon)
{
	target->add(name, actionEffect, action, icon);
}


juce::Drawable const* MenuComponent::getIcon(juce::String const& e)
{
	if(extensionMatch(Extensions::get().videoExtensions(), e))
	{
		return displayImage.get();
	}
	if(extensionMatch(Extensions::get().playlistExtensions(), e))
	{
		return playlistImage.get();
	}
	if(extensionMatch(Extensions::get().subtitlesExtensions(), e))
	{
		return subtitlesImage.get();
	}
	return nullptr;
}
juce::Drawable const* MenuComponent::getIcon(juce::File const& f)
{
	if(f.isDirectory())
	{
		return folderImage.get();
	}
	return getIcon(f.getFileExtension());
}

