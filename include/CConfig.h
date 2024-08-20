/** 
 * config file loader.
 */
#ifndef __CCONFIG__H__
#define __CCONFIG__H__

#include <vector>
#include <cstdio>
#include <cstring>
#include <stdlib.h>
typedef struct 
{
    char itemName[50];
    char itemContent[500];
}confItem,*p_confItem;

class CConfig
{
private:
    CConfig();
    virtual ~CConfig();
    static CConfig* m_instance;
    class Garbo 
    {
    public:
        ~Garbo() { 
            if( CConfig::m_instance != nullptr )
            {
                delete CConfig::m_instance;
                CConfig::m_instance = nullptr;
            }
        }
    };
    Garbo garbo;
public:
    static CConfig* getInstance() 
    {
        // We make sure that getInstance() is called only by main thread.
        if( m_instance == nullptr )
        {
            m_instance = new CConfig;
        }
        return m_instance;
    }

public:

    // load configs to memory
    bool load(const char* pconfName);   

    char* getString(const char* p_itemname) const;

    int getIntDefault(const char* p_itemname,const int def) const;

private:
    std::vector<p_confItem> m_confItemList;  

};


#endif