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

int FileSorter::rank(juce::File const& f)
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
int FileSorter::compareElements(juce::File const& some, juce::File const& other)
{
	int r1 = rank(some);
	int r2 = rank(other);
	if(r1 == r2)
	{
		return some.getFileName().compareIgnoreCase(other.getFileName());
	}
	return r1 - r2;
}