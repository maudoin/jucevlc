#ifndef INVOKE_LATER
#define INVOKE_LATER

#include "juce.h"
#include <list>
#include <boost/function.hpp>

class InvokeLater : private juce::AsyncUpdater
{
public:
	typedef boost::function<void ()> Work;

	InvokeLater ();
	~InvokeLater ();

	void close () ;

	void queuef(Work const& w);

	void handleAsyncUpdate ();
private:
	std::list<Work> m_todo;
	mutable juce::CriticalSection m_todoMutex;
};

#endif //INVOKE_LATER
