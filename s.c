# include <time.h>
# include <errno.h>
# include <fcntl.h>
# include <stdio.h>
# include <stdlib.h>
# include <string.h>
# include <signal.h> 
# include <unistd.h>
# include <pthread.h>
# include <sys/shm.h>
# include <sys/ipc.h>
# include <sys/msg.h>
# include <sys/stat.h>
# include <semaphore.h>
# include <sys/types.h>
# include <sys/socket.h>
# include <netinet/in.h>
# define SPIN 180
//好友结构体
struct Friend{
	int len;
	int state;
	char chatMsg[128][128];
	char fname[32];
}; 
//结构体
struct User{
	char name[32];
	char password[32];
	char telenumber[16];
	//状态信息 
	time_t start;			
	time_t time; 			//登入时长 
	int num;				//好友个数 
	int state;				//在线状态 
	int sockfd;				//sockfd号 
	int gState;				//群聊状态 
	char pid[32];			//登入进程 
	float sum;				//账户余额 
	//char friends[100][32];	
	//好友数组 
	struct Friend friends[32];
	int addNum;				//未读消息 
	char addMsg[1024][32];	//添加请求 
	char addName[1024][32];	//添加好友 
	
	int len;
	char hisMsg[1024][128];//未读消息 
	//sem[0]andmsg[0]接收选项消息 
	sem_t sem[1024];		//信号量
	char msg[1024][1024];	//消息记录 
	//第0位用于存放选项数据 
}; 

struct Redp{
	time_t time;		//创建时间 
	
	int num;			//拆分个数 
	int len;			//抢过的用户数目 
	float sum;			//红包余额 
	char bless[32];		//红包祝语 
	char user[1024][32];	//抢过的用户 
	char timeMsg[32];	//时间信息 
	struct User host;	//红包主人 
};

//线程参数:解决线程创建时只能传一个参数的问题 
struct Para{
	pthread_t id;
	char name[32];
};


//链表节点 
typedef struct LnodeU{
	struct User user;
	struct LnodeU *next;
}LnodeU,*LinklistU;

typedef struct LnodeR{
	struct Redp redp;
	struct LnodeR *next;
}LnodeR,*LinklistR;

//--------------------------全局变量--------------------------------------- 
LinklistU U;	//名媛链表头节点 
LinklistR R;	//红包链表头节点 
//------------------------------------------------------------------------- 

//==========================函数声明=======================================
//菜单类函数 
void reactReg(int);				//回应注册
void reactLog(int); 			//回应登入
void reactDes(int);				//销毁用户 
void reactUserMenu(int); 		//回应用户菜单 
void reactMainMenu(int *);		//回应主页面菜单 
//功能 
void priChat(int);				//私聊 
void groChat(int);				//群聊 
void rtChat(int *);				//实时对话 
void reactAdd(int);				//处理添加 
void reactSet(int);				//强制修改 
void listPack(int);				//罗列红包 
void reactPush(int);			//用户充值
void reactTran(int);			//用户转账 
void reactSend(int);			//发送红包 
void InitLink(void);			//初始链表
void readFile(void);			//读取文件 
void listUser(void);			//罗列用户
void writeFile(char);			//写入文件
void listFriends(int);			//输出闺蜜
void listHistory(int);			//输出消息 
void reactMakeFriends(int); 	//处理添加 
void reactMsg(struct Para *);	//处理消息
void addNode(char,struct User,struct Redp);//添加节点 
//方法
int findName(char[]);				//查找用户
int findState(char[]);				//查询状态
int findSockfd(char[]);				//查询sock 
int findChatState(char[],char[]);	//查询状态 
char *myRecv(int);					//封装接收
char *findPwd(char[]);				//查找密码 
void rmSem(int);					//消除信号 
void Inspection(void); 				//检测红包效期 
void setGState(int,int);			//修改群聊状态 
void setState(char[],int);			//修改状态 
void setPid(char[],char[]);			//修改进程号 
void setSockfd(char[],int);			//修改sock号 
void saveHis(char[],char[]);		//保存his 
void setChatState(char[],int);		//聊天状态位 
void rmArr(char[][32],int,int);		//移除元素 
void saveMsg(char[],char[],char[]);	//保存msg 
void addTime(char[],time_t,time_t);	//设置时间 
//========================================================================= 

int main(int argc,char *argv[])
{
	int *p;
	int sockfd;		//存放服务进程的sockfd
	int newSockfd;	//与新客户端建立连接的sockfd号 
	
	pthread_t id;	//线程号 
	
	//套接字链接的准备工作 
	int addrlen;
	struct sockaddr_in serv_addr,my_addr;
    serv_addr.sin_port = atoi(argv[1]);
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    memset(serv_addr.sin_zero,0,8);
     
    //初始化链表
	InitLink();
	//读取本地文件
	readFile();
    
    printf("%s","\033[1H\033[2J"); 
    printf("---服务器启动---\n"); 
    //获取sockfd[服务进程的sockfd:无用] 判断sockfd 
    sockfd = socket(AF_INET,SOCK_STREAM,0);
	if(sockfd<0)
		perror("socket");
	//绑定
	if(bind(sockfd,(struct sockaddr *)&serv_addr,sizeof(struct sockaddr))<0)
		perror("bind");
	//监听
	if(listen(sockfd,100)<0)
		perror("listen");
	//等待连接 
	while(1){
		newSockfd = accept(sockfd,(struct sockaddr *)&my_addr,&addrlen);
		if(newSockfd<0)
            perror("accept");
        printf("#system msg:\t客户端%d",newSockfd);
	    printf(" \033[32m已开启连接\033[0m\n");
        p = (int *)malloc(sizeof(int));
		*p = newSockfd;
		//p = &newSockfd;
		//printf("main id=%lu\n",id);
		pthread_create(&id,NULL,(void *)reactMainMenu,(void *)p); 
	}  
	pthread_join(id,NULL);  
	
	return 0;
}

void reactMainMenu(int *sockfd)
{
	//对客户端的主菜单做出回应 
	int len;
	char buf[1024];
	
	//printf("该线程为%d客户端服务\n",*sockfd);
	while(1){
		//printf("主函数\n");
		memset(buf,0,sizeof(buf));
		//接收选项 
		strcpy(buf,myRecv(*sockfd));
		
		//判断选项 
		if(strcmp(buf,"1")==0){
			reactReg(*sockfd);
		}else if(strcmp(buf,"2")==0){
			reactLog(*sockfd);
		}else if(strcmp(buf,"3")==0){
			//reactSet();
		}else if(strcmp(buf,"4")==0){
			reactDes(*sockfd); 
		}else if(strcmp(buf,"5")==0){
			//结束该客户端
			close(*sockfd);
	        printf("#system msg:\t客户端%d",*sockfd);
	        printf(" \033[33m已断开连接\033[0m\n");
	        pthread_exit(0);
		}else if(strcmp(buf,"ls")==0){
			listUser();
		}else if(strcmp(buf,"set")==0){
			//设置状态
			reactSet(*sockfd);
		}
	}
	return;
}

void reactReg(int sockfd)
{
	int i,num;
	char buf[32];
	struct User user;
	struct Redp empty;
	//接收合理的手机号 
	strcpy(user.telenumber,myRecv(sockfd));
	
	//获取验证码并发送 
	while(1){
		srand(time(NULL));
		num = rand()%9000+1000;
		sprintf(buf,"%d",num);//转化成char类型 
		if(send(sockfd,buf,8,0)<0)
			perror("send");
		
		//接收比对消息 
		strcpy(buf,myRecv(sockfd));
		
		//退出条件 
		if(strcmp(buf,"same")==0)
			break;
	}
	//接收用户名 
	while(1){
		strcpy(user.name,myRecv(sockfd));
		
		if(findName(user.name)==0){
			if(send(sockfd,"null",32,0)<0)
				perror("send");
			break;
		}
		if(send(sockfd,"exist",32,0)<0)
			perror("send");
	}
	strcpy(user.password,myRecv(sockfd));

	if(strcmp(user.password,"system[]exit")==0)
		return;
	
	if(findName(user.name)==1){
		if(send(sockfd,"exist",32,0)<0)
			perror("send");
		return;
	}
	
	//添加新用户操作
	user.num = 0;
	user.len = 0;
	user.sum = 0;
	user.time = 0;
	user.start = 0;
	user.state = 0;
	user.addNum = 0;
	user.gState = 0;
	user.sockfd = -1;
	strcpy(user.pid,"-1");
	for(i=0;i<1024;i++){
		sem_init(&user.sem[i],0,0);
		strcpy(user.msg[i],"");
		strcpy(user.addName[i],"");
		strcpy(user.hisMsg[i],"");
		//strcpy(user.addMsg[i],"");
	}
	memset(user.addMsg,0,1024);
	addNode('U',user,empty);
	writeFile('U');
	
	//回馈结果
	if(send(sockfd,"success",32,0)<0)
		perror("send");
	
	printf("#system msg:%s名媛已加入\n",user.name);
	return;
}

void reactLog(int sockfd)
{
	int i;
	pthread_t id,selfId;
	LinklistU u = U->next;
	struct Para *temp;
	char buf[32],pwd[32],name[32],pid[32];
	while(1){
		//recv name
		strcpy(name,myRecv(sockfd));

		//send match
		if(findName(name)==1){
			if(send(sockfd,"exist",32,0)<0)
				perror("send");
			break;
		}
		if(send(sockfd,"null",32,0)<0)
			perror("send");
	}
	for(i=0;i<3;i++){
		strcpy(pwd,myRecv(sockfd));

		if(strcmp(pwd,findPwd(name))==0){
			if(send(sockfd,"consistent",32,0)<0)
				perror("send");
			break;
		}
		if(send(sockfd,"inconsistent",32,0)<0)
			perror("send");
	}
	if(i==3)
		return;
		
	
	//返回在线状态
	if(findState(name)){
		if(send(sockfd,"on-line",32,0)<0)
			perror("send");
		return;
	}else if(!findState(name)){
		if(send(sockfd,"off-line",32,0)<0)
			perror("send");
	}else{
		if(send(sockfd,"abn",32,0)<0)
			perror("send");
		return;
	}
 
 	
	//Step1：接收该用户登入时对应客户端的进程号 
	addTime(name,0,time(NULL));
	strcpy(pid,myRecv(sockfd));
	setPid(name,pid);		//修改用户进程号 
	setState(name,1);		//修改登入状态 
	setSockfd(name,sockfd);	//修改sockfd 
	setChatState(name,0);	//设置聊天状态位 
	writeFile('U');			//写入文件 
	
	//Step2：准备线程参数结构体(linux中结构体指针需要开辟空间才能使用)
	temp = (struct Para *)malloc(sizeof(struct Para));
	strcpy(temp->name,name);//写入用户名
	temp->id = pthread_self();//写入子线程线程号 
	//printf("初始id = %lu\n",pthread_self());
	
	//Step3：开启响应消息的线程 并传入参数 
	pthread_create(&id,NULL,(void *)reactMsg,(void *)temp);
	
	//Step4：进入处理用户菜单函数 
	reactUserMenu(sockfd);
	
	//Step5：从用户菜单退出后的一系列下线操作
	/*[我觉的吧，写到reactUserMenu()函数里会好规范一点，
	但是要改函数参数，我懒，就算了] */ 
	addTime(name,time(NULL),0);
	setGState(sockfd,0);	//设置群聊下线 
	setState(name,0);		//将登录状态设置位离线 
	setPid(name,"-1");		//将用户进程号置为-1 
	setSockfd(name,-1);		//将用户sockfd设置为-1 
	writeFile('U');			//写入本地文件 
	pthread_cancel(id);		//关闭接收线程
	
	//Step6：提示退出 
	printf("#system msg:\t\33[34m%s\33[0m 名媛已退出登入！\n",name);
	
	//退回主菜单 
	return;
}

//销毁用户
void reactDes(int sockfd)
{
	int i,j;
	char name[32],buf[32],msg[128];
	LinklistU u = U->next,p=U,q=U;
	LinklistR r = R->next;
	if(recv(sockfd,name,32,0)<0)
		perror("recv_name");
	
	if(findName(name)==0){
		if(send(sockfd,"unexist",32,0)<0)
			perror("send");
		return;
	}else if(findState(name)!=0){
		if(send(sockfd,"on_line",32,0)<0)
			perror("send");
		return;
	}
	if(send(sockfd,"ok",32,0)<0)
		perror("send");
		
	if(recv(sockfd,buf,32,0)<0)
		perror("recv");
	
	if(strcmp(buf,"root")!=0)
		return;
		
	while(u){
		if(strcmp(u->user.name,name)==0){
			
			//关闭好友关系
			for(i=0;i<u->user.num;i++){
				q = U;
				while(q){
					if(strcmp(q->user.name,u->user.friends[i].fname)==0){
						for(j=0;j<q->user.num;j++){
							if(strcmp(q->user.friends[j].fname,u->user.name)==0){
								//输出提示 
								//#类型的hisMsg 
								strcpy(msg,"# ");
								strcat(msg,u->user.name);
								strcat(msg,"名媛 已注销账户");
								strcpy(q->user.hisMsg[q->user.len++],msg); 
								for(;j<q->user.num-1;j++){
									q->user.friends[j]=q->user.friends[j+1];
								}
								q->user.num--;
								break;
							}
						}
						break;
					}
					q = q->next;
				}
			} 
			
			//让所有红包失效
			while(r){
				if(strcmp(r->redp.host.name,u->user.name)==0){
					r->redp.time = 0;
					r->redp.sum = 0;
				}
				r = r->next;
			} 
			writeFile('R');
			p->next = u->next;
			free(u);
			writeFile('U'); 
			if(send(sockfd,"ok",32,0)<0)
				perror("send");
			return;
		}
		u = u->next;
		p = p->next;
	}
	
	if(send(sockfd,"error",32,0)<0)
		perror("send");
	
	return;
}			 

//处理消息 
void reactMsg(struct Para *temp)
{
	//int opt=1;
	int i,j,k,len,sockfd;
	char buf[1024],msg[1024];
	LinklistU u = U->next;
	//查找用户，执行相应操作 
	while(u){
		//判断节点是否为所需节点 
		if(strcmp(u->user.name,temp->name)==0){
			//找到节点后,先将sockfd读取出来 
			sockfd = u->user.sockfd;
			//循环接收消息 
			while(1){
				//清空buf[可以但，我觉得没必要] 
				memset(buf,0,sizeof(buf));
				//获取消息 
				len = recv(u->user.sockfd,buf,1024,0);
				//printf("sockfd\tbuf\tlen\n");
				//printf("%d\t|%s|\t%d\n",u->user.sockfd,buf,len);
				//疑问：如果被set-1的时候不就收不到了吗
				//等于-1的时候 len就会是-1了所以可以len<0的判断体里对强制下线做回应 
				if(len<0){ 
					//perror("#reactMsg_recv");我有自己的提示了，这句就可以不用了 
					//方法三：被强制下线的时候，我可以杀死他的进程
					//setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, (const void *)&opt, sizeof(opt));
					//printf("len<0\n");
					//Step1：关闭端口 
					close(sockfd);//另一种写法：shutdown(sockfd,SHUT_RDWR);
					
					//Step2：准备杀死客户端线程指令
					strcpy(buf,"kill ");
					strcat(buf,u->user.pid);
				
					//Step3：结束客户端的进程
					system(buf); 
					
					//Step4：对当前用户执行下线操作 
					u->user.state = 0;
					u->user.gState = 0;
					
					//printf("设置下线\n");
					for(k=0;k<u->user.num;k++){
						u->user.friends[k].state=0;
					}
					setSockfd(u->user.name,-1);
					setPid(u->user.name,"-1");
					if(u->user.start>0){
						u->user.time = u->user.time+time(NULL)-u->user.start;
						u->user.start=0;
					}
					writeFile('U');//保存到本地 
					//printf("%s->%d\n",u->user.name,u->user.gState);
					//Step4：关闭子线程 
					pthread_cancel(temp->id);
					
					//Step5：提示输出 
					printf("#system msg:\t\33[34m%s\33[0m 名媛已被强制下线！\n",u->user.name);
			        printf("#system msg:\t客户端%d",sockfd);
			        printf(" \033[33m已断开连接\033[0m\n");
			        
			        //Step6：关闭该线程/接收线程/孙线程/reactMsg 
					pthread_exit(NULL);
				}else if(len == 0){
					//执行下线操作
					for(k=0;k<u->user.num;k++){
						u->user.friends[k].state=0;
					}
					u->user.gState = 0;
					if(u->user.start>0){
						u->user.time = u->user.time+time(NULL)-u->user.start;
						u->user.start=0;
					}
					setState(u->user.name,0);
					setSockfd(u->user.name,-1);
					setPid(u->user.name,"-1");
					writeFile('U'); 
					//关闭子线程 
					pthread_cancel(temp->id);
					//printf("id = %lu\n",temp->id);
					
					//关闭端口 做相应的提示输出 
					close(sockfd);
					printf("#system msg:\t\33[34m%s\33[0m 名媛强制下线！\n",u->user.name);
			        printf("#system msg:\t客户端%d",sockfd);
			        printf(" \033[33m已断开连接\033[0m\n");
			        
			        //关闭当前线程 
					pthread_exit(NULL);
				}
				
				//处理消息
				j = strlen(buf);
				memset(msg,0,1024);
				for(i=1;i<j;i++){
					msg[i-1] = buf[i];
				} 
				//比对消息类型 
				if(strcmp(buf,"强制下线客户端的接口")==0){
					//好像只能通过发消息来控制客户端 
					//有什么办法让他直接回到userMenu 
					//目前没有哈 
					//其实也是有的，把客户端的进程结束掉，重新建立连接 
				}else if(buf[0]=='c'){
					//选项类消息 
					strcpy(u->user.msg[0],msg);
					sem_post(&u->user.sem[0]);
				}else if(buf[0]=='f'){
					//金额类数据
					strcpy(u->user.msg[3],msg);
					sem_post(&u->user.sem[3]); 
				}else if(buf[0]=='n'){
					//名字类消息
					strcpy(u->user.msg[1],msg);
					sem_post(&u->user.sem[1]); 
				}else if(buf[0]=='m'){
					//验证消息
					strcpy(u->user.msg[2],msg);
					sem_post(&u->user.sem[2]);
				}else if(buf[0]=='i'){
					//验证消息
					strcpy(u->user.msg[4],msg);
					sem_post(&u->user.sem[4]);
				}else if(buf[0]=='#'){
					if(send(sockfd,msg,128,0)<0)
						perror("send_showMsg");
				}else{
					//wait
					//printf("buf = %s\n",buf);
					printf("#system msg:undefined message\n");
					
				}
			}
			break;
		}
		u = u->next;
	} 
	if(!u){
		printf("线程加载失败！\n");
		pthread_cancel(temp->id);
	}
}

//回应用户菜单 
void reactUserMenu(int sockfd)
{
	int i,j;
	int *p;
	char buf[1024],temp[32],num[32];
	LinklistU u = U->next;
	/*
	测试子线程运行状态，来判断是否有被及时关闭 
	while(1){
		sleep(3);
		printf("子线程ing...\n");
	}
	*/
	while(u){
		if(u->user.sockfd==sockfd){
			//消除残留标志
			rmSem(sockfd); 
			/*
			p = (int *)malloc(sizeof(int));
			sem_getvalue(&u->user.sem[0],p);//获取sem值
			while(*p){
				sem_wait(&u->user.sem[0]);//若有值则消耗
				*p = *p-1;
			}
			sem_getvalue(&u->user.sem[1],p);//获取sem值
			while(*p){
				sem_wait(&u->user.sem[1]);//若有值则消耗
				*p = *p-1;
			}
			*/
			
			while(1){
				//发送未读消息数量 
				sprintf(buf,"%d",u->user.addNum);
				if(send(sockfd,buf,32,0)<0)
					perror("send_addNum");
				sprintf(buf,"%d",u->user.len);
				if(send(sockfd,buf,32,0)<0)
					perror("send_len");
				//等待消息信号量 
				sem_wait(&u->user.sem[0]);
				//获取接收到的消息 
				strcpy(buf,u->user.msg[0]);
				if(strcmp(buf,"1")==0){
					//大量用户数据的时候，建议把所需数据直接传过去 
					listFriends(sockfd);
				}else if(strcmp(buf,"2")==0){
					priChat(sockfd);
				}else if(strcmp(buf,"3")==0){
					groChat(sockfd);
				}else if(strcmp(buf,"4")==0){
					reactTran(sockfd);
				}else if(strcmp(buf,"5")==0){
					reactPush(sockfd);
				}else if(strcmp(buf,"6")==0){
					reactSend(sockfd);
				}else if(strcmp(buf,"7")==0){
					listPack(sockfd);
					reactGrab(sockfd);
				}else if(strcmp(buf,"8")==0){
					reactMakeFriends(sockfd);
				}else if(strcmp(buf,"10")==0){
					Inspection();
					sprintf(temp,"%.2f",u->user.sum);
					if(send(sockfd,temp,32,0)<0)
						perror("send_sum");
				}else if(strcmp(buf,"exit")==0){
					return;
				}else if(strcmp(buf,"#")==0){
					sprintf(num,"%d",u->user.addNum);
						if(send(sockfd,num,32,0)<0)
					perror("send_addNum");
					if(u->user.addNum==0)
						continue;
					reactAdd(sockfd);
				}else if(strcmp(buf,"@")==0){
					sprintf(num,"%d",u->user.len);
					if(send(sockfd,num,32,0)<0)
						perror("send_len");
					if(u->user.len==0)
						continue;
					listHistory(sockfd);
				}else if(strcmp(buf,"ls")==0){
					listUser();
				}else if(strcmp(buf,"show")==0){
					printf("%s","\033[1H\033[2J"); 
					for(i=0;i<u->user.num;i++){
						for(j=0;j<u->user.friends[i].len;j++){
							printf("%s\n",u->user.friends[i].chatMsg[j]);
						}
					}
				}else if(strcmp(buf,"gState")==0){
					printf("%s.gState=%d\n",u->user.name,u->user.gState); 
				}
				Inspection();
			}
			return; 
		}
		u = u->next;
	}
	if(!u){
		printf("# system msg:端口异常\n");
		return;
	}
	return;
}

//+++++++++++++++++++++++++++++++++方法函数++++++++++++++++++++++++++++++++
//封装接收函数 
char *myRecv(int sockfd)
{
	int len;
	static char buf[1024];
	LinklistU u = U->next;
	memset(buf,0,sizeof(buf));
	//接收内容 
	len = recv(sockfd,buf,1024,0);
	if(len<0)
		perror("recv");
	else if(len==0){
		//若当前进程有用户登入，需要强制下线
		while(u){
			if(u->user.sockfd==sockfd){
				u->user.state=0;
				u->user.sockfd=-1;
				u->user.gState = 0;
				if(u->user.start>0){
					u->user.time = time(NULL)+u->user.time-u->user.start;
					u->user.start = 0;
				}
				//printf("设置下线\n");
				writeFile('U');
				//printf("%s->%d\n",u->user.name,u->user.gState);
				break;
			}
			u = u->next;
		}
		 
		close(sockfd);
	    printf("#system msg:\t客户端%d",sockfd);
	    printf(" \033[33m已断开连接\033[0m\n");
        pthread_exit(0);
	}
	return buf;
}

//查找用户 
int findName(char name[])
{
	LinklistU u = U->next;
	while(u){
		if(strcmp(u->user.name,name)==0)
			return 1;
		u = u->next;
	}
	return 0;
}

//查询密码
char *findPwd(char name[])
{
	LinklistU u = U->next;
	while(u){
		if(strcmp(u->user.name,name)==0)
			return u->user.password;
		u = u->next;
	}
	return "";
}

//查询状态 
int findState(char name[])
{
	LinklistU u = U->next;
	while(u){
		if(strcmp(u->user.name,name)==0)
			return u->user.state;
		u = u->next;
	}
	return -1;
}

//查询聊天状态
int findChatState(char fname[],char dname[])
{
	int i;
	LinklistU u = U->next;
	while(u){
		if(strcmp(fname,u->user.name)==0){
			for(i=0;i<u->user.num;i++){
				if(strcmp(u->user.friends[i].fname,dname)==0)
					return u->user.friends[i].state;
			}
			return -1;
		}
		u = u->next;
	} 
	return -1;
}

//获取sockfd号 
int findSockfd(char name[])
{
	LinklistU u = U->next;
	while(u){
		if(strcmp(u->user.name,name)==0)
			return u->user.sockfd;
		u = u->next;
	}
	return -1;
}

//修改状态
void setState(char name[],int state)
{
	LinklistU u = U->next;
	while(u){
		if(strcmp(u->user.name,name)==0){
			u->user.state = state;
			if(state==0){
				if(u->user.start>0){
					u->user.time = u->user.time+time(NULL)-u->user.start;
					u->user.start = 0;
					writeFile('U');
				}
			}
			return;
		}
		u = u->next;
	}
	return;
}

//修改群聊状态
void setGState(int sockfd,int state)
{
	LinklistU u = U->next;
	while(u){
		if(u->user.sockfd==sockfd){
			u->user.gState = state;
			return;
		} 
		u = u->next;
	}
	return;
}

//修改sockfd 
void setSockfd(char name[],int sockfd)
{
	LinklistU u = U->next;
	while(u){
		if(strcmp(u->user.name,name)==0){
			//if(u->user.state==1)
			//	close(u->user.sockfd);
			u->user.sockfd = sockfd;
			return;
		}
		u = u->next;
	}
	return;
}

//修改进程号
void setPid(char name[],char pid[])
{
	LinklistU u = U->next;
	while(u){
		if(strcmp(u->user.name,name)==0){
			strcpy(u->user.pid,pid);
			return;
		}
		u = u->next;
	}
	return;
}

//聊天状态位全部置零 
void setChatState(char name[],int e)
{
	int i;
	LinklistU u = U->next;
	while(u){
		if(strcmp(u->user.name,name)==0){
			for(i=0;i<u->user.num;i++){
				u->user.friends[i].state=e;
			}
			return;
		}
		u = u->next;
	}
	return;
}

//设置登入时间
void addTime(char name[],time_t end,time_t start)
{
	LinklistU u = U->next;
	while(u){
		if(strcmp(u->user.name,name)==0){
			if(end == 0)
				u->user.start = start;
			else{
				if(u->user.start>0){
					u->user.time = u->user.time+end-u->user.start;
					u->user.start=0;
				}
			}
			return;
		}
		u = u->next;
	}
	return;
}

//移除数组元素 
void rmArr(char arr[][32],int index,int len)
{
	for(;index<len-1;index++)
		strcpy(arr[index],arr[index+1]);
	return;
}

//消除信号 
void rmSem(int sockfd)
{
	int *p,i;
	LinklistU u = U->next;
	while(u){
		if(u->user.sockfd==sockfd){
			for(i=0;i<1024;i++){
				p = (int *)malloc(sizeof(int));
				sem_getvalue(&u->user.sem[i],p);//获取sem值
				while(*p){
					sem_wait(&u->user.sem[i]);//若有值则消耗
					*p = *p-1;
				}
			}
			writeFile('U');
			return;
		}
		u = u->next;
	}
	return;
}

//保存消息
void saveMsg(char name[],char fname[],char msg[])
{
	int i,j,k;
	LinklistU u = U->next;
	while(u){
		if(strcmp(u->user.name,name)==0){
			for(i=0;i<u->user.num;i++){
				if(strcmp(u->user.friends[i].fname,fname)==0){
					strcpy(u->user.friends[i].chatMsg[u->user.friends[i].len++],msg);					
					if(u->user.friends[i].len>100){
						for(j=0;j<10;j++)
							strcpy(u->user.friends[i].chatMsg[j],u->user.friends[i].chatMsg[u->user.friends[i].len-10+j]);
						u->user.friends[i].len=10;
					}
				 return;
				}		
			}
			return;
		}
		u=u->next;
	}
	return;
}

void saveHis(char name[],char msg[])
{
	LinklistU u = U->next;
	while(u){
		if(strcmp(u->user.name,name)==0){
			//当未读消息超额时，如何清理
			strcpy(u->user.hisMsg[u->user.len++],msg); 
			return;
		}
		u = u->next;
	}
	return;
}

//haveuser抢过
//time = 0过期 
//sum = 0抢完了

//查询余额的时候需要退回
//拆红包的时候退回
//列红包的时候退回 
//检测红包效期
void Inspection()
{
	char msg[128],sum[32];
	LinklistR r = R->next;
	LinklistU u = U->next;
	while(r){
//测试点:3分钟退回 
		if(r->redp.time==0){
			r = r->next;
			continue;
		}
		if(time(NULL)-r->redp.time>SPIN){
			printf("检测到了\n"); 
			u = U->next;
			while(u){
				if(strcmp(u->user.name,r->redp.host.name)==0){
					u->user.sum+=r->redp.sum;
					if(r->redp.sum!=0){	
						strcpy(msg,"#");
						strcat(msg," 红包退回 ");
						sprintf(sum,"%.2f",r->redp.sum);
						strcat(msg,sum);
						strcat(msg,"元"); 
						strcpy(u->user.hisMsg[u->user.len++],msg); 
					}
					
					r->redp.time = 0;
					r->redp.sum = 0;
					r->redp.num = 0;					
					break;
				}
				u = u->next;
			}
		}
		r = r->next;
	}
	writeFile('R');
	return;
} 				  
//--------------------------------功能函数---------------------------------- 
//初始化链表函数
void InitLink()
{
	//为头节点开辟空间
	U = (LinklistU)malloc(sizeof(LnodeU));
	R = (LinklistR)malloc(sizeof(LnodeR));
	//让每个链表的尾部指向NULL 
	U->next = NULL;
	R->next = NULL; 
}

//写入本地文件
void writeFile(char type)
{
	FILE *fp;
	LinklistU u = U->next;
	LinklistR r = R->next;
	if(type=='U'){
		//保存名媛数据 
		fp = fopen("./data/user.dat","w+b");
		while(u){
			fwrite(&u->user,sizeof(struct User),1,fp);
			u = u->next;
		}
	}else if(type=='R'){
		//保存红包数据 
		fp = fopen("./data/redp.dat","w+b");
		while(r){
			fwrite(&r->redp,sizeof(struct Redp),1,fp);
			r = r->next;
		}
	}else{
		printf("无法识别要保存的数据类型\n");
		fclose(fp);
		return;
	}
	fclose(fp);
	return;
}

void addNode(char type,struct User user,struct Redp redp)
{
	LinklistU u=U,p;
	LinklistR r=R,q;
	//根据传入类型选择操作
	if(type == 'U'){
		//添加用户节点 
		while(u->next) 
			u = u->next;
		p = (LinklistU)malloc(sizeof(LnodeU));
		p->user = user;
		u->next = p;
		p->next = NULL;
	}else if(type == 'R'){
		//添加红包节点 
		while(r->next)
			r = r->next;
		q = (LinklistR)malloc(sizeof(LnodeR));
		q->redp = redp;
		r->next = q;
		q->next = NULL;
	}else{
		//类型有误 
		printf("Error:Type\n");
	}

	return;
}

//读取本地文件 
void readFile()
{
	FILE *fp;
	struct User user;
	struct Redp redp;
	fp = fopen("./data/user.dat","r+b");
	if(fp!=NULL){
		while(fread(&user,sizeof(struct User),1,fp)==1){
			addNode('U',user,redp);
		}
		fclose(fp);
	}
	
	fp = fopen("./data/redp.dat","r+b");
	if(fp!=NULL){
		while(fread(&redp,sizeof(struct Redp),1,fp)==1){
			addNode('R',user,redp);
		}
		fclose(fp);
	}
	
	return;
	
}

void listUser()
{
	int *temp;
	LinklistU u = U->next;
	printf("____________________________________________________________________________\n");
	printf("|名媛\t密码\t手机号\t\t余额\tsockfd\t pid\t在线\\离线   好友数 |\n");
	while(u){
		printf("|%s\t%s\t%s\t%.2f\t  %d\t%s\t",u->user.name,u->user.password,u->user.telenumber,u->user.sum,u->user.sockfd,u->user.pid);
		if(u->user.state)
			printf("\033[32m 在线\033[0m");
		else
			printf("\033[31m 离线\033[0m");
		printf("\t      %d\t   |\n",u->user.num);
		//temp = (int *)malloc(sizeof(int));
		//sem_getvalue(&u->user.sem[1],temp);
		//printf("|信号量：%d 选项：%s\t\t\t\t\t\t\t   |\n",*temp,u->user.msg[1]);
		u = u->next;
	}
	printf("----------------------------------------------------------------------------\n");
	return;
}

//闺蜜列表->客户端 
void listFriends(int sockfd)
{
	int i=0,count;
	char num[32];
	LinklistU u = U->next;
	while(u){
		if(u->user.sockfd==sockfd){
			sprintf(num,"%d",u->user.num);
			if(send(sockfd,num,32,0)<0)
				perror("send_num");
			count = u->user.num;
			while(count--){
				if(send(sockfd,u->user.friends[i++].fname,32,0)<0)
					perror("send_num"); 
			}
			return;
		}
		u = u->next; 
	}
	return;
}

//添加好友 
void reactMakeFriends(int sockfd)
{
	int i;
	char buf[32],name[32],fname[32];
	LinklistU u = U->next,p = U->next;
	while(u){
		if(u->user.sockfd==sockfd){
			sem_wait(&u->user.sem[1]);
			strcpy(fname,u->user.msg[1]);
			//判断是否已经是好友 
			for(i=0;i<u->user.num;i++){
				if(strcmp(u->user.friends[i].fname,fname)==0){
					if(send(sockfd,"had",32,0)<0)
						perror("send");
					return; 
				}
			}
			//判断是否存在 
			if(findName(fname)==0){
				if(send(sockfd,"unexist",32,0)<0)
					perror("send");
				return;
			}
			
			if(strcmp(fname,u->user.name)==0){
				if(send(sockfd,"same",32,0)<0)
					perror("send");
				return;
			}
			
			//符合情况，提示继续 
			if(send(sockfd,"ok",32,0)<0)
				perror("send");
			
			//获取 好友申请	
			sem_wait(&u->user.sem[2]);
			strcpy(buf,u->user.msg[2]); 
			
			if(strcmp(buf,"<'xyq'>|<'xyq'>")==0)
				return;
				
			while(p){
				if(strcmp(p->user.name,fname)==0){
					strcpy(p->user.addName[p->user.addNum],u->user.name);
					strcpy(p->user.addMsg[p->user.addNum],buf);
					p->user.addNum += 1;
					writeFile('U');
					break;
				}
				p = p->next;
			}
			return;
		}
		u = u->next; 
	}
	return;
}

//处理添加 
void reactAdd(int sockfd)
{
	int i,j,fd;
	char buf[32],temp[32],name[32],msg[32];
	LinklistU u = U->next,p=u;
	while(u){
		if(u->user.sockfd==sockfd){
			for(i=0;i<u->user.addNum;i++){
				if(send(sockfd,u->user.addName[i],32,0)<0)
					perror("send_name");
				if(send(sockfd,u->user.addMsg[i],32,0)<0)
					perror("send_msg");
			}
			sem_wait(&u->user.sem[0]);
			strcpy(buf,u->user.msg[0]);
			
			j = atoi(buf);
			strcpy(name,u->user.addName[j-1]);
			for(i=0;i<u->user.num;i++){
				if(strcmp(u->user.friends[i].fname,name)==0){
					rmArr(u->user.addName,j-1,u->user.addNum);
					rmArr(u->user.addMsg,j-1,u->user.addNum);
					u->user.addNum-=1;
					if(send(sockfd,"had",32,0)<0)
						perror("send");
					return;
				}
			}
			
			for(i=0;i<u->user.addNum;i++){
				sprintf(temp,"%d",i+1);
				if(strcmp(buf,temp)==0){
					if(findName(u->user.addName[i])==0){
						if(send(sockfd,"unexist",32,0)<0)
							perror("send");
						rmArr(u->user.addName,i,u->user.addNum);
						rmArr(u->user.addMsg,i,u->user.addNum);
						u->user.addNum-=1;
						return;
					}
					//如果对方用户存在，两边的好友链表相互加入
					while(p){
						//对方用户 
						if(strcmp(p->user.name,u->user.addName[i])==0){
							p->user.friends[p->user.num].state=0;
							strcpy(p->user.friends[p->user.num++].fname,u->user.name);
							strcpy(msg,"# ");
							strcat(msg,u->user.name);
							strcat(msg," 已通过好友验证");
							strcpy(p->user.hisMsg[p->user.len++],msg); 
							break;
						}
						p = p->next;
					}
					u->user.friends[u->user.num].state=0;
					strcpy(u->user.friends[u->user.num++].fname,u->user.addName[i]);
					rmArr(u->user.addName,i,u->user.addNum);
					rmArr(u->user.addMsg,i,u->user.addNum);
					u->user.addNum-=1;					
					writeFile('U');
					if(send(sockfd,"ok",32,0)<0)
						perror("send"); 
					return;
				}
			} 
			if(i==u->user.addNum){
				if(send(sockfd,"error",32,0)<0)
					perror("send");
				return;
			}
		}
		u = u->next;
	} 
	return;
}

//充值 
void reactPush(int sockfd)
{
	float sum;
	char buf[32];
	LinklistU u = U->next;
	while(u){
		if(u->user.sockfd==sockfd){
			sem_wait(&u->user.sem[3]);
			strcpy(buf,u->user.msg[3]);
			if(strcmp(buf,"error")==0)
				return;
				
			sum = atof(buf);
			
			u->user.sum+=sum;
			writeFile('U');
			if(send(sockfd,"ok",32,0)<0)
				perror("send");
			return;
		}
		u = u->next; 
	}
	if(send(sockfd,"error",32,0)<0)
		perror("send");
	return;
}

//用户转账
void reactTran(int sockfd)
{
	float num;
	int i,len;
	char buf[32],temp[32],name[32],msg[32];
	LinklistU u = U->next,p=U->next;
	listFriends(sockfd);
	while(u){
		if(u->user.sockfd==sockfd){
			sem_wait(&u->user.sem[1]);
			strcpy(name,u->user.msg[1]);
			for(i=0;i<u->user.num;i++){
				if(strcmp(u->user.friends[i].fname,name)==0){
					sprintf(buf,"%.2f",u->user.sum);
					if(send(sockfd,buf,32,0)<0)
						perror("send");
					
					sem_wait(&u->user.sem[3]);
					strcpy(temp,u->user.msg[3]);
					
					if(strcmp(temp,"error")==0)
						return;
					num = atof(temp);
					
					
					/*
					while(p){
						//对方用户 
						if(strcmp(p->user.name,u->user.addName[i])==0){
							strcpy(p->user.friends[p->user.num++].fname,u->user.name);
							strcpy(msg,"#");
							strcat(msg,u->user.name);
							strcat(msg," 已通过好友验证");
							strcpy(p->user.hisMsg[p->user.len++],msg); 
							break;
						}
						p = p->next;
					}
					*/
					
					while(p){
						if(strcmp(p->user.name,name)==0){
							p->user.sum+=num;
							strcpy(msg,"#");
							strcat(msg,u->user.name);
							strcat(msg," 向你转账 ￥");
							//printf("temp = %s\n",temp);
							strcat(msg,temp); 
							
							//printf("msg = %s\n",msg);
							strcpy(p->user.hisMsg[p->user.len++],msg);
							
							//printf("hisMsg = %s\n",p->user.hisMsg[p->user.len-1]);
							u->user.sum-=num;
							writeFile('U');
							if(send(sockfd,"ok",32,0)<0)
								perror("send");
							return;
						}
						p = p->next;
					}
					
					if(send(sockfd,"error",32,0)<0)
						perror("send");
					return;
				}
			}
			if(i==u->user.num){
				if(send(sockfd,"error",32,0)<0)
					perror("send");
				return;
			}
			return;
		}
		u = u->next;
	}
	return;
}

//未读消息 
void listHistory(int sockfd)
{
	int i,len=0,num=0,j,k,arr[1024]={0};
	char buf[32],name[1024][32],addMsg[1024][128],temp[32]="none";
	LinklistU u = U->next;
	Inspection();
	//printf("开始了\n");
	while(u){
		if(u->user.sockfd==sockfd){
			//区分消息类型  
			for(i=0;i<u->user.len;i++){
				//printf("msg:%s\n",u->user.hisMsg[i]);
				//将#类型的消息添加到addMsg中 
				if(u->user.hisMsg[i][0]=='#')
					strcpy(addMsg[num++],u->user.hisMsg[i]);
				//
				if(u->user.hisMsg[i][0]=='$'){
					j=0;
					memset(temp,0,strlen(temp));
					while(1){
						if(u->user.hisMsg[i][j+1]==':')
							break;
						temp[j] = u->user.hisMsg[i][j+1];
						j++;
					}
					//printf("%d temp=%s\n",i,temp);
					for(j=0;j<len;j++){
						if(strcmp(name[j],temp)!=0)
							continue;
						arr[j]+=1;
						break;
					}
					if(j==len){
						strcpy(name[len++],temp);
						arr[len-1]+=1;
					}
				}
			}
			//printf("num=%d len=%d\n",num,len);
			sprintf(buf,"%d",num);
			if(send(sockfd,buf,32,0)<0)
				perror("send");
			sprintf(buf,"%d",len);
			if(send(sockfd,buf,32,0)<0)
				perror("send");
			
			for(i=0;i<num;i++)
				if(send(sockfd,addMsg[i],128,0)<0)
					perror("send");
			
			for(i=0;i<len;i++){
				sprintf(buf,"%d",arr[i]);
				strcat(name[i]," ");
				strcat(name[i],buf);
				//printf("name[i] = %s\n",name[i]);
				if(send(sockfd,name[i],32,0)<0)
					perror("send");
			}
			
			j=0;
			for(i=0;i<u->user.len;i++)
				if(u->user.hisMsg[i][0]!='#')
					strcpy(u->user.hisMsg[j++],u->user.hisMsg[i]);
			
			u->user.len = j;
			/*
			//清空验证类的消息
			for(i=0;i<u->user.len;i++){
				if(u->user.hisMsg[i][0]=='#'){
					for(j=i;j<len-1;j++){
						strcpy(u->user.hisMsg[j],u->user.hisMsg[j+1]);
					}
					u->user.len--;
				}
			}
			*/
			writeFile('U');
			return;
		}
		u = u->next;
	}
	return;
}

//罗列红包
void listPack(int sockfd)
{
	int i,len=0;
	char buf[32];
	LinklistR r = R->next;
	
	Inspection(); 
	
	while(r){
		len++;
		r = r->next;
	}
	sprintf(buf,"%d",len);
	if(send(sockfd,buf,32,0)<0)
		perror("send_len");
	if(len==0){
		sleep(2);
		return;
	}
	r = R->next;
	//printf("name\ttime\tsum\n");
	while(r){
		//if(r->redp.time==0)
			//i = 0;
		//else
		//	i = time(NULL)-r->redp.time;
		//printf("%s\t%ds\t%.2f\n",r->redp.host.name,i,r->redp.sum);
		//发送时间文本
		if(send(sockfd,r->redp.timeMsg,32,0)<0)
			perror("send_timeMsg");
		//发送等级
		i = r->redp.host.time;
		sprintf(buf,"%d",i);
		if(send(sockfd,buf,32,0)<0)
			perror("send_time");
		//发送名字
		if(send(sockfd,r->redp.host.name,32,0)<0)
			perror("send_name");
		//发送祝语
		if(send(sockfd,r->redp.bless,32,0)<0)
			perror("send_bless"); 
		
		r=r->next;
	}
	return;
}

void priChat(int sockfd)
{
	int i,j,k,l;
	int len=0;//消息序列长度 
	int index;//好友序列 
	int flag; //聊天状态位 
	int state;//在线状态 
	int fsock;//好友sockfd 
	char buf[32],name[32]="none",fname[32],showMsg[32][128],msg[128],sendMsg[128];
	LinklistU u = U->next,q = U->next; 
	while(u){
		if(u->user.sockfd==sockfd){	
			//Step 1:输出好友列表
			listFriends(sockfd);
			//Step 2:获取聊天对象
			sem_wait(&u->user.sem[1]);
			strcpy(fname,u->user.msg[1]);
			
			for(index=0;index<u->user.num;index++){
				if(strcmp(fname,u->user.friends[index].fname)==0){
					u->user.friends[index].state = 1; 
					
//---------------------//这里会被更新！！！！！！！ 
					//获取好友sockfd
					fsock = findSockfd(fname); 
					
					//提示成功进入聊天 
					if(send(sockfd,"ok",32,0)<0)
						perror("send_ok");
				//	printf("fname = %s\n",fname);	
					l = 0;
					for(i=0;i<u->user.len;i++){
						//Step 5:检测hisMsg[32] 
						if(u->user.hisMsg[i][0]=='$'){
							j=1;
							k=0;
							memset(name,0,strlen(name));
							while(u->user.hisMsg[i][j]!=':')
								name[k++]=u->user.hisMsg[i][j++];
							//printf("name = %s\n",name);
							if(strcmp(name,fname)==0){
								strcpy(showMsg[len++],u->user.hisMsg[i]);
								//消息不参与更新 
								
								//但是需要被储存到历史记录
								saveMsg(u->user.name,fname,u->user.hisMsg[i]);
								continue;
							} 
						}
						strcmp(u->user.hisMsg[l++],u->user.hisMsg[i]);
					}
					u->user.len = l;
					writeFile('U');
					//printf("i = %d\tlen = %d\n",i,len);
					//Step 6:查询历史消息 
					//i,j,k,name结束 
					if(len==0)
						for(;len<6&&len<u->user.friends[index].len;len++)
							strcpy(showMsg[len],u->user.friends[index].chatMsg[u->user.friends[index].len+len-6]);
					
					//Step 7:首轮发送 
					//state
					state=findState(fname);
					sprintf(buf,"%d",state);
					if(send(sockfd,buf,32,0)<0)
						perror("send_state");
					//flag
					fsock = findSockfd(fname);
					sprintf(buf,"%d",fsock);
					if(send(sockfd,buf,32,0)<0)
						perror("send_fsock");
					//len
					sprintf(buf,"%d",len);
					if(send(sockfd,buf,32,0)<0)
						perror("send_len");
					
					if(len!=0)
						for(i=0;i<len;i++)
							if(send(sockfd,showMsg[i],128,0)<0)
								perror("send_showMsg");	
					//Step 8:开始聊天 
					while(1){
						sem_wait(&u->user.sem[2]);
						strcpy(msg,u->user.msg[2]);
						if(strcmp(msg,"exit")==0){
							//printf("退出聊天框！\n");
							u->user.friends[index].state=0;
							return;
						}
						//封装信息
						strcpy(sendMsg,"$");
						strcat(sendMsg,u->user.name);
						strcat(sendMsg,":");
						strcat(sendMsg,msg);
						
						//检测聊天状态位 
						flag = findChatState(fname,u->user.name);
						//printf("\t封装后:%s,聊天状态:%d\n",sendMsg,flag);
						if(flag == 1){
							//关闭接收线程 
							//发送
							fsock = findSockfd(fname); 
							if(send(fsock,sendMsg,128,0)<0)
								perror("send_msg");
							//储存到历史消息
							saveMsg(fname,u->user.name,sendMsg);
							saveMsg(u->user.name,fname,sendMsg);
							writeFile('U');
						}else{
							fsock = 0; 
							saveMsg(u->user.name,fname,sendMsg);
							saveHis(fname,sendMsg);
							writeFile('U');
						}
					} 
					return;
				}
			}
			//Step 4:无好友->退出  none 
			if(index==u->user.num){
				if(send(sockfd,"none",32,0)<0)
					perror("recv");
				return;
			}
			return;
		}
		u = u->next;
	}
	return;
}

//群聊
void groChat(int sockfd)
{
	char buf[256],msg[256];
	LinklistU u = U->next,p = U->next;
	while(u){
		if(u->user.sockfd == sockfd){
			u->user.gState = 1;
			
			while(1){
				//告知系统参与群聊, 
				sem_wait(&u->user.sem[2]);
				strcpy(buf,u->user.msg[2]);
				
				//printf("buf=%s\n",buf);
				if(strcmp(buf,"exit")==0){
					u->user.gState = 0;
					return;
				}
				strcpy(msg,"$:");
				strcat(msg,u->user.name);
				strcat(msg,":");
				strcat(msg,buf);
				p = U->next;
				while(p){
					//printf("name:%s gState=%d\n",p->user.name,p->user.gState);
					if(p->user.gState==1&&p->user.state==1){
						if(send(p->user.sockfd,msg,256,0)<0)
							perror("send_groMsg");
						//printf("->%s\t",p->user.name);
					}
					p = p->next;
				}
			}
			
			return;
		}
		u = u->next;
	}
	return;
}
void reactSend(int sockfd)
{
	int i;
	struct Redp rp;
	char buf[32];
	LinklistU u = U->next;
	struct User user;
	while(u){
		if(u->user.sockfd==sockfd){
			//Step1:获取金额 
			sem_wait(&u->user.sem[3]);
			strcpy(buf,u->user.msg[3]);
			rp.sum = atof(buf);
			if(u->user.sum<=rp.sum){
				if(send(sockfd,"error",32,0)<0)
					perror("send_error");
				sleep(1);
				return;
			}
			if(send(sockfd,"ok",32,0)<0)
				perror("send_error");
			//Step2:获取个数 
			sem_wait(&u->user.sem[4]);
			strcpy(buf,u->user.msg[4]);
			rp.num = atoi(buf);
			//Step3:获取祝语 
			sem_wait(&u->user.sem[2]);
			strcpy(rp.bless,u->user.msg[2]);
			
			//Step4:设置宿主 
			//更新等级
			if(u->user.start>0){
				u->user.time = time(NULL)-u->user.start+u->user.time;
				u->user.start = time(NULL);
			} 
			rp.host = u->user;
			
			//Step5:清空列表 
			rp.len=0;
			for(i=0;i<1024;i++)
				memset(rp.user[i],0,sizeof(rp.user[i]));
				
			//Step6:保存时间文本并写入时间 
			time(&rp.time);
			strcpy(rp.timeMsg,asctime(gmtime(&rp.time)));
			
			//Step7:扣除金额
			u->user.sum -= rp.sum;
			
			//Step8:添加节点 
			addNode('R',user,rp);
			
			//Step9:保存至本地 
			writeFile('U');
			writeFile('R');
			if(send(sockfd,"ok",32,0)<0)
				perror("send_ok"); 
			return;
		}
		u = u->next;
	}
	return;
}

int reactGrab(int sockfd)
{
	int i,index,len=0,temp,rm,f;
	char buf[32],msg[128],sum[32];
	float out;
	LinklistR r = R->next;
	LinklistU u = U->next,q = U->next;
	while(r){
		r = r->next;
		len++;
	}
	sprintf(buf,"%d",len);
	if(send(sockfd,buf,32,0)<0)
		perror("send_len");
		
	if(len==0)
		return;
		
	while(u){
		if(u->user.sockfd==sockfd){
			sem_wait(&u->user.sem[4]);
			strcpy(buf,u->user.msg[4]);
			index = atoi(buf);
			
			r = R->next;
			for(i=0;i<index;i++)
				r = r->next;
			
			//判断红包是否已被抢完 
			if(r->redp.num==0){
				if(send(sockfd,"end",32,0)<0)
					perror("send_end");
				return;
			}
			
			//Step1:判断失效
			if(r->redp.time==0){
				if(send(sockfd,"over",32,0)<0)
					perror("send_over");
				return;
			}
//测试点180s 
			//Step2:检测红包效期
			if(time(NULL)-r->redp.time>SPIN){
				q = U->next;
				while(q){
					if(strcmp(q->user.name,r->redp.host.name)==0){
						q->user.sum+=r->redp.sum;
						//# 红包退回 %.2f元
						strcpy(msg,"#");
						strcat(msg," 红包退回 ");
						sprintf(sum,"%.2f",r->redp.sum);
						strcat(msg,sum);
						strcat(msg,"元"); 
						saveHis(r->redp.host.name,msg);

						r->redp.sum=0;
						r->redp.time=0;
						
						writeFile('U');
						writeFile('R');
						if(send(sockfd,"over",32,0)<0)
							perror("send_over");
						return;
					}
					q = q->next;
				}
			}
			
			//Step3:判断占有
			for(i=0;i<r->redp.len;i++){
				if(strcmp(r->redp.user[i],u->user.name)!=0)
					continue;
				
				if(send(sockfd,"had",32,0)<0)
					perror("send_had");
				return;
			}
			
			//Step4:判断余额 [可以舍去]
			if(r->redp.sum==0||r->redp.num==0){
				if(send(sockfd,"end",32,0)<0)
					perror("send_end");
				return;
			}
			
			memset(buf,0,sizeof(buf));
			//抢红包
			if(r->redp.num==1){
				sprintf(buf,"%.2f",r->redp.sum);
				u->user.sum+=r->redp.sum;
				r->redp.num=0;
				strcpy(r->redp.user[r->redp.len++],u->user.name);
				r->redp.time=0;
			}else{
				
				temp = r->redp.sum*100-r->redp.num;
				srand(time(NULL));
				rm = rand()%temp+1;
				i = rm/100;
				f = rm%100;
				out = i+f*0.01;
				sprintf(buf,"%.2f",out);
				r->redp.sum-=out;
				u->user.sum+=out;
				
				r->redp.num--;
				strcpy(r->redp.user[r->redp.len++],u->user.name);
			}
			
			writeFile('U');
			writeFile('R');
			
			if(send(sockfd,buf,32,0)<0)
				perror("send_out");
			return; 
		}
		u = u->next;
	}
	return;
}

//响应强制下线 
void reactSet(int sockfd)
{
	char name[32];
	LinklistU u = U->next;
	strcpy(name,myRecv(sockfd));
	setSockfd(name,-1);
	setState(name,0);
	return;
}

