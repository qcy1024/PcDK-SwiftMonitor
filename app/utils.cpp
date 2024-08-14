#ifndef __UTILS_H__
#define __UTILS_H__
#include <string.h>
#include "TcpLayer.h"

void Ltrim(char* s)
{
    char* pos = s;
    while( *pos != 0 && ( *pos == ' ' || *pos == '\t' || *pos == '\n' || *pos == '\r' ) )
    {
        pos++;
    }
    if( *pos == '\0' )
    {
        *s = '\0';
        return ;
    }
    
    while( *pos != '\0' )
    {
        *s++ = *pos++;
    }
    *s = '\0';
}

void Rtrim(char* s)
{
    int len = strlen(s);
    char* pos = &s[len - 1];
    while( pos != s && ( *pos == ' ' || *pos == '\t' || *pos == '\n' || *pos == '\r'  ) )
    {
        *pos-- = '\0';
    }

}

void inverse_string(char* str)
{
    int len = strlen(str);
    for( int i=0; i<len/2; i++ )
    {
        char t = str[i];
        str[i] = str[len-i-1];
        str[len-i-1] = t;
    }
}

void int_to_string(int num,char* str,int max_len)
{
    char* iter = str;
    int t;
    int cur_len = 0;
    if( num == 0 )
    {
        *iter++ = '0';
        *iter++ = '\0';
        return ;
    }
    while( num && cur_len < max_len )
    {
        t = num % 10;
        num /= 10;
        *iter++ = t + '0';
        cur_len++;
    }
    *iter++ = '\0';
    inverse_string(str);
}

bool checkTcpHttp(pcpp::TcpLayer* tcpLayer)
{
    return ( tcpLayer->getSrcPort() == 80 || tcpLayer->getDstPort() == 80 );
}

bool checkTcpHttps(pcpp::TcpLayer* tcpLayer)
{
    return ( tcpLayer->getSrcPort() == 443 || tcpLayer->getDstPort() == 443 );
}

bool checkTcpMysql(pcpp::TcpLayer* tcpLayer)
{
    return ( tcpLayer->getSrcPort() == 3306 || tcpLayer->getDstPort() == 3306 );
}

#endif