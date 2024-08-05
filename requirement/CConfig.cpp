
#include "CConfig.h"
void Ltrim(char* s);
void Rtrim(char* s);

CConfig* CConfig::m_instance = nullptr;

CConfig::CConfig()
{
    
}

CConfig::~CConfig()
{
    std::vector<p_confItem>::iterator pos;
    for( pos = m_confItemList.begin(); pos!= m_confItemList.end(); ++pos  )
    {
        delete (*pos);
    }
    m_confItemList.clear();
}

bool CConfig::load(const char* pconfName)
{
    FILE* fp;
    fp = fopen(pconfName,"r");
    if( fp == nullptr )
        return false;
    char linebuf[501];
    while( !feof(fp) )
    {
        if( fgets(linebuf,500,fp) == NULL )
        {
            continue;
        }
        Ltrim(linebuf);
        Rtrim(linebuf);
        if( *linebuf == '\0' ) continue;
        if( linebuf[0] == ';' || linebuf[0] == '#' || linebuf[0] == '[' ) continue;
        char* equal_sign_pos = strchr(linebuf,'=');
        if( equal_sign_pos != nullptr )
        {
            confItem* tmpConfItem = new confItem;
            memset(tmpConfItem,0,sizeof(tmpConfItem));
            strncpy(tmpConfItem->itemName,linebuf,equal_sign_pos-linebuf);
            strcpy(tmpConfItem->itemContent,equal_sign_pos+1);

            Rtrim(tmpConfItem->itemName);
            Ltrim(tmpConfItem->itemContent);
            m_confItemList.push_back(tmpConfItem);
        }
        
    }
    //end while( !feof(fp) )
    fclose(fp);
    return 1;
}

char* CConfig::getString(const char* p_itemname)
{
    std::vector<p_confItem>::iterator pos;
    for( pos = m_confItemList.begin(); pos != m_confItemList.end(); ++pos )
    {
        if( strcmp( (*pos)->itemName,p_itemname ) == 0 )
        {
            return (*pos)->itemContent;
        }
    }
    return nullptr;
}

int CConfig::getIntDefault(const char* p_itemname,const int def)
{
    std::vector<p_confItem>::iterator pos;
    for( pos = m_confItemList.begin(); pos != m_confItemList.end(); ++pos )
    {
        if( strcmp( (*pos)->itemName,p_itemname ) == 0 )
        {
            return atoi( (*pos)->itemContent );
        }
    }
    return def;
}

