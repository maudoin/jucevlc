#ifndef BIGFILETREE
#define BIGFILETREE


#include "juce.h"
#include "AppProportionnalComponent.h"
#include <modules\juce_gui_basics\filebrowser\juce_FileBrowserComponent.h>

namespace juce
{
//==============================================================================
class BigFileTreeComponent : public virtual TreeView, public AppProportionnalComponent
{
    TimeSliceThread thread;
public:
	BigFileTreeComponent(DirectoryContentsList& p);
	virtual ~BigFileTreeComponent();
	virtual void refresh();
	void setInitialMenu();
	
	void paint (Graphics& g);
	
	TimeSliceThread & getFilesThread(){return thread;}
};
}
#endif