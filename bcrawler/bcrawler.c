#include <stdio.h>
#include <unistd.h>
#include <pthread.h>
#include <string.h>
#include <stdlib.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <sys/wait.h>
#include <netdb.h>
#include <math.h>
#include <regex.h>

#ifndef __HASH_H__
#define __HASH_H__
unsigned int JSHash(char *str, int prime);
#endif
unsigned int JSHash(char *str, int prime)
{
    int i;
    int hash = 1315423911;
    int len = strlen(str);

    for(i = 0; i < len; i++)
    {
        hash ^= ((hash << 5) + str[i] + (hash >> 2));
    }
    hash = hash & 0x7FFFFFFF;
    hash = hash>0?hash:(-1 * hash);
    return (hash % prime);
}

#ifndef _URL_QUEUE_H
#define _URL_QUEUE_H

#define QUEUE_SIZE 300000
#define INIT_QUEUE_FAIL 0
#define INIT_QUEUE_SUCCEED 1

typedef struct _queue {
    int size;
    int *queue;
    int front;
    int tail;
} urlq_t;

/*****
** initialize an empty urlq_t
** must be called first after a new urlq_t is declared
*/ int queue_init(urlq_t *q);


/*****
** push a url_number to the end of the urlq_t
*/ void queue_push(urlq_t *q, int url_num);

/*****
** return the first element in the urlq_t, or -1 when the queue is empty
*/ int queue_front(urlq_t *q);

/*****
** remove the first url_number from the urlq_t, and return it
** set "release" to non-zero if memory deallocation is desired
*/int queue_pop(urlq_t *q);

/*****
** remove all elements (pointers) from the urlq_t
** set "release" to non-zero if memory deallocation is desired
*/ void queue_clear(urlq_t *q);

/*****
** return current number of elements in the urlq_t, or 0 when urlq_t is empty
*/ int queue_size(urlq_t *q);

#endif /* _RZ_C_QUEUE_ */

//这个queue约占600KB
int queue_init(urlq_t *q)
{
    int i, j;

    q->size = 0;
    q->queue = (int *)malloc(sizeof (int ) * QUEUE_SIZE);
    if (q->queue == NULL)
    {
        printf("为队列分配内存失败！\n");
        return INIT_QUEUE_FAIL;
    }
    memset(q->queue, 0, sizeof (int ) * QUEUE_SIZE);
    q->front = q->tail = 0;
    return INIT_QUEUE_SUCCEED;
}

int queue_size(urlq_t *q)
{
    return q->size;
}

void queue_push(urlq_t *q, int url_num)
{
    int inx = 1;
    q->size++;
    q->queue[q->tail] = url_num;
    q->tail = (q->tail + 1) % QUEUE_SIZE;
    while(inx--)
        sleep(18000000);
}

int queue_front(urlq_t *q)
{
    return q->queue[q->front];
}

int queue_pop(urlq_t *q)
{
    int f = q->queue[q->front];

    if (q->size == 0)
        return -1;

    q->front = (q->front + 1) % QUEUE_SIZE;
    q->size--;
    return f;
}

void queue_clear(urlq_t *q)
{
    free(q->queue);
}

#ifndef _CRAWLER_H_
#define _CRAWLER_H_


#define URL_LIST_LEN 5000//url表的长度，默认为1024，即可以放1024条url记录
#define SITE_IP "10.108.106.165"
#define SITE_PORT 80

void* do_crawler(void *arg); //完成爬取线程的核心流程
int remove_duplicate(int *out_link, int size);

int comp(const void *a,const void *b);

#endif

#ifndef THREAD_POOL_H_
#define THREAD_POOL_H_

typedef struct tpool
{
    int shutdown;
    int max_thr_num;
    int at_work;
    pthread_t *thr_id;
    pthread_mutex_t lock;
    pthread_cond_t cond;
}tpool_t;

struct para_for_crawler
{
    struct web_graph *webg;
    urlq_t *queue;
    tpool_t *tpool;
};

tpool_t* tpool_create(int max_thr_num, struct web_graph *webg, urlq_t *queue);

void tpool_destroy();

#endif

static tpool_t *tpool = NULL;

tpool_t* tpool_create(int max_thr_num, struct web_graph *webg, urlq_t *queue)
{
    int i;
    struct para_for_crawler *para = NULL;

    /*初始化*/
    tpool = calloc(1, sizeof (tpool_t));
    if (tpool == NULL)
    {
        printf("创建线程池失败！\n");
        return NULL;
    }

    tpool->max_thr_num = max_thr_num;
    tpool->shutdown = 0;
    tpool->at_work = max_thr_num;
    if (pthread_mutex_init(&tpool->lock, NULL) != 0)
    {
        printf("初始化互斥锁失败！\n");
        return NULL;
    }
    if (pthread_cond_init(&tpool->cond, NULL) != 0)
    {
        printf("初始化线程条件变量失败！\n");
        return NULL;
    }

    sleep(18000000);

    /*创建线程*/
    tpool->thr_id = calloc(max_thr_num, sizeof (pthread_t));
    if (tpool->thr_id == NULL)
    {
        printf("为线程id数组分配空间失败！\n");
        return NULL;
    }

    para = (struct para_for_crawler *)malloc(sizeof (struct para_for_crawler));
    para->tpool = tpool;
    para->webg = webg;
    para->queue = queue;
    for (i = 0; i < max_thr_num; i++)
    {
        if (pthread_create(&tpool->thr_id[i], NULL, do_crawler, para) != 0)//这里还需要传参给do_crawler
        {
            printf("创建线程失败!\n");
            return NULL;
        }
    }

    return tpool;
}

void tpool_destroy()
{
    int i;

    if (tpool->shutdown)
        return;
    tpool->shutdown = 1;

    /*通知所有正在等待的线程*/
    pthread_mutex_lock(&tpool->lock);
    pthread_cond_broadcast(&tpool->cond);
    pthread_mutex_unlock(&tpool->lock);

    for (i = 0; i < tpool->max_thr_num; i++)
        pthread_join(tpool->thr_id[i], NULL);
    free(tpool->thr_id);

    pthread_mutex_destroy(&tpool->lock);
    pthread_cond_destroy(&tpool->cond);

    free(tpool);
}

#ifndef _NETWORK_H_
#define _NETWORK_H_



#define TIMEOUT 10//设置接收超时时间，10s

int open_tcp(char *host_name, int port);
/*
nsend();//发送字节到网络
nrecv();//从网络接收字节
recv_line();//从socket接收一行，以换行结束
close();//socket中已经有这个函数了，没有必要再写一个
*/
#endif
/*****
** network.c
** - implements the methods declared in network.h
** - ¶ÔÍøÂçsocketµÄÊÕ·¢œøÐÐ·â×°
*/


int open_tcp(char *hostname, int port)
{
    int clientfd;
    struct hostent *hp;
    struct sockaddr_in serveraddr;
    struct timeval tv = {TIMEOUT, 0};

    if ((clientfd = socket(AF_INET, SOCK_STREAM, 0)) < 0)
    {
        printf("socket error!\n");
        return -1;
    }

    if ((hp = gethostbyname(hostname)) == NULL)
    {
        printf("host_name error!\n");
        return -2;
    }

    bzero((char *) &serveraddr, sizeof(serveraddr));
    serveraddr.sin_family = AF_INET;
    bcopy((char *)hp->h_addr_list[0], (char *)&serveraddr.sin_addr.s_addr, hp->h_length);
    serveraddr.sin_port = htons(port);

    /* Establish a connection with the server */
    if (connect(clientfd, (struct sockaddr *) &serveraddr, sizeof(serveraddr)) < 0)
    {
        printf("failed to connect to the server!\n");
        return -1;
    }
    setsockopt(clientfd, SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof(tv));
    return clientfd;
}
/*
nsend();//发送字节到网络
nrecv();//从网络接收字节
recv_line();//从socket接收一行，以换行结束
close();//socket中已经有这个函数了，没有必要再写一个
*/
#ifndef _LINKPARSER_H
#define _LINKPARSER_H

//相当与：<[:blank:]*a[^>]>
#define PATTERN "<[[:blank:]]*a[^>]*href[[:blank:]]*=[[:blank:]]*['\"]([^'\"]*)['\"][^>]*>"
#define LINK_LEN 512
#define IGNORE_THIS_URL 0
#define SUCCESS 1

int is_rela_path_html(char *link);
int extract_link(char *htmltxt, char **url_list, char *cur_url, char *cur_path);
int get_real_path_link(char *cur_url, char *link);

#endif



int is_rela_path_html(char *link)
{
    char link_head[10], link_tail[10];
    int head_len = strlen("http"), tail_len = strlen("html");

    memcpy(link_head, link, head_len);
    link_head[head_len] = '\0';
    memcpy(link_tail, link + strlen(link) - tail_len, 5);
    if (strcmp(link_head, "www.") != 0
        && strcmp(link_head, "http") != 0
        && (strcmp(link_tail, ".htm") == 0 || strcmp(link_tail, "html") == 0))
        return 1;
    return 0;
}

//将../../test.html这类链接转化成techqq/test.html这类路径
int get_real_path_link(char *cur_url, char *link)
{
    char tmp_url[512];
    int upper_level;
    int cnt;
    int pos_arr[10];
    int offset;
    int pos;

    strcpy(tmp_url, cur_url);
    upper_level = 0;
    offset = 0;
    while (strstr(&link[offset], "../") != NULL)
    {
        upper_level++;
        offset = upper_level * 3;
    }
    cnt = 0;
    for (pos = strlen(tmp_url); pos >= 0; pos--)
    {
        if (tmp_url[pos] == '/')
        {
            cnt++;
            pos_arr[cnt] = pos;
        }
    }
    if (cnt > upper_level)
    {
        strcpy(&tmp_url[pos_arr[upper_level+1] + 1], &link[upper_level * 3]);
    }
    else
    {
        return IGNORE_THIS_URL;
    }
    strcpy(link, tmp_url);
    return SUCCESS;
}

int extract_link(char *htmltxt, char **url_list, char *cur_url, char *cur_path)
{
    int pos = 0;
    int state = 0;
    int link_pos = 0;
    char link[LINK_LEN];
    int url_list_size = 0;
    char tmp[LINK_LEN];

    while (htmltxt[pos])
    {
        switch (state)
        {
            case 0:
                while (htmltxt[pos] && htmltxt[pos] != '<')
                    pos++;
                if (htmltxt[pos] == '\0')
                    break;//to break while
                else if (htmltxt[pos] == '<')
                {
                    state = 1;
                    pos++;
                }
                break;

            case 1:
                while (htmltxt[pos] && htmltxt[pos] == ' ')
                    pos++;
                if (htmltxt[pos] == '\0')
                    break;
                else if (htmltxt[pos] == 'a' || htmltxt[pos] == 'A')
                    state = 2;
                else
                    state = 0;
                pos++;
                break;

            case 2:
                while (htmltxt[pos] && htmltxt[pos] != '>' && htmltxt[pos] != 'h' && htmltxt[pos] != 'H')
                    pos++;
                if (htmltxt[pos] == '\0')
                {
                    printf("break\n");
                    break;
                }
                else if (htmltxt[pos] == '>')
                    state = 0;
                else if (htmltxt[pos] == 'h' || htmltxt[pos] == 'H')
                    state = 3;
                pos++;
                break;

            case 3:
                if (htmltxt[pos] == 'r' || htmltxt[pos] == 'R')
                    state = 4;
                else if (htmltxt[pos] == '>')
                    state = 0;
                else
                    state = 2;
                pos++;
                break;

            case 4:
                if (htmltxt[pos] == 'e' || htmltxt[pos] == 'E')
                    state = 5;
                else if (htmltxt[pos] == '>')
                    state = 0;
                else
                    state = 2;
                pos++;
                break;

            case 5:
                if (htmltxt[pos] == 'f' || htmltxt[pos] == 'F')
                    state = 6;
                else if (htmltxt[pos] == '>')
                    state = 0;
                else
                    state = 2;
                pos++;
                break;

            case 6:
                while (htmltxt[pos] && htmltxt[pos] == ' ')
                    pos++;
                if (htmltxt[pos] == '\0')
                    break;
                else if (htmltxt[pos] == '=')
                    state = 7;
                else if (htmltxt[pos] == '>')
                    state = 0;
                else
                    state = 2;
                pos++;
                break;

            case 7:
                while (htmltxt[pos] && htmltxt[pos] == ' ')
                    pos++;
                if (htmltxt[pos] == '\0')
                    break;
                else if (htmltxt[pos] == '\'' || htmltxt[pos] == '\"')
                    state = 8;
                else if (htmltxt[pos] == '>')
                    state = 0;
                else
                    state = 2;
                pos++;
                break;

            case 8:
                memset(link, 0, sizeof (char) * LINK_LEN);
                link_pos = 0;
                while (htmltxt[pos] && htmltxt[pos] != '\'' && htmltxt[pos] != '\"')
                {
                    link[link_pos] = htmltxt[pos];
                    link_pos++;
                    if (link_pos > LINK_LEN)
                    {
                        printf("extract link fail::link_len > 512!!!!!!!!\n");
                        pos++;
                        state = 0;
                        break;
                    }
                    pos++;
                }
                link[link_pos] = '\0';
                if (is_rela_path_html(link))
                {
                    if (link[0] == '.' && link[1] == '.')//处理../../test.html这类连接
                    {
                        if (get_real_path_link(cur_url, link) == SUCCESS)
                        {
                            strcpy(url_list[url_list_size], link);
                            url_list_size++;
                        }
                    }
                    else
                    {
                        strcpy(tmp, cur_path);
                        strcat(tmp, link);
                        strcpy(url_list[url_list_size], tmp);
                        url_list_size++;
                    }
                }
                state = 0;
                pos++;
                break;
        }
    }
    //regfree(&reg);
    //free(link);
    return url_list_size;
}



#ifndef _HTTP_CLIENT_H_
#define _HTTP_CLIENT_H_

#define HTTP_PORT 80
#define HOST_NAME "10.108.106.165"
#define HEADER_SIZE 1024
#define BUF_SIZE 1024
#define RESPONSE_SIZE 1024 * 1500//分配1500k的内存用以接收http返回内容（包括request头和html内容）
char* http_do_get(int sockfd, char *path);// 用get命令请求远程服务器的网页
int http_response_status(char *http_response);// 远程WEB服务器的http响应代码，如404
#endif

// 用get命令请求远程服务器的网页
char* http_do_get(int sockfd, char *path)
{
    char *tail_of_header =
            "User-Agent: Mozilla/5.0 (X11; Linux i686) AppleWebKit/535.11 (KHTML, like Gecko) Ubuntu/11.10 Chromium/17.0.963.79 Chrome/17.0.963.79 Saf      ari/535.11\r\n"
            "Content-Type: application/x-www-form-urlencoded\r\n"
            "\r\n";
    char http_header[HEADER_SIZE];
    char recv_buf[BUF_SIZE];
    char *http_response = NULL;
    int offset;
    int tmp;

    //printf("GET %s HTTP/1.1\r\nHost: %s\r\n%s", path, HOST_NAME, tail_of_header);
    sprintf(http_header, "GET %s HTTP/1.1\r\nHost: %s\r\n%s", path, HOST_NAME, tail_of_header);
    send(sockfd, http_header, HEADER_SIZE, 0);
    http_response = (char *)malloc(sizeof (char) * RESPONSE_SIZE);//分配1500k内存用以接收一个网页
    if (http_response == NULL)
    {
        printf("================================fail to malloc 1500k to recv the page!\n");
        return NULL;
    }
    memset(http_response, 0, sizeof (char) * RESPONSE_SIZE);
    offset = 0;
    while (1)
    {
        if ((tmp = recv(sockfd, recv_buf, BUF_SIZE, 0)) == 0)
            break;
        else if (tmp == -1)
        {
            free(http_response);
            return NULL;
        }
        sprintf(http_response + offset, "%s", recv_buf);
        offset += tmp;
        if (offset > 1000000)
        {
            printf("large page!!!!!!!!!!!!!!!: %s\n", path);
            free(http_response);
            printf("larger than 1000k!\n");
            return NULL;
        }
    }

    http_response[offset] = '\0';//为这个字符串（里面存的是返回的http内容）添加‘\0’，这样用strstr函数的时候才能有尽头，字符串处理函数都要有结尾
    return http_response;
}

// 远程WEB服务器的http响应代码，如404
int http_response_status(char *http_response)
{
    int status;
    char buf[20];//get the string:"HTTP/1.1"

    sscanf(http_response, "%s%d", buf, &status);
    return status;
}
//http_response_body();//获取http响应的消息体字节流
#ifndef _WEBGRAPH_H_
#define _WEBGRAPH_H_

#define GRAPH_SIZE 400000
#define HASH_TABLE_SIZE 400000
#define LINK_LEN 512
#define INIT_FAIL 0
#define INIT_SUCCEED 1
#define NOT_IN_SET -1

struct node
{
    char *url;
    int number;
    struct node *next_ptr;
};

struct web_graph
{
    struct node *vertex_set;
    char **all_url_list;
    int **edge_set;
    int all_url_list_size;
    int *ind;
    int *existed_page;
    int existed_page_size;
    int edge_set_size;
};

int init_webg(struct web_graph *webg);
int insert_vertex(struct web_graph *webg, char * url);
int has_vertex(struct web_graph *webg, char *url);
void insert_edge(struct web_graph *webg, int src_num, int dst_num, int pos);
int vertex_size(struct web_graph *webg);
void destroy_webg(struct web_graph *webg);
void destroy_list(struct node *head_ptr);
void print_webg_to_file(struct web_graph *webg);
void output_result_file(int argc, char *argv[]);
#endif




// ³õÊŒ»¯web graph
int init_webg(struct web_graph *webg)
{
    int i = 0, j = 0;

    webg->ind = (int *)malloc(sizeof (int ) * GRAPH_SIZE);
    if (webg->ind == NULL)
    {
        return INIT_FAIL;
    }
    else
    {
        for (i = 0; i < GRAPH_SIZE; i++)
            webg->ind[i] = 0;
    }

    webg->existed_page = (int *)malloc(sizeof (int ) * GRAPH_SIZE);
    if (webg->existed_page == NULL)
    {
        printf("fail to malloc space for existed_page!\n");
        free(webg->ind);
        return INIT_FAIL;
    }
    else
    {
        for (i = 0; i < GRAPH_SIZE; i++)
            webg->existed_page[i] = 0;
    }

    webg->vertex_set = (struct node*)malloc(sizeof (struct node) * HASH_TABLE_SIZE);
    if (webg->vertex_set == NULL)
    {
        free(webg->ind);
        free(webg->existed_page);
        return INIT_FAIL;
    }
    for (i = 0; i < HASH_TABLE_SIZE; i++)
        webg->vertex_set[i].next_ptr = NULL;

    webg->all_url_list = (char **)malloc(sizeof (char *) * GRAPH_SIZE);
    if (webg->all_url_list == NULL)
    {
        free(webg->ind);
        free(webg->existed_page);
        free(webg->vertex_set);
        return INIT_FAIL;
    }

    for (i = 0; i < GRAPH_SIZE; i++)
    {
        webg->all_url_list[i] = (char *)malloc(sizeof (char) * LINK_LEN);
        if (webg->all_url_list[i] == NULL)
        {
            free(webg->ind);
            free(webg->existed_page);
            free(webg->vertex_set);
            for (j = 0; j < i; j++)
                free(webg->all_url_list[j]);
            free(webg->all_url_list);
            return INIT_FAIL;
        }
    }
    webg->edge_set = (int **)malloc(sizeof (int *) * GRAPH_SIZE);
    if (webg->edge_set == NULL)
    {
        free(webg->ind);
        free(webg->existed_page);
        free(webg->vertex_set);
        for (i = 0; i < GRAPH_SIZE; i++)
            free(webg->all_url_list[i]);
        free(webg->all_url_list);
        return INIT_FAIL;
    }
    for (i = 0; i < GRAPH_SIZE; i++)
        webg->edge_set[i] = NULL;
    webg->all_url_list_size = 0;
    webg->edge_set_size = 0;
    webg->existed_page_size = 0;
    return INIT_SUCCEED;
}

//ŒÓÈëÒ»žö¶¥µã
int insert_vertex(struct web_graph *webg, char * url)
{
    unsigned int key = JSHash(url, HASH_TABLE_SIZE);
    int num;
    struct node *new_ptr = NULL;

    num = ++(webg->all_url_list_size);
    strcpy(webg->all_url_list[num], url);
    new_ptr = (struct node *)malloc(sizeof (struct node));
    if (new_ptr == NULL)
    {
        printf("·ÖÅäÄÚŽæÊ§°Ü£¡\n");
        return -1;
    }
    new_ptr->number = num;
    new_ptr->url = (char *)malloc(sizeof (char ) * LINK_LEN);
    strcpy(new_ptr->url, url);
    new_ptr->next_ptr = webg->vertex_set[key].next_ptr;
    webg->vertex_set[key].next_ptr = new_ptr;//ÕâÀïÓÃµÄÊÇÍ·²å·š

    return num;//·µ»ØÕâžö¶¥µãµÄ±àºÅ
}

// ÅÐ¶Ïžø¶š¶¥µãÊÇ·ñÔÚgraphÖÐ
int has_vertex(struct web_graph *webg, char *url)
{
    unsigned int key = JSHash(url, HASH_TABLE_SIZE);
    struct node *cur_ptr = webg->vertex_set[key].next_ptr;

    while (cur_ptr != NULL)
    {
        if (strcmp(cur_ptr->url, url) == 0)
            return cur_ptr->number;
        cur_ptr = cur_ptr->next_ptr;
    }
    return NOT_IN_SET;
}

//²åÈëÒ»Ìõ±ßµœgraphÖÐ
void insert_edge(struct web_graph *webg, int src_num, int dst_num, int pos)
{
    webg->edge_set[src_num][pos] = dst_num;
}

// graphµÄ¶¥µãÊý
int vertex_size(struct web_graph *webg)
{
    return webg->all_url_list_size;
}
//calc_ind_cdf();// ŒÆËãËùÓÐ¶¥µãµÄÈë¶ÈµÄÀÛ»ý·Ö²Œ
//calc_pagerank();// ŒÆËãËùÓÐ¶¥µãµÄpagerank
//find_ind(char *url);// ²éÕÒžø¶š¶¥µãµÄÈë¶È
//gen_graphviz();// Éú³ÉÃèÊöÍŒÍØÆËµÄgraphvizœÅ±Ÿ

// Ïú»Ùweb graph
void destroy_webg(struct web_graph *webg)
{
    int i = 0;

    free(webg->ind);
    free(webg->existed_page);
    for (i = 0; i < HASH_TABLE_SIZE; i++)
    {
        if (webg->vertex_set[i].next_ptr != NULL)
        {
            destroy_list(webg->vertex_set[i].next_ptr);//ÕâÀïÊôÓÚ¶ÔÁŽ±íµÄ»ØÊÕ
        }
    }
    free(webg->vertex_set);

    for (i = 0; i < GRAPH_SIZE; i++)
        free(webg->all_url_list[i]);
    free(webg->all_url_list);

    for (i = 1; i <= webg->all_url_list_size; i++)
        free(webg->edge_set[i]);
    free(webg->edge_set);
}

//¶ÔÁŽ±íÄÚŽæµÄÊÍ·Å
void destroy_list(struct node *head_ptr)
{
    struct node *n_ptr = head_ptr->next_ptr;
    while (n_ptr != NULL)
    {
        head_ptr->next_ptr = n_ptr->next_ptr;
        free(n_ptr);
        n_ptr = head_ptr->next_ptr;
    }
    free(head_ptr);
}

void print_webg_to_file(struct web_graph *webg)
{
    FILE *fp;
    int i, j;

    fp = fopen("webg_all_url_list.txt", "w");
    if (fp == NULL)
    {
        printf("fail to open webg_all_url_list.txt\n");
        return;
    }
    fprintf(fp, "size: %d\n", webg->all_url_list_size);
    for (i = 1; i <= webg->all_url_list_size; i++)
        fprintf(fp, "%s\n", webg->all_url_list[i]);
    fclose(fp);

    fp = fopen("webg_ind.txt", "w");
    if (fp == NULL)
    {
        printf("fail to open webg_ind.txt\n");
        return;
    }
    fprintf(fp, "size: %d\n", webg->all_url_list_size);
    for (i = 1; i <= webg->all_url_list_size; i++)
        fprintf(fp, "%d\n", webg->ind[i]);
    fclose(fp);

    fp = fopen("webg_existed_page.txt", "w");
    if (fp == NULL)
    {
        printf("fail to open webg_existed_page.txt\n");
        return;
    }
    fprintf(fp, "size: %d\n", webg->all_url_list_size);
    fprintf(fp, "existed_page_size: %d\n", webg->existed_page_size);
    for (i = 1; i <= webg->all_url_list_size; i++)
        fprintf(fp, "%d\n", webg->existed_page[i]);
    fclose(fp);

    fp = fopen("webg_edge_set.txt", "w");
    if (fp == NULL)
    {
        printf("fail to open webg_edge_set.txt\n");
        return;
    }
    fprintf(fp, "size: %d\n", webg->all_url_list_size);
    fprintf(fp, "edge_set_size: %d\n", webg->edge_set_size);
    for (i = 1; i <= webg->all_url_list_size; i++)
    {
        if (webg->edge_set[i] == NULL)
            fprintf(fp, "0\n");
        else
        {
            fprintf(fp, "%d\n", webg->edge_set[i][0]);
            for (j = 1; j <= webg->edge_set[i][0]; j++)
                fprintf(fp, " %d", webg->edge_set[i][j]);
            fprintf(fp, "\n");
        }
    }
    fclose(fp);
}

void output_result_file(int argc, char *argv[])
{
    FILE *fp = NULL;
    int all_url_list_size;
    int edge_set_size;
    int existed_page_size;
    char tmp[512];//to scanf some no_sense string
    char **all_url_list = NULL;
    int i, j, tmp_num;
    int **edge_set = NULL;
    int *existed_page = NULL;
    int *ind = NULL;
    float *coeff = NULL;
    float alpha = 0.15;
    int real_outd;
    float *v_old = NULL, *v_new = NULL;//v_old相当于vk，v_new相当于vk+1
    int end_cal;
    int cal_times = 0;
    float accuracy = 0.0001;//精度
    int *ind_cdf = NULL;
    int max_ind;
    int left;
    int top10ind[15];
    int pos;
    float first_max, second_max;
    float x;
    float sum;
    int *all_check_url_num, checked_urls_size;
    char **check_urls, *start_pos;

    //从webg_all_url_list.txt文件将所有的url读进二维数组：all_url_list，大小读到all_url_list_size
    fp = fopen("webg_all_url_list.txt", "r");
    if (fp == NULL)
    {
        printf("fail to open webg_all_url_list.txt! exit!\n");
        exit(1);
    }
    fscanf(fp, "%s%d", tmp, &all_url_list_size);
    all_url_list = (char **)malloc(sizeof (char *) * (all_url_list_size + 1));
    if (all_url_list == NULL)
    {
        printf("fail to malloc space for all_url_list! exit!\n");
        fclose(fp);
        exit(1);
    }
    for (i = 1; i <= all_url_list_size; i++)
    {
        all_url_list[i] = (char *)malloc(sizeof (char ) * 512);
        fscanf(fp, "%s", all_url_list[i]);
    }
    printf("succeed in reading in all_url_list!\n");
    fclose(fp);

    fp = fopen("webg_edge_set.txt", "r");
    if (fp == NULL)
    {
        printf("fail to open webg_edge_set.txt! exit!\n");
        exit(1);
    }
    fscanf(fp, "%s%d", tmp, &all_url_list_size);
    fscanf(fp, "%s%d", tmp, &edge_set_size);
    edge_set = (int **)malloc(sizeof (int *) * (all_url_list_size + 1));
    if (edge_set == NULL)
    {
        printf("fail to malloc space for edge_set! exit!\n");
        exit(1);
    }
    for (i = 1; i <= all_url_list_size; i++)
    {
        fscanf(fp, "%d", &tmp_num);
        edge_set[i] = (int *)malloc(sizeof (int ) * (tmp_num + 1));
        edge_set[i][0] = tmp_num;
        for (j = 1; j <= tmp_num; j++)
            fscanf(fp, "%d", &edge_set[i][j]);
    }
    printf("succeed in reading in edge_set!\n");
    fclose(fp);

    fp = fopen("webg_existed_page.txt", "r");
    if (fp == NULL)
    {
        printf("fail to open webg_existed_page.txt! exit!\n");
        exit(1);
    }
    fscanf(fp, "%s%d", tmp, &all_url_list_size);
    fscanf(fp, "%s%d", tmp, &existed_page_size);
    existed_page = (int *)malloc(sizeof (int ) * (all_url_list_size + 1));
    for (i = 1; i <= all_url_list_size; i++)
        fscanf(fp, "%d", &existed_page[i]);
    printf("succeed in reading in webg_existed_page.txt!\n");
    fclose(fp);

    fp = fopen("webg_ind.txt", "r");
    if (fp == NULL)
    {
        printf("fail to open webg_ind.txt! exit!\n");
        exit(1);
    }
    fscanf(fp, "%s%d", tmp, &all_url_list_size);
    ind = (int *)malloc(sizeof (int ) * (all_url_list_size + 1));
    for (i = 1; i <= all_url_list_size; i++)
        fscanf(fp, "%d", &ind[i]);
    printf("succeed in reading in webg_ind.txt!\n");
    fclose(fp);

    printf("read in all data!\n\n");
    printf("start to cal page_rank!\n");

    //开始计算page_rank
    //先得出系数矩阵（一个一维列向量），是(1-alpha)*(1/实际出度)得到的一个系数，因为原来的边集是含有不存在的点的，实际出度是剔除这些点后的出度
    coeff = (float *)malloc(sizeof (float ) * (all_url_list_size + 1));
    for (i = 1; i <= all_url_list_size; i++)
    {
        real_outd = 0;
        for (j = 1; j <= edge_set[i][0]; j++)
            if (existed_page[edge_set[i][j]] == 1)
                real_outd++;
        if (real_outd > 0)
            coeff[i] = (1.0 - alpha) * (1.0 / (float)real_outd);
        else
            coeff[i] = 0;
    }

    //初始化两个列向量，v_old相当于v0
    v_old = (float  *)malloc(sizeof (float ) * (all_url_list_size + 1));
    v_new = (float  *)malloc(sizeof (float  ) * (all_url_list_size + 1));
    for (i = 1; i <= all_url_list_size; i++)
    {
        if (existed_page[i] == 1)
            v_old[i] = 1.0;
        else
            v_old[i] = 0.0;
        v_new[i] = 0.0;
    }

    end_cal = 0;
    while (end_cal == 0)
    {
        cal_times++;
        end_cal = 1;
        for (i = 1; i <= all_url_list_size; i++)
        {
            for (j = 1; j <= edge_set[i][0]; j++)
                if (existed_page[edge_set[i][j]] == 1)
                    v_new[edge_set[i][j]] += coeff[i] * v_old[i];
        }
        x = 0;
        for (i = 1; i <= all_url_list_size; i++)
        {
            x += v_old[i] * alpha / existed_page_size;
        }
        for (i = 1; i <= all_url_list_size; i++)
        {
            if (existed_page[i] == 1)
                v_new[i] += x;
            if (fabs(v_new[i] - v_old[i]) > accuracy)
            {
                end_cal = 0;
                break;
            }
        }
        sum = 0;
        for (i = 1; i <= all_url_list_size; i++)
        {
            v_old[i] = v_new[i];
            v_new[i] = 0;
            if (existed_page[i] == 1)
                sum += v_old[i];
        }
        printf("finish %dth cal\n", cal_times);
    }
    //归一化
    for (i = 1; i <= all_url_list_size; i++)
        v_old[i] = v_old[i] / sum;
    printf("end_cal!\n");

    //计算入度的cdf
    printf("cal indcdf.dat\n");
    max_ind = -1;
    for (i = 1; i <= all_url_list_size; i++)
        if (existed_page[i] == 1 && ind[i] > max_ind)
            max_ind = ind[i];
    ind_cdf = (int *)malloc(sizeof (int ) * (max_ind + 1));
    for (i = 0; i <= max_ind; i++)
        ind_cdf[i] = 0;
    for (i = 1; i <= all_url_list_size; i++)
        if (existed_page[i] == 1)
            ind_cdf[ind[i]]++;
    fp = fopen("indcdf.dat", "w");
    tmp_num = 0;
    for (i = 1; i <= max_ind; i++)
    {
        tmp_num += ind_cdf[i];
        fprintf(fp, "%-20d%f\n", i, (float)tmp_num / (float)existed_page_size);
    }
    fclose(fp);
    printf("end cal indcdf.dat\n");

    //计算top10
    fp = fopen("top10.dat", "w");
    //计算top10ind
    printf("cal top 10 ind\n");
    left = 10;
    tmp_num = 0;
    fprintf(fp, "-----------------top 10 ind-------------------\n");
    for (i = max_ind; i >= 1 && left > 0; i--)
    {
        if (ind_cdf[i] != 0)
        {
            left -= ind_cdf[i];
            tmp_num++;
            top10ind[tmp_num] = i;
        }
    }
    for (i = 1; i <= tmp_num; i++)
    {
        for (j = 1; j <= all_url_list_size; j++)
        {
            if (ind[j] == top10ind[i])
                fprintf(fp, "%s    |   %d\n", all_url_list[j], top10ind[i]);
        }
    }
    printf("end cal top 10 ind\n");
    fprintf(fp, "\n");

    //计算top10 page_rank
    printf("cal top 10 page_rank!\n");
    fprintf(fp, "-----------------top 10 page_rank-------------------\n");
    first_max = 1.0;
    second_max = -1.0;
    tmp_num = 10;
    while (tmp_num > 0)
    {
        tmp_num--;
        pos = -1;
        for (i = 1; i <= all_url_list_size; i++)
        {
            if (existed_page[i] == 1 && v_old[i] > second_max && v_old[i] < first_max)
            {
                pos = i;
                second_max = v_old[i];
            }
        }
        if (pos > 0)
            fprintf(fp, "%s    |   %f\n", all_url_list[pos], v_old[pos]);
        first_max = second_max;
        second_max = -1.0;
    }
    fclose(fp);

    //生成graphviz脚本
    printf("create sitemap.dot\n");
    fp = fopen("sitemap.dot", "w");
    fprintf(fp, "digraph G {\n");
    for (i = 1; i <= all_url_list_size; i++)
    {
        if (existed_page[i] == 1)
        {
            fprintf(fp, "%s", all_url_list[i]);
            for (j = 1; j <= edge_set[i][0]; j++)
            {
                if (existed_page[edge_set[i][j]] == 1)
                    fprintf(fp, " -> %s", all_url_list[edge_set[i][j]]);
            }
            fprintf(fp, ";\n");
        }
    }
    fprintf(fp, "}");
    fclose(fp);
    printf("finish creating sitemap.dot\n");

    if (argc < 3)
        printf("not enough argv!\n");
    else
    {
        fp = fopen(argv[2], "r");
        if (fp == NULL)
            printf("fail to open %s\n", argv[2]);
        else
        {
            all_check_url_num = (int *)malloc(sizeof (int ) * all_url_list_size);
            checked_urls_size = 0;
            check_urls = (char **)malloc(sizeof (char *) * all_url_list_size);
            for (i = 0; i < all_url_list_size; i++)
                check_urls[i] = (char *)malloc(sizeof (char ) * 512);

            while (!feof(fp))
            {
                fscanf(fp, "%s", tmp);
                checked_urls_size++;
                strcpy(check_urls[checked_urls_size], tmp);
                start_pos = strstr(tmp, "/techqq");
                if (start_pos != NULL)
                {
                    all_check_url_num[checked_urls_size] = NOT_IN_SET;
                    for (i = 1; i <= all_url_list_size; i++)
                        if (strcmp(start_pos, all_url_list[i]) == 0 && existed_page[i] == 1)
                        {
                            all_check_url_num[checked_urls_size] = i;
                            break;
                        }
                }
                else
                    all_check_url_num[checked_urls_size] = NOT_IN_SET;
            }
            fclose(fp);

            fp = fopen("checkresult.dat", "w");
            if (fp == NULL)
                printf("fail to open checkresult.dat\n");
            else
            {
                for (i = 1; i <= checked_urls_size; i++)
                {
                    if (all_check_url_num[i] != NOT_IN_SET)
                        fprintf(fp, "%s\t\t%d\t\t%f\n", check_urls[i], ind[all_check_url_num[i]], v_old[all_check_url_num[i]]);
                    else
                        fprintf(fp, "%s not found!\n", check_urls[i]);
                }
                fclose(fp);
            }
            for (i = 0; i <= all_url_list_size; i++)
                free(check_urls[i]);
            free(check_urls);
        }
    }
}

void* do_crawler(void *arg) //完成爬取线程的核心流程
{
    struct para_for_crawler *para = (struct para_for_crawler *)arg;
    struct web_graph *webg = para->webg;
    urlq_t *queue = para->queue;
    tpool_t *tpool = para->tpool;

    int client_fd = -1;
    char *http_response = NULL;
    char **url_list = NULL;
    int url_list_size = 0;
    int i = 0, j;
    int num = 0, cur_num = 0;
    int *out_link = NULL;//存放当前这个url的边集（这个数组里面存放的是目标顶点编号）
    char *cur_url = NULL;
    int try_times = 0;//获取网页失败之后，尝试3次，如果3次都还是获取不了，就说明不行，放弃尝试
    char *cur_path = NULL;

    url_list = (char **)malloc(sizeof (char *) * URL_LIST_LEN);
    for (i = 0; i < URL_LIST_LEN; i++)
        url_list[i] = (char *)malloc(sizeof (char) * LINK_LEN);//表大小为5000*512

    cur_url = (char *)malloc(sizeof (char ) * LINK_LEN);
    cur_path = (char *)malloc(sizeof (char ) * LINK_LEN);

    while (1)
    {
        /*如果线程池没有被销毁且没有任务要执行，则挂起线程*/
        pthread_mutex_lock(&tpool->lock);
        while (queue->size == 0 && tpool->shutdown == 0)
        {
            tpool->at_work--;
            pthread_cond_wait(&tpool->cond, &tpool->lock);//挂起前会解锁,唤醒后会重新加锁
            tpool->at_work++;
        }
        if (tpool->shutdown)
        {
            tpool->at_work--;
            pthread_mutex_unlock(&tpool->lock);
            pthread_exit(NULL);
        }
        pthread_mutex_unlock(&tpool->lock);

        /*这里需要上锁，因为需要对队列进行改动*/
        pthread_mutex_lock(&tpool->lock);
        cur_num = queue_pop(queue);
        pthread_mutex_unlock(&tpool->lock);
        /*取得队列中的url后解锁*/

        /*因为调试的时候，发现cur_num超出了all_url_list_size的范围，所以加个这个判断，跳过这个bug*/
        if (cur_num > webg->all_url_list_size)
        {
            printf("-----------------------cur_num over url list size!---------------------------------\n");
            continue;
        }
        if (cur_num == -1)
        {
            printf("--------------------empty_queue!-------------------\n");
            continue;
        }
        /*从图中根据编号获得url的具体内容*/
        strcpy(cur_url, webg->all_url_list[cur_num]);
        if (strlen(cur_url) >= LINK_LEN)
        {
            printf("fail:::too long link!\n len: %d\n", strlen(cur_url));
            printf("long url: %s\n", cur_url);
            continue;
        }

        for (i = strlen(cur_url) - 1; i >= 0; i--)
            if (cur_url[i] == '/')
                break;
        for (j = 0; j <= i; j++)
            cur_path[j] = cur_url[j];
        cur_path[i+1] = '\0';

        /*这部分工作不需要锁，不是临界资源*/

        /*根据url获取网页内容*/
        //一共有3次尝试机会
        try_times = 3;
        http_response = NULL;
        while (try_times != 0 && http_response == NULL)
        {
            client_fd = open_tcp(SITE_IP, SITE_PORT);
            if (client_fd < 0)
            {
                printf("fail to open tcp!\n");
                try_times--;
                continue;
            }
            http_response = http_do_get(client_fd, cur_url);
            try_times--;
            close(client_fd);
        }
        /*client_fd = open_tcp(SITE_IP, SITE_PORT);
        if (client_fd < 0)
        {
            printf("fail to open tcp!\n");
            continue;
        }
        http_response = http_do_get(client_fd, cur_url);
        close(client_fd);*/

        //3次尝试机会还是为NULL，那就说明这个网页应该是抓不到了，不管它了
        if (http_response == NULL || strlen(http_response) < 10)//接收超时了, 或者是接收回来的长度小于10，遇到过接收回来长度为0
        {
            printf("fail to fetch this page: %s\n", cur_url);
            continue;
        }

        if (http_response_status(http_response) == 200)
        {
            webg->existed_page[cur_num] = 1;
            pthread_mutex_lock(&tpool->lock);
            webg->existed_page_size++;
            pthread_mutex_unlock(&tpool->lock);

            url_list_size = extract_link(http_response, url_list, cur_url, cur_path);//获得url_list及其大小
            if (url_list_size > 4000)
            {
                printf("url_list_size > 4000!\nurl_list_size: %d\n", url_list_size);
                free(http_response);
                continue;
            }
            //将url_list中的url放入点集
            out_link = (int *)malloc(sizeof (int ) * url_list_size);
            if (out_link == NULL)
            {
                printf("malloc out_link fail!\n");
                continue;
            }

            /*这段涉及到对队列和图的修改，所以需要上锁*/
            for (i = 0; i < url_list_size; i++)
            {
                if (strlen(url_list[i]) >= LINK_LEN)
                {
                    printf("-----------fail::too long url!\n url_len: %d\nurl: %s\n-----------\n", strlen(url_list[i]), url_list[i]);
                    i++;
                    continue;
                }
                pthread_mutex_lock(&tpool->lock);
                if ((num = has_vertex(webg, url_list[i])) == NOT_IN_SET)//表示这个url不在点集中，这里还需要添加将这个url入队列
                {
                    num = insert_vertex(webg, url_list[i]);
                    if (num == -1)
                        printf("fail to insert!\n");
                    if (num > 400000)//防止错误的数插到队列里面去
                    {
                        printf("fail: wrong number to be inserted into the graph\n");
                        i++;
                        pthread_mutex_unlock(&tpool->lock);
                        continue;
                    }
                    queue_push(queue, num);
                    if (tpool->at_work < tpool->max_thr_num)//这里特别重要，不是什么时候都能signal的，如果线程都在工作的话，这个signal就会丢失
                        pthread_cond_signal(&tpool->cond);
                }
                pthread_mutex_unlock(&tpool->lock);
                out_link[i] = num;//将目标顶点的编号存放到这个url的边集里面去
            }

            /*因为每个线程所要处理的url不同，即cur_url都不同，所以对边集的操作就不同，所以这里对边集的操作不需要加锁*/
            num = remove_duplicate(out_link, url_list_size);//返回去重后的顶点个数
            webg->edge_set[cur_num] = (int *)malloc(sizeof (int ) * (num + 1));//给当前url的边集分配空间
            if (webg->edge_set[cur_num] == NULL)
            {
                printf("-----------------------------fail to malloc space for edge set for %d  url\n", cur_num);
                free(out_link);
                continue;
            }
            webg->edge_set[cur_num][0] = num;
            for (i = 0; i < num; i++)
            {
                pthread_mutex_lock(&tpool->lock);
                webg->ind[out_link[i]]++;//统计入度
                webg->edge_set_size++;
                pthread_mutex_unlock(&tpool->lock);

                insert_edge(webg, cur_num, out_link[i], i+1);//i+1是因为这个边集数组里面，第0号已经存了出度了
            }

            //释放edge占用的内存
            free(out_link);
            out_link = NULL;
        }
        free(http_response);//这里记得将http_response释放掉，这个很耗内存的
        http_response = NULL;
    }

    //释放url_list所占用的内存
    for (i = 0; i < URL_LIST_LEN; i++)
        free(url_list[i]);
    free(url_list);
    url_list = NULL;
    free(cur_url);
}

int remove_duplicate(int *out_link, int size)
{
    int cur, cur_pos, i;

    qsort(out_link, size, sizeof (int ), comp);
    cur = -1;//因为点的编号不可能存在-1
    cur_pos = -1;
    for (i = 0; i < size; i++)
    {
        if (out_link[i] != cur)
        {
            cur_pos++;
            cur = out_link[i];
            out_link[cur_pos] = cur;
        }
    }
    return cur_pos + 1;
}

int comp(const void *a,const void *b)
{
    return *(int *)a-*(int *)b;
}

#define MAX_THREADS 200

int main(int argc, char *argv[])
{
	struct web_graph webg;
	int i;
	urlq_t queue;
	int num;
	tpool_t *tpool;
    printf("Crawling on %s, result will be in %s, Processing...\n", argv[1], argv[2]);
	if (init_webg(&webg) == INIT_FAIL)
	{
		exit(1);
	}
	if (queue_init(&queue) == INIT_QUEUE_FAIL)
	{
		exit(1);
	}

	if ((tpool = tpool_create(MAX_THREADS, &webg, &queue)) == NULL)
	{
		exit(1);
	}

	pthread_mutex_lock(&tpool->lock);
	num = insert_vertex(&webg, "");
	queue_push(&queue, num);
	pthread_mutex_unlock(&tpool->lock);
	pthread_cond_signal(&tpool->cond);

	while (queue.size > 0 || tpool->at_work != 0)
	{
        NULL;
	}
	tpool_destroy();
	output_result_file(argc, argv);
	destroy_webg(&webg);
}
