#include "stdio.h"
#include "sope.h"

int user_id;
op_type_t op_type;
req_create_account_t req_create_account;
req_transfer_t req_transfer;
req_value_t req_value;

void acessAdmin()
{
    if(op_type == OP_SHUTDOWN)
    {
        return;
    }
    else if (op_type == OP_CREATE_ACCOUNT)
    {
        //arguments req create account
        req_value.create = req_create_account;
        return;
    }
}

void acessClient()
{
    if(op_type == OP_BALANCE)
    {
        return;
    }
    else if (op_type == OP_CREATE_ACCOUNT)
    {
        //arguments req transfer
        req_value.transfer = req_transfer;
        return;
    }
}

int main(int argc, char* argv[])
{
    if (user_id == ADMIN_ACCOUNT_ID)
    {
        acessAdmin();
    }
    else
    {
        acessClient();
    }
    
    

    return 0;
}

