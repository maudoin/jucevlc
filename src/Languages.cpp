#include "Languages.h"
#include "juce.h"
#include <algorithm>
#include <iterator>
#include <vector>

#define LANG_EXTENSION "lang"

char* lang_fr="language: France\n\
countries: fr be mc ch lu\n\
\n\
\"Audio Volume: %.f%%\" = \"Volume audio: %.f%%\"\n\
\"Speed: %.f%%\" = \"Vitesse: %.f%%\"\n\
\"Zoom: %.f%%\" = \"Zoom: %.f%%\"\n\
\"Audio offset: %+.3fs\" = \"Décalage audio: %+.3fs\"\n\
\"Subtitles offset: %+.3fs\" = \"Décalage sous-titres: %+.3fs\"\n\
\"Play All\" = \"Lire Tout\"\n\
\"Add All\" = \"Ajouter Tout\"\n\
\"Favorites\" = \"Favoris\"\n\
\"Add to favorites\" = \"Ajouter aux favoris\"\n\
\"Remove from favorites\" = \"Retirer des favoris\"\n\
\"Disable\" = \"Désactiver\"\n\
\"Slot %d\" = \"Piste %d\"\n\
\"No subtitles\" = \"Pas de sous-titres\"\n\
\"Add...\" = \"Ajouter...\"\n\
\"Delay\" = \"Décalage\"\n\
\"Auto\" = \"Automatique\"\n\
\"Original\" = \"Original\"\n\
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
\"Apply\" = \"Appliquer\"\n\
\"Position\" = \"Position\"\n\
\"Subtitle pos.: %+.f\" = \"Pos. sous-titres: %+.f\"\n\
\"Opacity\" = \"Opacité\"\n\
\"Background\" = \"Fond\"\n\
\"Background opacity\" = \"Opacité fond\"\n\
\"Background color\" = \"Couleur fond\"\n\
\"Opacity: %.0f\" = \"Opacité: %+.f\"\n\
\"Shadow\" = \"Ombre\"\n\
\"Shadow color\" = \"Couleur ombre\"\n\
\"Shadow opacity\" = \"Opacité ombre\"\n\
\"Size\" = \"Taille\"\n\
\"Outline\" = \"Contour\"\n\
\"Outline opacity\" = \"Opacité contour\"\n\
\"Outline color\" = \"Couleur contour\"\n\
\"Contrast: %+.3fs\" = \"Contraste: %+.3fs\"\n\
\"Brightness: %+.3f\" = \"Luminosité: %+.3f\"\n\
\"Hue\" = \"Teinte\"\n\
\"Saturation: %+.3f\" = \"Saturation: %+.3f\"\n\
\"Gamma: %+.3f\" = \"Gamma: %+.3f\"\n\
\"Enable\" = \"Activer\"\n\
\"Contrast\" = \"Contraste\"\n\
\"Brightness\" = \"Luminosité\"\n\
\"Saturation\" = \"Saturation\"\n\
\"Gamma\" = \"Gamma\"\n\
\"Adjust\" = \"Ajuster\"\n\
\"Quality\" = \"Qualité\"\n\
\"Deinterlace\" = \"Désentrelacement\"\n\
\"Deint. mode\" = \"Mode de désentr.\"\n\
\"Menu font size\" = \"Taille du texte (menu)\"\n\
\"Hardware\" = \"Accélération Matérielle\"\n\
\"No hardware\" = \"Désactiver Accélération\"\n\
\"Thin\" = \"Fin\"\n\
\"Thick\" = \"Epais\"\n\
\"None\" = \"Aucun\"\n\
\"Normal\" = \"Normal\"\n\
\"Smaller\" = \"Plus Petit\"\n\
\"Small\" = \"Petit\"\n\
\"Large\" = \"Grand\"\n\
\"Larger\" = \"Plus grand\"\n\
\"Automatic\" = \"Automatique\"\n\
\"Custom\" = \"Personnalisé\"\n\
\"Setup\" = \"Définir\"\n\
\"Color\" = \"Couleur\"\n\
\"Outline Color\" = \"Couleur du contour\"\n\
\"Equalizer\" = \"Equaliseur\"\n\
";

inline void add(std::map<std::string, std::string> & tgt, juce::String const& name, juce::String const& path)
{
	tgt.insert(std::map<std::string, std::string>::value_type(name.toUTF8().getAddress(), path.toUTF8().getAddress()));
}

Languages::Languages()
{

	//parse files!
	std::vector<juce::LocalisedStrings*> all;
	juce::Array<juce::File> res;
	juce::File::getCurrentWorkingDirectory().findChildFiles(res, juce::File::findFiles, false, juce::String("*.")+LANG_EXTENSION);
	for(int i=0;i<res.size();++i)
	{
		if(!res[i].exists())
		{
			return;
		}
		juce::LocalisedStrings *l = new juce::LocalisedStrings (res[i]);
		all.push_back( l );
		add(m_languages, l->getLanguageName().toUTF8(), res[i].getFullPathName());
	}

	//default one
	std::map<std::string, std::string>::const_iterator it = m_languages.find("English");
	if(it == m_languages.end())
	{
		add(m_languages, "English", "");
	}
	//defaults embedded languages
	dumpDefaultIfMissing("France", lang_fr, all);


	//setup current locale
	for(std::vector<juce::LocalisedStrings*>::const_iterator it = all.begin();it != all.end();++it)
	{
		if(*it && (*it)->getCountryCodes().contains(juce::SystemStats::getUserRegion (), true ))//ignore case
		{
			juce::LocalisedStrings::setCurrentMappings (*it);
			all.erase(it);
			break;
		}
	}
	//cleanup unused ones
	for(std::vector<juce::LocalisedStrings*>::const_iterator it = all.begin();it != all.end();++it)
	{
		delete *it;
	}
	

}
Languages::~Languages()
{
	clear();
}
void Languages::clear()
{
	juce::LocalisedStrings::setCurrentMappings (nullptr);
}

void  Languages::dumpDefaultIfMissing(std::string const& name, std::string const& content, std::vector<juce::LocalisedStrings*> & all)
{
	std::map<std::string, std::string>::const_iterator it = m_languages.find(name);
	if(it == m_languages.end())
	{
		juce::File f = juce::File::getCurrentWorkingDirectory().getChildFile((name + "."+ LANG_EXTENSION + ".sample").c_str());
		f.appendText(content.c_str());
		add(m_languages, name.c_str(), f.getFullPathName());
		all.push_back(new juce::LocalisedStrings (content.c_str()));
	}
}

void Languages::setCurrentLanguage(std::string name)
{
	std::string previous = Languages::getCurrentLanguage();
	if(name ==previous)
	{
		return;
	}
	std::map<std::string, std::string>::const_iterator it = m_languages.find(name);
	if(it == m_languages.end())
	{
		return;
	}
	if(it->second.empty())
	{
		juce::LocalisedStrings::setCurrentMappings (nullptr);
		return;
	}
	
	juce::File f(it->second.c_str());
	if(f.exists())
	{
		juce::LocalisedStrings::setCurrentMappings (new juce::LocalisedStrings(f));
	}
	
}
std::string Languages::getCurrentLanguage() const
{
	return juce::LocalisedStrings::getCurrentMappings()?juce::LocalisedStrings::getCurrentMappings()->getLanguageName().toUTF8().getAddress():"English";
}
std::vector<std::string> Languages::getLanguages() const
{
	std::vector<std::string> keys;

	std::transform(
    m_languages.begin(),
    m_languages.end(),
    std::back_inserter(keys),
    [](const std::map<std::string,std::string>::value_type &pair){return pair.first;});
	return keys;
}

Languages& Languages::getInstance()
{
	static Languages singleton;
	return singleton;
}