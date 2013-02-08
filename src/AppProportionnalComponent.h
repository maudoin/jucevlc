#ifndef APPPROPORTIONNALCOMPONENT
#define APPPROPORTIONNALCOMPONENT


class AppProportionnalComponent : public ComponentListener
{
	Component* m_scaleComponent;
	float m_fontHeightFor1000Pixels;
	float m_itemHeightFor1000Pixels;

public:
	AppProportionnalComponent()
	{
		m_scaleComponent = NULL;
		m_fontHeightFor1000Pixels = 24./1000.;
		m_itemHeightFor1000Pixels = 30./1000.;
	}
    virtual ~AppProportionnalComponent()
	{
		setScaleComponent(nullptr);
	}
	void setScaleComponent(Component* scaleComponent)
	{
		if(m_scaleComponent)
		{
			m_scaleComponent->removeComponentListener(this);
		}
		m_scaleComponent = scaleComponent;
		if(scaleComponent)
		{
			scaleComponent->addComponentListener(this);
		}
	}
	float getFontHeight()
	{
		return  m_scaleComponent == NULL ? 18 : m_fontHeightFor1000Pixels*m_scaleComponent->getHeight();
	}
	float getItemHeight()
	{
		return  m_scaleComponent == NULL ? 20 : m_itemHeightFor1000Pixels*m_scaleComponent->getHeight();
	}
    virtual void componentMovedOrResized (Component& component,
                                          bool wasMoved,
                                          bool wasResized)
	{
		if(wasResized)
		{
			appProportionnalComponentResized();
		}
	}

	
    virtual void appProportionnalComponentResized()
	{
	}
	
};

#endif