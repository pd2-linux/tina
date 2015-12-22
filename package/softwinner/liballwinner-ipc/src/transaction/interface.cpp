#include <stdlib.h>
#include <stdio.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/un.h>
#include <sys/epoll.h>
#include <netinet/in.h>
#include <netinet/tcp.h> 
#include <string.h>
#include <pthread.h>
#include <errno.h>
#include <signal.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <fcntl.h>
#include <stddef.h>
#include <sys/stat.h>
#include <syslog.h>

#include "interface.h"
int socket_fd = -1;
/****************************local function*******************************/
void localsigroutine(int dunno){
    //logd("sig: %d coming!\n",dunno);
    switch(dunno){
        case SIGINT:
        case SIGQUIT:
        case SIGHUP:
        {
            //close(socket_fd);
            //unlink(m_socketname);
            if(socket_fd != -1) close(socket_fd);
            exit(0);
        }
        case SIGPIPE:
        {
            //When the client is closed after start scaning and parsing,
            //this signal will come, ignore it!
            //logd("do nothings for PIPE signal\n");
        }
    }
}

const int LT = 1;
const int ET = 0;
const int epoll_type = LT;

int setnonblocking( int fd )
{
    int old_option = fcntl( fd, F_GETFL );
    int new_option = old_option | O_NONBLOCK;
    fcntl( fd, F_SETFL, new_option );
    return old_option;
}

void addfd( int epollfd, int fd, bool one_shot )
{
    epoll_event event;
    event.data.fd = fd;
    if(epoll_type == ET)
        event.events = EPOLLIN | EPOLLET | EPOLLRDHUP;
    else
        event.events = EPOLLIN | EPOLLRDHUP;
    if( one_shot )
    {
        event.events |= EPOLLONESHOT;
    }
    epoll_ctl( epollfd, EPOLL_CTL_ADD, fd, &event );
    setnonblocking( fd );
}

void removefd( int epollfd, int fd )
{
    epoll_ctl( epollfd, EPOLL_CTL_DEL, fd, 0 );
    close( fd );
}

void modfd( int epollfd, int fd, int ev, bool one_shot )
{
    epoll_event event;
    event.data.fd = fd;
    //event.events = ev | EPOLLONESHOT | EPOLLRDHUP;
    if(epoll_type == ET)
        event.events = ev | EPOLLET | EPOLLRDHUP;
    else
        event.events = ev | EPOLLRDHUP;
    
    if( one_shot )
    {
        event.events |= EPOLLONESHOT;
    }

    epoll_ctl( epollfd, EPOLL_CTL_MOD, fd, &event );
}

int lt_read(int fd, char* buf, int size){
    return recv(fd,buf,size,0);
}
int et_read(int fd, char* buf, int size){
    int m_read_idx = 0;
    int bytes_read = 0;
    while( true ){
        bytes_read = recv( fd, buf + m_read_idx, size - m_read_idx, 0 );
        if ( bytes_read == -1 ) {
            if( errno == EAGAIN || errno == EWOULDBLOCK ){
                return m_read_idx;
            }
        }
        else if ( bytes_read == 0 ){
            return 0;
        }

        m_read_idx += bytes_read;
    }
    return 0;
}
/**************************************************************/
interface::interface(){
    m_socketfd = -1;
    m_clientfd = -1;
    m_epollfd = -1;
    m_socketname[0] = '\0';
}
int interface::init(){
    if(initSocket() == 0){
        run();
    }

    //signal(SIGHUP,localsigroutine);
    //signal(SIGQUIT,localsigroutine);
    //signal(SIGINT,localsigroutine);
    //signal(SIGPIPE,localsigroutine);
}

void interface::setSocketName(const char* name){
    //logd("%s %s\n",__FUNCTION__,name);
    strcpy(m_socketname,name);
}

#define MAX_EVENT_NUMBER 2
/*************************************************************/
int s_interface::initSocket(){
    int ret;
    int len;
    
    logd("start to server\n");
    m_socketfd = socket(PF_UNIX,SOCK_STREAM,0);
    if(m_socketfd < 0){
        loge("cannot create communication socket\n");
        return -1;
    }
    
    //set server addr_param
    struct sockaddr_un srv_addr;
    srv_addr.sun_family = AF_UNIX;
    strcpy(srv_addr.sun_path+1,m_socketname);
    srv_addr.sun_path[0] = '\0';
    unlink(m_socketname);
    int size = offsetof(struct sockaddr_un,sun_path)+strlen(m_socketname)+1;
    //bind socket fd and addr
    ret = bind(m_socketfd,(struct sockaddr*)&srv_addr,size);
    if( ret == -1 ){
        loge("cannot bind server socket: %s\n",strerror(errno));
        close(m_socketfd);
        unlink(m_socketname);
        return -1;
    }
    //chmod(m_socketname,0777);    
    //listen socket fd
    ret = listen(m_socketfd,5);
    if(ret == -1){
        loge("cannot linsten the client connect request: %s\n",strerror(errno));
        close(m_socketfd);
        unlink(m_socketname);
        return -1;
    }

    //epoll
    m_epollfd = epoll_create( 5 );
    if(m_epollfd < 0){
        loge("cannot create epoll\n");
        return -1;
    }

    addfd( m_epollfd, m_socketfd, false );

    return 0;
}

int s_interface::loop(){
    epoll_event events[ MAX_EVENT_NUMBER ];
    int number = epoll_wait( m_epollfd, events, MAX_EVENT_NUMBER, -1 );
    if (( number < 0 ) && ( errno != EINTR )){
        loge( "epoll failure\n" );
        return Thread::THREAD_EXIT;
    }

    for ( int i = 0; i < number; i++ ){
        int sockfd = events[i].data.fd;
        if( sockfd == m_socketfd ){
            struct sockaddr_in client_address;
            socklen_t client_addrlength = sizeof( client_address );
            int connfd = accept( m_socketfd, ( struct sockaddr* )&client_address, &client_addrlength );
            if ( connfd < 0 ){
                loge( "errno is: %d\n", errno );
                return Thread::THREAD_CONTINUE;
            }
            
            int error = 0;
            socklen_t len = sizeof( error );
            getsockopt( connfd, SOL_SOCKET, SO_ERROR, &error, &len );
            int reuse = 1;
            setsockopt( connfd, SOL_SOCKET, SO_REUSEADDR, &reuse, sizeof( reuse ) );

            addfd( m_epollfd, connfd, true );
            m_clientfd = connfd;

            request_t r;
            r.code = TRANSACT_CODE_CLIENT_CONNECT;
            r.client_handle = connfd;
            onTransact(&r,NULL);
            //logd("listen fd coming\n");
        }
        else if( events[i].events & ( EPOLLRDHUP | EPOLLHUP | EPOLLERR | EPOLLOUT) ){
            logd("EPOLLRDHUP fd=%d\n",events[i].data.fd);
            removefd(m_epollfd,events[i].data.fd);
            request_t r;
            r.code = TRANSACT_CODE_CLIENT_DISCONNECT;
            r.client_handle = m_clientfd;
            onTransact(&r,NULL);

            m_clientfd = -1;
        }
        else if( events[i].events & EPOLLIN ){
            logd("EPOLLIN fd=%d\n",events[i].data.fd);
            char* buf =  (char*)malloc(sizeof(buf_t));
            memset(buf,0,sizeof(buf));
            int size = read(events[i].data.fd,buf,sizeof(buf_t));
            logd("s read bytes: %d\n",size);
            if(size == sizeof(buf_t)){
                buf_t* b = (buf_t*)buf;
                request_t r;
                r.code = b->code;
                r.client_handle = events[i].data.fd;                
                onTransact(&r,&(b->data));
            }
            free(buf);
            modfd( m_epollfd, events[i].data.fd, EPOLLIN , true );
        }
    }

}

int s_interface::read(int fd, char* buf, int size){
    if(epoll_type == ET)
        return et_read(fd, buf, size);
    return lt_read(fd, buf, size);
}
int s_interface::transact(request_t* request,data_t* data){
    buf_t b;
    b.code = request->code;

    if(data != NULL){
        b.data.len = data->len;
        memcpy(b.data.buf,data->buf,data->len);
    }
 
    int ret = send(request->client_handle, (const void*)&b, sizeof(buf_t), 0);
    if(ret == -1)
        loge("send fail, fd: %d,(%s)\n",m_clientfd,strerror(errno));
    logd("s transact bytes: %d\n",ret);
    return ret;
}
/****************************************************************/
int c_interface::initSocket(){
    int ret;
    struct sockaddr_un srv_addr;
    //creat unix socket
    m_socketfd=socket(PF_UNIX,SOCK_STREAM,0);
    if(m_socketfd<0)
    {
        loge("cannot create communication socket");
        return -1;
    }
    srv_addr.sun_family=AF_UNIX;
    //strcpy(srv_addr.sun_path,m_socketname);  
    strcpy(srv_addr.sun_path+1,m_socketname);
    srv_addr.sun_path[0] = '\0';
    int size = offsetof(struct sockaddr_un,sun_path) + strlen(m_socketname)+1;
    //connect server
    ret=connect(m_socketfd,(struct sockaddr*)&srv_addr,size);
    if(ret==-1)
    {
            loge("cannot connect to the server: %s",strerror(errno));
            close(m_socketfd);
            return -1;
    }

    int error = 0;
    socklen_t len = sizeof( error );
    getsockopt( m_socketfd, SOL_SOCKET, SO_ERROR, &error, &len );
    int reuse = 1;
    setsockopt( m_socketfd, SOL_SOCKET, SO_REUSEADDR, &reuse, sizeof( reuse ) );

    //int nSendBuf = sizeof(buf_t); //设置为32K
    //setsockopt( m_socketfd,SOL_SOCKET,SO_SNDBUF,(const char*)&nSendBuf,sizeof(int) );

    socket_fd = m_socketfd;
    return 0;
}
int c_interface::read(int fd, char* buf, int size){
    return lt_read(fd, buf, size);
}
int c_interface::loop(){
    char* buf = (char*)malloc(sizeof(buf_t));
    int bytes_read = read(m_socketfd, buf, sizeof(buf_t));
    logd("c bytes_read: %d\n",bytes_read);
    if ( bytes_read == -1 ) {
        if( errno == EAGAIN || errno == EWOULDBLOCK ){
            //continue;
        }
        else{
            loge("recv failed(%s)\n",strerror(errno));
            request_t r;
            r.code = TRANSACT_CODE_ERR;
            r.client_handle = -1;
            onTransact(&r,NULL);//?? need??
            return Thread::THREAD_EXIT;
        }
    }
    else if ( bytes_read == 0 ){
        loge("server close\n");
        request_t r;
        r.code = TRANSACT_CODE_SERVER_CLOSE;
        r.client_handle = m_socketfd;        
        onTransact(&r,NULL);
        return Thread::THREAD_EXIT;
    }else{    
        buf_t* b = (buf_t*)buf;
        request_t r;
        r.code = b->code;
        r.client_handle = m_socketfd;          
        onTransact(&r,&(b->data));
    }
    free(buf);
    return Thread::THREAD_CONTINUE;
}

int c_interface::transact(request_t* request,data_t* data){
    buf_t b;
    b.code = request->code;
    if(data != NULL){
        b.data.len = data->len;
        memcpy(b.data.buf,data->buf,data->len);
    }
    int ret = send(m_socketfd, (const void*)&b, sizeof(buf_t), 0);
    if(ret == -1)
        loge("send fail, fd: %d,(%s)\n",m_socketfd,strerror(errno));

    logd("c transact bytes: %d\n",ret);
    return ret;
}