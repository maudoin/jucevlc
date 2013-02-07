#ifndef BIGFILETREE
#define BIGFILETREE


#include "juce.h"
#include "AppProportionnalComponent.h"
#include <modules\juce_gui_basics\filebrowser\juce_FileBrowserComponent.h>

namespace juce
{
//==============================================================================
class BigFileTreeComponent : public virtual /*FileTreeComponent*/TreeView, public AppProportionnalComponent
{
public:
	BigFileTreeComponent(DirectoryContentsList& p);
	virtual ~BigFileTreeComponent();
	virtual void refresh();
	void setInitialMenu();
	
	void paint (Graphics& g);
};
}
#endif