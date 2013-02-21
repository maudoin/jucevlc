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
class AbstractFileAction
{
public:
	virtual ~AbstractFileAction () { }
	virtual void operator() (MenuTreeItem& parent, juce::File const& file) = 0;
	virtual AbstractFileAction* clone() const = 0;
};
//==============================================================================
class MenuTreeItem
{
public:
	virtual ~MenuTreeItem(){}
    virtual void addAction(juce::String const& name, AbstractAction* action, const juce::Drawable* icon = nullptr) = 0;
    virtual void addFile(juce::String const& name, juce::File const& file_, AbstractFileAction* fileMethod_) = 0;
	virtual void addRootFiles(AbstractFileAction* fileMethod) = 0;
	virtual void addChildrenFiles(juce::File const& parent, AbstractFileAction* fileMethod, 
		int whatToLookFor = juce::File::findFilesAndDirectories|juce::File::ignoreHiddenFiles,
        const juce::String& wildCardPattern = "*",
        bool searchRecursively = false) = 0;
	virtual void focusItemAsMenuShortcut() = 0;
	virtual void focusParent() = 0;
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