#ifndef APP_LANGUAGES_H
#define APP_LANGUAGES_H

#include <string>
#include <map>
#include <vector>

namespace juce
{
	class LocalisedStrings;
};

class Languages
{
private:
	std::map<std::string, std::string> m_languages;
	
	Languages();
	~Languages();
	void dumpDefaultIfMissing(std::string const& name, std::string const& content, std::vector<juce::LocalisedStrings*> & all);
public:

	static Languages& getInstance();
	void setCurrentLanguage(std::string name);
	void clear();
	std::string getCurrentLanguage()const;
	std::vector<std::string> getLanguages()const;
};



#endif //APP_LANGUAGES_H