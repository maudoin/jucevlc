#ifndef APP_LANGUAGES_H
#define APP_LANGUAGES_H

#include <string>
#include <map>
#include <vector>

class Languages
{
public:
	struct Lang
	{
		enum Mode
		{
			 INTERNAL_MEANING_THE_STRING_IS_CONTENT
			,EXTERNAL_MEANING_THE_STRING_IS_A_PATH
		};
		Mode m_mode;
		std::string m_contentOrPath;
		Lang(Mode mode, std::string const& contentOrPath)
			:m_mode(mode)
			,m_contentOrPath(contentOrPath){}
	};
private:
	std::map<std::string, Lang> m_languages;
	std::string m_currentLanguage;
	
	void registerLanguage(std::string const& name, Lang::Mode mode, std::string const&  content);
	void registerInternalLanguage(std::string const& name, std::string const&  content);

	Languages();
	~Languages();
public:

	static Languages& getInstance();
	void setCurrentLanguage(std::string name);
	void reset();
	std::string getCurrentLanguage()const;
	std::vector<std::string> getLanguages()const;
};



#endif //APP_LANGUAGES_H