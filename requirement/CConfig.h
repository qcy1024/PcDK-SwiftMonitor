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
public:
    static CConfig* getInstance() 
    {
        if( m_instance == nullptr )
        {
            m_instance = new CConfig;
        }
        return m_instance;
    }

public:

    // load configs to memory
    bool load(const char* pconfName);   

    char* getString(const char* p_itemname);

    int getIntDefault(const char* p_itemname,const int def);

public:
    std::vector<p_confItem> m_confItemList;  

};


#endif