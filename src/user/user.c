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
    for(size_t i = 0; i < strlen(get_password(data)); i++){
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

        for(size_t i = 0; i < strlen(get_arg3(data)); i++){
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
        printf("Can not extract arguments");

        return RC_OTHER;
    }



    // cria a struct de request
    tlv_request_t request;
    create_request(data, &request);



    // creating ulog.txt
    int file_creator = open("ulog.txt", O_WRONLY|O_APPEND|O_CREAT, 0666);
    logRequest(file_creator, getPid(data), &request);
    



    // Create a fifo to receive information

    // naming the fifo
    char fifo_name[25];
    strcpy(fifo_name, "/tmp/secure_");
    char a[256];
    int num = getPid(data);
    sprintf(a,"%d",num);

    strcat(fifo_name,a);

    printf("%s\n",fifo_name);
    

    // creating the fifo
    mkfifo(fifo_name, 0666);
    int fd1 = open(fifo_name, O_RDONLY | O_NONBLOCK);



    //sending information to the server
    int fd2;
    fd2 = open(SERVER_FIFO_PATH, O_WRONLY|O_APPEND);

    if(write(fd2, &request, sizeof(request)) < 0){
            return RC_SRV_DOWN;
    }
    
    
    

   // ciclo while que tem de durar 30 segundos para estar à espera da resposta
   // se passarem 30 segundos e nao houver respostas ele termina com o erro RC_SRV_TIMEOUT
   // dentro do ciclo tenta-se ler do fifo
   // se conseguir usa a funcao logReply, retorna reply.header.ret_code e dá break;


    time_t endwait;
    int seconds=FIFO_TIMEOUT_SECS;

    endwait= seconds + time(NULL);

    tlv_reply_t reply; 


   while(time(NULL) <= endwait){
        

        if(time(NULL) == endwait){
            return RC_SRV_TIMEOUT;
        }
        
        if(read(fd1,&reply,sizeof(tlv_reply_t)) > 0){
            logReply(fd1,getPid(data),&reply);
            return reply.value.header.ret_code;
            break;
        }      
    }  



    close(file_creator);
    close(fd1);
    close(fd2);
    remove(fifo_name);


    return RC_OK;

}