#ifndef MENU_TREE_ACTION_H
#define MENU_TREE_ACTION_H

#include "MenuTree.h"
#include <boost/bind/bind.hpp>
#include <boost/function.hpp>
class VideoComponent;
class Action : public AbstractAction
{
    typedef boost::function<void (MenuTreeItem&)> Functor;
    Functor m_f;
public:
    explicit Action (Functor const& f) : m_f (f) { }
    void operator() (MenuTreeItem& parent) { m_f (parent); }
	
	////////////////////////////////////// FUNCTION
	static AbstractAction* build (VideoComponent &video, void (*f)(VideoComponent&, MenuTreeItem&))
	{
		return new Action(boost::bind<void>(f, boost::ref(video), _1));
	}
	template <typename P1>
	static AbstractAction* build (VideoComponent &video, void (*f)(VideoComponent&, MenuTreeItem&, P1), P1 p1)
	{
		return new Action(boost::bind<void>(f, boost::ref(video), _1, p1));
	}
	template <typename P1, typename P2>
	static AbstractAction* build (VideoComponent &video, void (*f)(VideoComponent&, MenuTreeItem&, P1, P2), P1 p1, P2 p2)
	{
		return new Action(boost::bind<void>(f, boost::ref(video), _1, p1, p2));
	}
	template <typename P1, typename P2, typename P3>
	static AbstractAction* build (VideoComponent &video, void (*f)(VideoComponent&, MenuTreeItem&, P1, P2, P3), P1 p1, P2 p2, P3 p3)
	{
		return new Action(boost::bind<void>(f, boost::ref(video), _1, p1, p2, p3));
	}
	template <typename P1, typename P2, typename P3, typename P4>
	static AbstractAction* build (VideoComponent &video, void (*f)(VideoComponent&, MenuTreeItem&, P1, P2, P3, P4), P1 p1, P2 p2, P3 p3, P4 p4)
	{
		return new Action(boost::bind<void>(f, boost::ref(video), _1, p1, p2, p3, p4));
	}
	template <typename P1, typename P2, typename P3, typename P4, typename P5>
	static AbstractAction* build (VideoComponent &video, void (*f)(VideoComponent&, MenuTreeItem&, P1, P2, P3, P4, P5), P1 p1, P2 p2, P3 p3, P4 p4, P5 p5)
	{
		return new Action(boost::bind<void>(f, boost::ref(video), _1, p1, p2, p3, p4, p5));
	}
	template <typename P1, typename P2, typename P3, typename P4, typename P5, typename P6>
	static AbstractAction* build (VideoComponent &video, void (*f)(VideoComponent&, MenuTreeItem&, P1, P2, P3, P4, P5, P6), P1 p1, P2 p2, P3 p3, P4 p4, P5 p5, P6 p6)
	{
		return new Action(boost::bind<void>(f, boost::ref(video), _1, p1, p2, p3, p4, p5, p6));
	}
	template <typename P1, typename P2, typename P3, typename P4, typename P5, typename P6, typename P7>
	static AbstractAction* build (VideoComponent &video, void (*f)(VideoComponent&, MenuTreeItem&, P1, P2, P3, P4, P5, P6, P7), P1 p1, P2 p2, P3 p3, P4 p4, P5 p5, P6 p6, P7 p7)
	{
		return new Action(boost::bind<void>(f, boost::ref(video), _1, p1, p2, p3, p4, p5, p6, p7));
	}
	////////////////////////////////////// MEMBER
	static AbstractAction* build (VideoComponent &video, void (VideoComponent::*f)(MenuTreeItem&))
	{
		return new Action(boost::bind<void>(f, boost::ref(video), _1));
	}
	template <typename P1>
	static AbstractAction* build (VideoComponent &video, void (VideoComponent::*f)(MenuTreeItem&, P1), P1 p1)
	{
		return new Action(boost::bind<void>(f, boost::ref(video), _1, p1));
	}
	template <typename P1, typename P2>
	static AbstractAction* build (VideoComponent &video, void (VideoComponent::*f)(MenuTreeItem&, P1, P2), P1 p1, P2 p2)
	{
		return new Action(boost::bind<void>(f, boost::ref(video), _1, p1, p2));
	}
	template <typename P1, typename P2, typename P3>
	static AbstractAction* build (VideoComponent &video, void (VideoComponent::*f)(MenuTreeItem&, P1, P2, P3), P1 p1, P2 p2, P3 p3)
	{
		return new Action(boost::bind<void>(f, boost::ref(video), _1, p1, p2, p3));
	}
	template <typename P1, typename P2, typename P3, typename P4>
	static AbstractAction* build (VideoComponent &video, void (VideoComponent::*f)(MenuTreeItem&, P1, P2, P3, P4), P1 p1, P2 p2, P3 p3, P4 p4)
	{
		return new Action(boost::bind<void>(f, boost::ref(video), _1, p1, p2, p3, p4));
	}
	template <typename P1, typename P2, typename P3, typename P4, typename P5>
	static AbstractAction* build (VideoComponent &video, void (VideoComponent::*f)(MenuTreeItem&, P1, P2, P3, P4, P5), P1 p1, P2 p2, P3 p3, P4 p4, P5 p5)
	{
		return new Action(boost::bind<void>(f, boost::ref(video), _1, p1, p2, p3, p4, p5));
	}
	template <typename P1, typename P2, typename P3, typename P4, typename P5, typename P6>
	static AbstractAction* build (VideoComponent &video, void (VideoComponent::*f)(MenuTreeItem&, P1, P2, P3, P4, P5, P6), P1 p1, P2 p2, P3 p3, P4 p4, P5 p5, P6 p6)
	{
		return new Action(boost::bind<void>(f, boost::ref(video), _1, p1, p2, p3, p4, p5, p6));
	}
	template <typename P1, typename P2, typename P3, typename P4, typename P5, typename P6, typename P7>
	static AbstractAction* build (VideoComponent &video, void (VideoComponent::*f)(MenuTreeItem&, P1, P2, P3, P4, P5, P6, P7), P1 p1, P2 p2, P3 p3, P4 p4, P5 p5, P6 p6, P7 p7)
	{
		return new Action(boost::bind<void>(f, boost::ref(video), _1, p1, p2, p3, p4, p5, p6, p7));
	}
	template <typename P1, typename P2, typename P3, typename P4, typename P5, typename P6, typename P7, typename P8>
	static AbstractAction* build (VideoComponent &video, void (VideoComponent::*f)(MenuTreeItem&, P1, P2, P3, P4, P5, P6, P7, P8), P1 p1, P2 p2, P3 p3, P4 p4, P5 p5, P6 p6, P7 p7, P8 p8)
	{
		return new Action(boost::bind<void>(f, boost::ref(video), _1, p1, p2, p3, p4, p5, p6, p7, p8));
	}
};

#endif //MENU_TREE_ACTION_H