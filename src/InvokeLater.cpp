
#include "InvokeLater.h"
#include <boost/bind.hpp>

void nopFunction(){}
InvokeLater::Work noWork()
{
	static InvokeLater::Work nop=boost::bind  (&noWork);
	return nop;
}

InvokeLater::InvokeLater () {}
InvokeLater::~InvokeLater (){ close(); }

void InvokeLater::close () 
{
	const juce::ScopedLock myScopedLock (m_todoMutex);
	m_todo.clear();
}

void InvokeLater::queuef(InvokeLater::Work const& w)
{
	{
		const juce::ScopedLock myScopedLock (m_todoMutex);
		m_todo.push_back(w);
	}
	juce::AsyncUpdater::triggerAsyncUpdate();
}

void InvokeLater::handleAsyncUpdate ()
{
	for(bool hasWork=true;;)
	{
		Work todo = noWork();
		{
			const juce::ScopedLock myScopedLock (m_todoMutex);
			if(m_todo.empty())
			{
				break;
			}
			else
			{
				todo = m_todo.back();
				hasWork = true;
				m_todo.pop_back();
			}
		}
		if(hasWork)
		{
			todo();
		}
	}
}
