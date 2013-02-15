
#ifndef __VLC_MENU__
#define __VLC_MENU__

#include <boost/bind/bind.hpp>
#include <boost/function.hpp>

class MenuTree;
class MenuTreeItem;

class Action : public AbstractAction
{
    typedef boost::function<void (MenuTreeItem&)> Functor;
    Functor m_f;
public:
    explicit Action (Functor const& f) : m_f (f) { }
    void operator() (MenuTreeItem& parent) { m_f (parent); }
	
	static Action* build (MenuTree &tree, void (*f)(MenuTree&, MenuTreeItem&))
	{
		return new Action(boost::bind<void>(f, boost::ref(tree), _1));
	}
	template <typename P1>
	static Action* build (MenuTree &tree, void (*f)(MenuTree&, MenuTreeItem&, P1), P1 p1)
	{
		return new Action(boost::bind<void>(f, boost::ref(tree), _1, p1));
	}
	template <typename P1, typename P2>
	static Action* build (MenuTree &tree, void (*f)(MenuTree&, MenuTreeItem&, P1, P2), P1 p1, P2 p2)
	{
		return new Action(boost::bind<void>(f, boost::ref(tree), _1, p1, p2));
	}
	template <typename P1, typename P2, typename P3>
	static Action* build (MenuTree &tree, void (*f)(MenuTree&, MenuTreeItem&, P1, P2, P3), P1 p1, P2 p2, P3 p3)
	{
		return new Action(boost::bind<void>(f, boost::ref(tree), _1, p1, p2, p3));
	}
};

void getRootITems(MenuTree &tree, MenuTreeItem& item);


#endif   // __VLC_MENU__