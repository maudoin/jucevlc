
#include "AbstractMenu.h"
#include "Icons.h"
#include "FileSorter.h"
#include "Extensions.h"


AbstractMenu::AbstractMenu()
{
}

void AbstractMenu::listShortcuts(MenuComponentValue const&, FileMethod const& fileMethod, juce::StringArray const& shortcuts)
{
	for(int i=0;i<shortcuts.size();++i)
	{
		juce::File path(shortcuts[i]);
		juce::String driveRoot = path.getFullPathName().upToFirstOccurrenceOf(juce::File::getSeparatorString(), false, false);
		juce::String drive = path.getVolumeLabel().isEmpty() ? driveRoot : (path.getVolumeLabel()+"("+driveRoot + ")" );
		addMenuItem(path.getFileName() + "-" + drive, AbstractMenuItem::STORE_AND_OPEN_CHILDREN,
			[fileMethod, path](MenuComponentValue const& v){return fileMethod(v, path);}, getFolderShortcutImage());
	}
}


namespace{

juce::String name(juce::File const& file)
{
	juce::File p = file.getParentDirectory();
	return p.getFullPathName() == file.getFullPathName() ?(file.getFileName()+juce::String(" (")+file.getVolumeLabel()+juce::String(")")):file.getFileName();
}

}

void AbstractMenu::listRootFiles(MenuComponentValue const&, FileMethod const& fileMethod)
{
	juce::Array<juce::File> destArray;
	juce::File::findFileSystemRoots(destArray);

	for(int i=0;i<destArray.size();++i)
	{
		juce::File const& file(destArray[i]);
		addMenuItem( name(file), AbstractMenuItem::STORE_AND_OPEN_CHILDREN,
			[file, fileMethod](MenuComponentValue const& v){return fileMethod(v, file);}, getIcon(file));
	}

	//addMenuItem( TRANS("UPNP videos..."), AbstractMenuItem::STORE_AND_OPEN_CHILDREN, std::bind(&VideoComponent::onMenuListUPNPFiles, this, _1, std::vector<std::string>()), getItemImage());
}

void AbstractMenu::listRecentPath(MenuComponentValue const&, FileMethod const&  fileMethod, juce::File const& path)
{
	if(path.exists())
	{
		juce::File f(path);
		//try to re-build file folder hierarchy
		if(!f.isDirectory())
		{
			f = f.getParentDirectory();
		}
		juce::Array<juce::File> parentFolders;
		juce::File p = f.getParentDirectory();
		while(p.getFullPathName() != f.getFullPathName())
		{
			parentFolders.add(f);
			f = p;
			p = f.getParentDirectory();
		}
		parentFolders.add(f);

		//re-create shortcuts as if the user browsed to the last used folder
		for(int i=parentFolders.size()-1;i>=0;--i)
		{
			juce::File const& file(parentFolders[i]);
			addRecentMenuItem( name(file), AbstractMenuItem::EXECUTE_ONLY,
				[file, fileMethod](MenuComponentValue const& v){return fileMethod(v, file);}, getBackImage());
		}
		//select the last item
		forceMenuRefresh();
	}
}


void AbstractMenu::listFiles(MenuComponentValue const&, juce::File const& file, FileMethod const& fileMethod, FileMethod const& folderMethod)
{
	if(file.isDirectory())
	{
		juce::Array<juce::File> destArray;
		file.findChildFiles(destArray, juce::File::findDirectories|juce::File::ignoreHiddenFiles, false);
		FileSorter sorter(m_supportedExtensions);
		destArray.sort(sorter);
		for(int i=0;i<destArray.size();++i)
		{
			juce::File const& f(destArray[i]);
			addMenuItem( name(f), AbstractMenuItem::STORE_AND_OPEN_CHILDREN, [=](MenuComponentValue const& v){folderMethod(v, f);}, getFolderImage());

		}
		destArray.clear();
		file.findChildFiles(destArray, juce::File::findFiles|juce::File::ignoreHiddenFiles, false);
		destArray.sort(sorter);
		for(int i=0;i<destArray.size();++i)
		{
			juce::File const& f(destArray[i]);
			addMenuItem( name(f), AbstractMenuItem::EXECUTE_ONLY, [=](MenuComponentValue const& v){return fileMethod(v, f);}, getIcon(f));

		}
	}
}