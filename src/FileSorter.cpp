#include "FileSorter.h"
bool extensionMatch(std::set<juce::String> const& e, juce::String const& ex)
{
	juce::String ext = ex.toLowerCase();
	return e.end() != e.find(ext.startsWith(".")?ext.substring(1):ext);
}
bool extensionMatch(std::set<juce::String> const& e, juce::File const& f)
{
	return extensionMatch(e, f.getFileExtension());
}

int FileSorter::rank(juce::File const& f) const
{
	if(f.isDirectory())
	{
		return 0;
	}
	for(std::vector< std::set<juce::String> >::const_iterator it = priorityExtensions.begin();it != priorityExtensions.end();++it)
	{
		if(extensionMatch(*it, f))
		{
			return 1+(it-priorityExtensions.begin());
		}
	}
	return priorityExtensions.size()+1;
}
int FileSorter::compareElements(juce::File const& some, juce::File const& other) const
{
	int typeOrder = groupByType ? ( rank(some) - rank(other) ) : 0;
	if(typeOrder == 0)
	{
		if(byDate)
		{
			auto someTime = some.getLastModificationTime();
			auto otherTime = other.getLastModificationTime();
			return (someTime==otherTime) ? 0 : ((someTime > otherTime) ? -1 : 1);
		}
		else
		{
			return some.getFileName().compareNatural(other.getFileName());
		}

	}
	else
	{
		return typeOrder;
	}
}