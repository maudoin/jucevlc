#ifndef MENU_TREE_H
#define MENU_TREE_H


#include "juce.h"
#include "AppProportionnalComponent.h"
	
//==============================================================================
class MenuTreeItem;
class AbstractAction
{
public:
	virtual ~AbstractAction () { }
	virtual void operator() (MenuTreeItem& parent) = 0;
};
//==============================================================================
class MenuTreeItem
{
public:
	virtual ~MenuTreeItem(){}
    virtual MenuTreeItem* addAction(juce::String const& name, AbstractAction* action, const juce::Drawable* icon = nullptr) = 0;
	virtual void focusItemAsMenuShortcut() = 0;
	virtual MenuTreeItem* getMenuTreeItemParent() = 0;
	virtual void forceSelection(bool force = true) = 0;
	virtual void forceParentSelection(bool force = true) = 0;
	virtual bool isMenuShortcut() = 0;
};

//==============================================================================
class MenuTree : public virtual juce::TreeView, public AppProportionnalComponent
{
	AbstractAction* rootAction;
    juce::Drawable const* itemImage;
    juce::Drawable const* folderImage;
    juce::Drawable const* folderShortcutImage;
public:
	MenuTree();
	virtual ~MenuTree();
	virtual void refresh();
	void setInitialMenu();

	void resized();
	void paint (juce::Graphics& g);
	
	void setRootAction(AbstractAction* rootAction_){rootAction=rootAction_;refresh();}
	
	void setItemImage(juce::Drawable const* itemImage_){itemImage=itemImage_;}
	void setFolderImage(juce::Drawable const* folderImage_){folderImage=folderImage_;}
	void setFolderShortcutImage(juce::Drawable const* folderShortcutImage_){folderShortcutImage=folderShortcutImage_;}
	juce::Drawable const* getItemImage() const { return itemImage; };
	juce::Drawable const* getFolderImage() const { return folderImage; };
	juce::Drawable const* getFolderShortcutImage() const { return folderShortcutImage; };
};

#endif //MENU_TREE_H