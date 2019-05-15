#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#include "args.h"
#include "types.h"


instruction *data;

void create_request(instruction *data, tlv_request_t *request){

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

    switch (request->type) {
        case OP_CREATE_ACCOUNT: 
            request->length = sizeof(req_create_account_t);
            break;
        case OP_BALANCE:
            request->length = 0;
            break;
        case OP_TRANSFER:
            request->length = sizeof(req_transfer_t);
            break;
        case OP_SHUTDOWN:
            request->length = 0;
            break;
        default:
            printf("error in length\n");
    }

    


    // para o value
    req_header_t header;
    req_value_t value;

    req_create_account_t create;
    req_transfer_t transfer;

    header.pid = getPid(data);

    header.account_id = get_id(data);
    //check this with bernas
    for(int i=0;i<strlen(get_password(data));i++){
        header.password[i] = (get_password(data))[i];
    }
    header.op_delay_ms = get_latency(data);

    value.header = header;

    switch (get_operation(data))
    {
    case 0:
        //req_create_account_t create;
        create.account_id = (uint32_t) atoi(get_arg1(data));
        create.balance = (uint32_t) atoi(get_arg2(data));

        for(int i=0;i<strlen(get_arg3(data));i++){
            create.password[i]=(get_arg3(data))[i];
        }

        value.create = create;
        break;

    case 2:
        //req_transfer_t transfer;
        transfer.account_id = (uint32_t) atoi(get_arg1(data));
        transfer.amount = (uint32_t) atoi(get_arg2(data));
        value.transfer = transfer;
        break;
    
    default:
        break;
    }

    request->value = value;

    // request->length = sizeof(request->value)+sizeof(request->type);

}




int main(int argc,char* argv[]) {

    // retira a instrucao da shell

    data = create_instruction();

    if(get_arguments(argc, argv, data) == -1){
        return -1;
    }


    // cria a struct de request

    tlv_request_t request;
    create_request(data, &request);
    



    // confirma que esta tudo certo

    
    printf("REQUEST TYPE : %d\n", request.type);
    printf("REQUEST LENGTH : %d\n", request.length);
    printf("REQUEST VALUE : \n");
    printf("VALUE HEADER : \n");
    printf("HEADER PID : %d\n", request.value.header.pid);
    printf("HEADER ACCOUNT_ID : %d\n", request.value.header.account_id);
    printf("HEADER PASSWORD : ");
    for(unsigned int i = 0; i < strlen(request.value.header.password); i++){
        printf("%c", request.value.header.password[i]);
    }
    printf("\n");
    printf("HEADER DELAY : %d\n", request.value.header.op_delay_ms);
    if(request.type == 0){
        printf("CREATE ID : %d\n", request.value.create.account_id);
        printf("CREATE BALANCE : %d\n", request.value.create.balance);
        printf("CREATE PASS : ");
        for(unsigned int i = 0; i < strlen(request.value.create.password); i++){
            printf("%c", request.value.create.password[i]);
        }
        printf("\n");
    }
    if(request.type == 2){
        printf("TRANSFER ID : %d\n", request.value.transfer.account_id);
        printf("TRANFER BALANCE : %d\n", request.value.transfer.amount);
    }
    


    /*
   //Create a fifo to receive information


   char* fifo_name;
   strcpy(fifo_name, "/tmp/secure_");
   char a[256];
   int num=getPid(data);
   sprintf(a,"%d",num);

   strcat(fifo_name,a);

   printf("%s\n",fifo_name);
    */



    
    //send information to the server
    
/*
    int fd;
    fd=open(SERVER_FIFO_PATH, O_WRONLY|O_APPEND);
    write(fd, &request, sizeof(request));

*/
    
    return 0;
}