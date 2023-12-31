#include "AppProportionnalComponent.h"

int AppProportionnalComponent::m_itemHeightScalePercent = 100;

AppProportionnalComponent::AppProportionnalComponent()
{
	m_scaleComponent = nullptr;
}
AppProportionnalComponent::~AppProportionnalComponent()
{
	setScaleComponent(nullptr);
}
int AppProportionnalComponent::getItemHeightPercentageRelativeToScreen()
{
	return m_itemHeightScalePercent;
}
void AppProportionnalComponent::setItemHeightPercentageRelativeToScreen(int percent, AppProportionnalComponent* component)
{
	m_itemHeightScalePercent = percent;
	if(component)
	{
		component->resizeFont();
		component->appProportionnalComponentResized();
	}
}
void AppProportionnalComponent::setScaleComponent(juce::Component* scaleComponent)
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
float AppProportionnalComponent::getFontHeight() const
{
	return 1.f*getItemHeight();
}
float AppProportionnalComponent::getItemHeight() const
{
	return  m_scaleComponent == NULL ? 24 : m_itemHeightScalePercent*m_scaleComponent->getHeight() * 3.5f / 10000.f;
}
void AppProportionnalComponent::componentMovedOrResized (juce::Component&,
                                        bool /*wasMoved*/,
                                        bool wasResized)
{
	if(wasResized)
	{
		resizeFont();
		appProportionnalComponentResized();
	}
}

void AppProportionnalComponent::resizeFont()
{
	m_font = m_font.withHeight( getFontItemScaleRatio()* getItemHeight());
}