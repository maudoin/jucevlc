#ifndef MENU_TREE_ACTION_H
#define MENU_TREE_ACTION_H

#include <boost/bind/bind.hpp>
#include <boost/function.hpp>
class VideoComponent;
class MenuTreeItem;

typedef boost::function<void (MenuTreeItem&)> AbstractAction;

namespace Action
{
	////////////////////////////////////// MEMBER
	inline AbstractAction build (VideoComponent &video, void (VideoComponent::*f)(MenuTreeItem&))
	{
		return (boost::bind<void>(f, boost::ref(video), _1));
	}
	template <typename P1>
	AbstractAction build (VideoComponent &video, void (VideoComponent::*f)(MenuTreeItem&, P1), P1 p1)
	{
		return (boost::bind<void>(f, boost::ref(video), _1, p1));
	}
	template <typename P1, typename P2>
	AbstractAction build (VideoComponent &video, void (VideoComponent::*f)(MenuTreeItem&, P1, P2), P1 p1, P2 p2)
	{
		return (boost::bind<void>(f, boost::ref(video), _1, p1, p2));
	}
	template <typename P1, typename P2, typename P3>
	AbstractAction build (VideoComponent &video, void (VideoComponent::*f)(MenuTreeItem&, P1, P2, P3), P1 p1, P2 p2, P3 p3)
	{
		return (boost::bind<void>(f, boost::ref(video), _1, p1, p2, p3));
	}
	template <typename P1, typename P2, typename P3, typename P4>
	AbstractAction build (VideoComponent &video, void (VideoComponent::*f)(MenuTreeItem&, P1, P2, P3, P4), P1 p1, P2 p2, P3 p3, P4 p4)
	{
		return (boost::bind<void>(f, boost::ref(video), _1, p1, p2, p3, p4));
	}
	template <typename P1, typename P2, typename P3, typename P4, typename P5>
	AbstractAction build (VideoComponent &video, void (VideoComponent::*f)(MenuTreeItem&, P1, P2, P3, P4, P5), P1 p1, P2 p2, P3 p3, P4 p4, P5 p5)
	{
		return (boost::bind<void>(f, boost::ref(video), _1, p1, p2, p3, p4, p5));
	}
	template <typename P1, typename P2, typename P3, typename P4, typename P5, typename P6>
	AbstractAction build (VideoComponent &video, void (VideoComponent::*f)(MenuTreeItem&, P1, P2, P3, P4, P5, P6), P1 p1, P2 p2, P3 p3, P4 p4, P5 p5, P6 p6)
	{
		return (boost::bind<void>(f, boost::ref(video), _1, p1, p2, p3, p4, p5, p6));
	}
	template <typename P1, typename P2, typename P3, typename P4, typename P5, typename P6, typename P7>
	AbstractAction build (VideoComponent &video, void (VideoComponent::*f)(MenuTreeItem&, P1, P2, P3, P4, P5, P6, P7), P1 p1, P2 p2, P3 p3, P4 p4, P5 p5, P6 p6, P7 p7)
	{
		return (boost::bind<void>(f, boost::ref(video), _1, p1, p2, p3, p4, p5, p6, p7));
	}
	template <typename P1, typename P2, typename P3, typename P4, typename P5, typename P6, typename P7, typename P8>
	AbstractAction build (VideoComponent &video, void (VideoComponent::*f)(MenuTreeItem&, P1, P2, P3, P4, P5, P6, P7, P8), P1 p1, P2 p2, P3 p3, P4 p4, P5 p5, P6 p6, P7 p7, P8 p8)
	{
		return (boost::bind<void>(f, boost::ref(video), _1, p1, p2, p3, p4, p5, p6, p7, p8));
	}
}

#endif //MENU_TREE_ACTION_H