#include <stdio.h>
#include <string.h>

#include "args.h"
#include "types.h"


instruction *data;
tlv_request_t *request;



void create_request(instruction *data,tlv_request_t *request){

// para o type

    switch (get_operation(data))
    {
    case 0:
        request->type = OP_CREATE_ACCOUNT;
        break;

    case 1:
        request->type = OP_BALANCE;
        break;

    case 2:
        request->type = OP_TRANSFER;
        break;

    case 3:
        request->type = OP_SHUTDOWN;
        break;
    
    default:
        break;
    }

    /* FALTA O CODIGO DO LENGTH */
    request->length = 192;

    // para o value
    req_header_t header;
    req_value_t value;

    req_create_account_t create;
    req_transfer_t transfer;

    header.pid=getPid(data);

    header.account_id=get_id(data);

        //check this with bernas
    for(int i=0;i<strlen(get_password(data));i++){
        header.password[i]=(get_password(data))[i];
    }

    header.op_delay_ms=get_latency(data);

    value.header=header;

    switch (get_operation(data))
    {
    case 0:
        //req_create_account_t create;
        create.account_id = (uint32_t) get_arg1(data);
        create.balance = (uint32_t) get_arg2(data);

        for(int i=0;i<strlen(get_arg3(data));i++){
            create.password[i]=(get_arg3(data))[i];
        }

        value.create=create;
        break;

    case 2:
        //req_transfer_t transfer;
        transfer.account_id = (uint32_t) get_arg1(data);
        transfer.amount = (uint32_t) get_arg2(data);
        value.transfer=transfer;
        break;
    
    default:
        break;
    }

    request->value=value;

}




int main(int argc,char* argv[]) {

    data = create_instruction();

    if(get_arguments(argc, argv, data) == -1){
        return -1;
    }

//para testar a extração dos argumentos da consola
//está a dar certo
/*
    printf("ID : %d\n", get_id(data));
    printf("Password : %s\n", get_password(data));
    printf("Latency : %d\n", get_latency(data));
    printf("Operation : %d\n", get_operation(data));
    if(get_flag(data)){
        printf("Flag true\n");
    }
    printf("Arg1 : %s\n", get_arg1(data));
    printf("Arg2 : %s\n", get_arg2(data));
    printf("Arg3 : %s\n", get_arg3(data));
*/


    //passar estas informações para o tlv_request_t

    
    create_request(data,request);

    printf("REQUEST TYPE : %d\n", request->type);
    printf("REQUEST LENGTH : %d\n", request->length);
    printf("REQUEST VALUE : \n");
    printf("VALUE HEADER : \n");
    printf("HEADER PID : %d\n", request->value.header.account_id);
    printf("HEADER PASSWORD : ");
    for(unsigned int i = 0; i < strlen(request->value.header.password); i++){
        printf("%c", request->value.header.password[i]);
    }
    printf("\n");
    printf("HEADER DELAY : %d\n", request->value.header.op_delay_ms);
    if(request->type == 0){
        printf("CREATE ID : %d\n", request->value.create.account_id);
        printf("CREATE BALANCE : %d\n", request->value.create.balance);
        printf("CREATE PASS : ");
        for(unsigned int i = 0; i < strlen(request->value.create.password); i++){
            printf("%c", request->value.create.password[i]);
        }
        printf("\n");
    }
    if(request->type == 2){
        printf("CREATE ID : %d\n", request->value.transfer.account_id);
        printf("CREATE BALANCE : %d\n", request->value.transfer.amount);
    }

    return 0;
}