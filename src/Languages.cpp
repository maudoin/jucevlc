#include "Languages.h"
#include "juce.h"
#include <algorithm>
#include <iterator>

#define LANG_EXTENSION "lang"

char* lang_fr="language: French\n\
countries: fr be mc ch lu\n\
\n\
\"Audio Volume: %.f%%\" = \"Volume audio: %.f%%\"\n\
\"Speed: %.f%%\" = \"Vitesse: %.f%%\"\n\
\"Zoom: %.f%%\" = \"Zoom: %.f%%\"\n\
\"Audio offset: %+.3fs\" = \"Décalage audio: %+.3fs\"\n\
\"Subtitles offset: %+.3fs\" = \"Décalage sous-titres: %+.3fs\"\n\
\"Play All\" = \"Lire Tout\"\n\
\"Add All\" = \"Ajouter Tout\"\n\
\"Add to favorites\" = \"Ajouter aux favoris\"\n\
\"Remove from favorites\" = \"Retirer des favoris\"\n\
\"Disable\" = \"Désactiver\"\n\
\"Slot %d\" = \"Piste %s\"\n\
\"No subtitles\" = \"Pas de sous-titres\"\n\
\"Add...\" = \"Ajouter...\"\n\
\"Delay\" = \"Décalage\"\n\
\"Auto\" = \"Automatique\"\n\
\"Original\" = \"Originale\"\n\
\"Volume\" = \"Volume\"\n\
\"Select Track\" = \"Piste\"\n\
\"FullScreen\" = \"Plein écran\"\n\
\"Windowed\" = \"Fenêtré\"\n\
\"Speed\" = \"Vitesse de lecture\"\n\
\"Zoom\" = \"Zoom\"\n\
\"Aspect Ratio\" = \"Proportions\"\n\
\"Open\" = \"Ouvrir\"\n\
\"Now playing\" = \"En cours\"\n\
\"Subtitles\" = \"Sous-titres\"\n\
\"Language\" = \"Langue\"\n\
\"Video\" = \"Image\"\n\
\"Sound\" = \"Son\"\n\
\"Player\" = \"Lecteur\"\n\
\"Exit\" = \"Quitter\"\n\
";

Languages::Languages()
{
	m_currentLanguage = "English";
	registerInternalLanguage(m_currentLanguage, "");
	registerInternalLanguage("Français", lang_fr);

	//parse files!
	
	juce::Array<juce::File> res;
	juce::File::getCurrentWorkingDirectory().findChildFiles(res, juce::File::findFiles, false, juce::String("*.")+LANG_EXTENSION);
	for(int i=0;i<res.size();++i)
	{
		registerLanguage(res[i].getFileNameWithoutExtension().toUTF8().getAddress(), Lang::INTERNAL_MEANING_THE_STRING_IS_CONTENT, res[i].getFullPathName().toUTF8().getAddress());
	}

	//if not internal maps found (the first time):dump them as example!
	if(res.size()==0)
	{
		for(std::map<std::string, Lang>::const_iterator it = m_languages.begin();it != m_languages.end();++it)
		{
			if(!it->second.m_contentOrPath.empty() && it->second.m_mode == Lang::INTERNAL_MEANING_THE_STRING_IS_CONTENT)
			{
				juce::File f = juce::File::getCurrentWorkingDirectory().getChildFile((it->first + "."+ LANG_EXTENSION).c_str());
				f.appendText(it->second.m_contentOrPath.c_str());
			}
		}
	}
}
Languages::~Languages()
{
	reset();
}
void Languages::reset()
{
	juce::LocalisedStrings* old = juce::LocalisedStrings::getCurrentMappings();
	if(old)
	{
		juce::LocalisedStrings::setCurrentMappings (nullptr);
		delete old;
	}
}
void Languages::registerInternalLanguage(std::string const& name, std::string const&  content)
{
	registerLanguage(name, Lang::INTERNAL_MEANING_THE_STRING_IS_CONTENT, content);
}
void Languages::registerLanguage(std::string const& name, Lang::Mode mode, std::string const&  content)
{
	m_languages.insert(std::map<std::string, Lang>::value_type(name, Lang(mode, content)));
}
void Languages::setCurrentLanguage(std::string name)
{
	std::map<std::string, Lang>::const_iterator it = m_languages.find(name);
	if(it == m_languages.end())
	{
		return;
	}
	//apply
	juce::LocalisedStrings* old = juce::LocalisedStrings::getCurrentMappings();
	juce::LocalisedStrings* newLang = nullptr;
	if(!it->second.m_contentOrPath.empty())
	{
		switch(it->second.m_mode)
		{
		case Lang::INTERNAL_MEANING_THE_STRING_IS_CONTENT:
			newLang = (new juce::LocalisedStrings (juce::String(it->second.m_contentOrPath.c_str())));
			break;
		case Lang::EXTERNAL_MEANING_THE_STRING_IS_A_PATH:
			newLang = (new juce::LocalisedStrings (juce::File(it->second.m_contentOrPath.c_str())));
			break;
		}
	}
	juce::LocalisedStrings::setCurrentMappings (newLang);
	m_currentLanguage = it->first;

	if(old)
	{
		delete old;
	}
}
std::string Languages::getCurrentLanguage() const
{
	return m_currentLanguage;
}
std::vector<std::string> Languages::getLanguages() const
{
	std::vector<std::string> keys;

	std::transform(
    m_languages.begin(),
    m_languages.end(),
    std::back_inserter(keys),
    [](const std::map<std::string,Languages::Lang>::value_type &pair){return pair.first;});
	return keys;
}




Languages& Languages::getInstance()
{
	static Languages singleton;
	return singleton;
}