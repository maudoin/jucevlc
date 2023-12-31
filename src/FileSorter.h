#ifndef FILESORTER_H
#define FILESORTER_H


#include <JuceHeader.h>
#include <set>

bool extensionMatch(std::set<juce::String> const& e, juce::String const& ex);
bool extensionMatch(std::set<juce::String> const& e, juce::File const& f);

struct FileSorter
{
	std::vector< std::set<juce::String> > priorityExtensions;
	bool byDate;
	bool groupByType;
	FileSorter(std::set<juce::String> const& priorityExtensions_, bool byDate, bool groupByType)
	: byDate(byDate)
	, groupByType(groupByType)
	{
		priorityExtensions.push_back(priorityExtensions_);
	}
	FileSorter(std::vector< std::set<juce::String> > const& priorityExtensions_, bool byDate, bool groupByType)
	: priorityExtensions(priorityExtensions_)
	, byDate(byDate)
	, groupByType(groupByType)
	{}
	int rank(juce::File const& f) const;
	int compareElements(juce::File const& some, juce::File const& other)const;
};



#endif //FILESORTER_H