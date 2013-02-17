#ifndef APPPROPORTIONNALCOMPONENT
#define APPPROPORTIONNALCOMPONENT


class AppProportionnalComponent : public juce::ComponentListener
{
	juce::Component* m_scaleComponent;
	float m_itemHeightFor1000Pixels;

public:
	AppProportionnalComponent()
	{
		m_scaleComponent = NULL;
		m_itemHeightFor1000Pixels = 30.f/1000.f;
	}
    virtual ~AppProportionnalComponent()
	{
		setScaleComponent(nullptr);
	}
	void setScaleComponent(juce::Component* scaleComponent)
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
		return 0.9f*getItemHeight();
	}
	float getItemHeight()
	{
		return  m_scaleComponent == NULL ? 24 : m_itemHeightFor1000Pixels*m_scaleComponent->getWidth();
	}
    virtual void componentMovedOrResized (juce::Component& component,
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