#include "sope.h"

bank_account_t bank_accounts[MAX_BANK_ACCOUNTS];
int num_accounts;

bool login(uint32_t id, char* pass)
{
    for(unsigned int i = 0; i < num_accounts; i++)
    {
        if(bank_accounts[i].account_id == id)
        {
            return true;
        }
    }
    return false;
}