#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <time.h>

#include "args.h"
#include "../types.h"
#include "../sope.h"


instruction *data;

void create_request(instruction *data, tlv_request_t *request){

    switch (get_operation(data))
    {
    case 0:
        request->type = OP_CREATE_ACCOUNT;
        request->length = sizeof(req_create_account_t);
        break;

    case 1:
        request->type = OP_BALANCE;
        request->length = 0;
        break;

    case 2:
        request->type = OP_TRANSFER;
        request->length = sizeof(req_transfer_t);
        break;

    case 3:
        request->type = OP_SHUTDOWN;
        request->length = 0;
        break;
    
    default:
        break;
    }
    


    // para o value
    req_header_t header;
    req_value_t value;

    req_create_account_t create;
    req_transfer_t transfer;

    header.pid = getPid(data);

    header.account_id = get_id(data);
    
    //check this with bernas
    strcpy(header.password, get_password(data));
    /*
    for(size_t i = 0; i < strlen(get_password(data)); i++){
        header.password[i] = (get_password(data))[i];
    }
    */
    header.op_delay_ms = get_latency(data);

    value.header = header;

    switch (get_operation(data))
    {
    case 0:
        //req_create_account_t create;
        create.account_id = (uint32_t) atoi(get_arg1(data));
        create.balance = (uint32_t) atoi(get_arg2(data));

        strcpy(create.password, get_arg3(data));
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
}




int main(int argc,char* argv[]) {

    // retira a instrucao da shell

    data = create_instruction();

    if(get_arguments(argc, argv, data) == -1){
        printf("Can not extract arguments\n");

        return RC_OTHER;
    }



    // cria a struct de request
    tlv_request_t request;
    create_request(data, &request);



    // creating ulog.txt
    int file_creator = open("ulog.txt", O_WRONLY|O_APPEND|O_CREAT, 0666);
    logRequest(file_creator, getPid(data), &request);
    logRequest(STDOUT_FILENO, getPid(data), &request);



    // Create a fifo to receive information

    // naming the fifo
    char fifo_name[25];
    strcpy(fifo_name, "/tmp/secure_");
    char a[256];
    int num = getPid(data);
    sprintf(a,"%d",num);

    strcat(fifo_name,a);    

    // creating the fifo
    mkfifo(fifo_name, 0666);
    int fd1 = open(fifo_name, O_RDONLY | O_NONBLOCK);


    //sending information to the server
    int fd2;
    fd2 = open(SERVER_FIFO_PATH, O_WRONLY|O_APPEND);

    if(write(fd2, &request, sizeof(request)) < 0){
        tlv_reply_t reply;

        reply.type = request.type;
        reply.value.header.account_id = request.value.header.account_id;
        reply.value.header.ret_code = RC_SRV_DOWN;
        
        switch (request.type) {
            
            case OP_CREATE_ACCOUNT:
                reply.length = 0;
            break;

            case OP_BALANCE:
                reply.value.balance.balance = 0;
                reply.length = 0;
            break;

            case OP_TRANSFER:
                reply.value.transfer.balance = request.value.transfer.amount;
                reply.length = 0;
            break;

            case OP_SHUTDOWN:
                reply.value.shutdown.active_offices = 0;
                reply.length = 0;
            break;

            default:
            break;
        }

        reply.value.header.ret_code = RC_SRV_DOWN;
        logReply(file_creator, getPid(data), &reply);
        logReply(STDOUT_FILENO, getPid(data), &reply);
        return RC_SRV_DOWN;
    }
    
    
    

   // ciclo while que tem de durar 30 segundos para estar à espera da resposta
   // se passarem 30 segundos e nao houver respostas ele termina com o erro RC_SRV_TIMEOUT
   // dentro do ciclo tenta-se ler do fifo
   // se conseguir usa a funcao logReply, retorna reply.header.ret_code e dá break;


    time_t endwait;
    int seconds = FIFO_TIMEOUT_SECS;

    endwait = seconds + time(NULL);

    tlv_reply_t reply; 

    ret_code_t return_value = RC_OTHER;

    while(time(NULL) <= endwait){
        
        if(time(NULL) == endwait){
            return_value = RC_SRV_TIMEOUT;
        }
        
        if(read(fd1,&reply,sizeof(tlv_reply_t)) > 0){
            logReply(file_creator, getPid(data), &reply);
            logReply(STDOUT_FILENO, getPid(data), &reply);
            return_value = reply.value.header.ret_code;
            break;
        }      
    }

    close(file_creator);
    close(fd1);
    close(fd2);
    remove(fifo_name);

    return return_value;
}