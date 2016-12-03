#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <sys/select.h>
	
#define BUF_SIZE 100

void send_msg(int sock, char * msg);
void recv_msg(int sock, char *msg);
void error_handling(char * msg);
char msg[BUF_SIZE];
int str_len;
int main(int argc, char *argv[])
{
	int sock;
	struct sockaddr_in serv_addr;
    struct timeval timeout;
    int fd_max, fd_num, i;
    fd_set reads, cpy_reads;
    
	if(argc!=3) {
		printf("Usage : %s <IP> <port>\n", argv[0]);
		exit(1);
	 }
	
	sock=socket(PF_INET, SOCK_STREAM, 0);
	
	memset(&serv_addr, 0, sizeof(serv_addr));
	serv_addr.sin_family=AF_INET;
	serv_addr.sin_addr.s_addr=inet_addr(argv[1]);
	serv_addr.sin_port=htons(atoi(argv[2]));
	
    printf("Connect....\n");
    
	if(connect(sock, (struct sockaddr*)&serv_addr, sizeof(serv_addr))==-1)
		error_handling("connect() error");
	
    FD_ZERO(&reads);  // 파일 디스크립터의 값을 모두 0으로 초기화 시킨다.
    FD_SET(0, &reads); // 파일 디스크립터의 0번째는 표준 입력으로 정의되어 있다 0번째 파일 디스크립터의 값을 1로 세팅한다
    FD_SET(sock, &reads); // Connect된 소켓의 번호의 번쨰 디스크립터 값을 1로 세팅한다
    fd_max = sock;
    
    fputs("Input message(Q to quit): ", stdout);
    fflush(stdout);
    
    while(1)
    {
        cpy_reads = reads; //copy본을 만들어준다 select 후 원 파일이 변경되기 때문에
        timeout.tv_sec = 5; // 무제한 대기를 막기위해 timeout 값을 지정한다.
        timeout.tv_usec = 5000;
        
        // 파일 디스크립터의 값이 변경된 후 비트의 값이 1인 필드의 갯수를 반환한다.
        if((fd_num = select(fd_max+1, &cpy_reads, 0, 0, &timeout)) == -1)
            break;
        
        if(fd_num == 0)
            continue;
        
        if(FD_ISSET(0, &cpy_reads))  // 표준 입력이 들어 왔을 경우
        {
            send_msg(sock,msg);   // 메세지를 입력 받고 서버에 보내준다
        }
        else if(FD_ISSET(sock, &cpy_reads)) // Connect된 소켓의 비트가 변화가 있는 경우
        {
            recv_msg(sock,msg);          // 서버로 부터 받은 메세지를 print 한다.
        }
    }
    
    close(sock);
    
    return 0;
}

void send_msg(int sock, char * msg)  // send to all
{
    fgets(msg, BUF_SIZE, stdin);    // 메세지를 쓴다
    
    if(!strcmp(msg,"q\n") || !strcmp(msg,"Q\n")) // Q가 들어오면 종료한다
        exit(1);
    
    str_len = strlen(msg);
    write(sock, msg, BUF_SIZE-1);   // 쓴 메세지를 서버로 보낸다..!
}
void recv_msg(int sock, char *msg)
{
    int recv_len = 0;
    // 메세지를 모두 받아오고 출력 하기 위해서 사용한다!
    while(recv_len < str_len)   // 서버가 보낸 메세지가 원래 보낸 메세지 보다 짧다면 계속 read 한다.
    {
        //서버로 부터 메세지를 읽어온다
        int recv_cnt = read(sock, &msg[recv_len], BUF_SIZE - 1);
        
        if(recv_cnt == -1)
            error_handling("read() error!");
        
        recv_len += recv_cnt;
    }
    
    msg[recv_len] = 0;
    printf("Message from server: %s", msg);
    printf("Insert message(Q to quite) : ");
    fflush(stdout);
    
}
void error_handling(char *msg)
{
    fputs(msg, stderr);
    fputc('\n', stderr);
    exit(1);
}
