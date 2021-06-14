# include <time.h>
# include <errno.h>
# include <fcntl.h>
# include <stdio.h>
# include <stdlib.h>
# include <string.h>
# include <unistd.h>
# include <pthread.h>
# include <sys/shm.h>
# include <sys/ipc.h>
# include <sys/msg.h>
# include <sys/stat.h>
# include <sys/types.h>
# include <sys/socket.h>
# include <netinet/in.h>

# define FSCG "ok" 

//
//--------------------------全局变量--------------------------------------- 
int curSockfd;						//存放当前进程的sock号
struct sockaddr_in serv_addr;		//服务器地址结构体(应该是吧) 
//------------------------------------------------------------------------- 
char ft[][8]={
	"\33[30m","\33[31m","\33[32m","\33[33m","\33[34m","\33[35m",
	"\33[36m","\33[37m","\33[38m","\33[39m"
};
char bg[][8]={
	"\33[40m","\33[41m","\33[42m","\33[43m","\33[44m","\33[45m",
	"\33[46m","\33[47m","\33[48m","\33[49m"
};
//==========================函数声明=======================================
int testLen(char []);			//测量长度 
void myReg(void);				//注册 
void myLog(void);				//登入
void myDes(void);				//注销  
void priChat(void);				//私聊
void groChat(void);				//群聊 
void showMsg(void);				//接收
void showGro(void);				//接收 
void mainMenu(void);			//主菜单
void grabPack(void);			//抢红包 
void userMenu(void);			//用户页面 
void sendPack(void); 			//发送红包 
void userPush(void);			//用户充值 
void userTran(void);			//用户转账
void setState(void);			//设置状态  
void listPack(void);			//罗列红包 
void addFriends(int);			//验证消息
void makeFriends(void);			//添加好友
void listFriends(void);			//输出闺蜜列表
void listHistory(void);			//输出历史消息 
void showPack(int,char[],int,char[],char[]);//红包样式 
//========================================================================= 

int main(int argc,char *argv[])
{
	int *p;
	int sockfd;		//存放socket函数返回值，标识当前进程的sockfd 
	pthread_t id;
	//socket套接字的准备工作 
	serv_addr.sin_port = atoi(argv[2]);
	serv_addr.sin_family = AF_INET;
	serv_addr.sin_addr.s_addr = inet_addr(argv[1]);
	memset(serv_addr.sin_zero,0,8);
    //获取sockfd		判断是否成功		将sockfd赋值给curSockfd 
	sockfd = socket(AF_INET,SOCK_STREAM,0);
	if(sockfd<0)
		perror("sockfd");
	curSockfd = sockfd;
	//开始连接,并判断是否成功 
	if(connect(sockfd,(struct sockaddr *)&serv_addr,sizeof(struct sockaddr))<0){
		perror("connect");
		return 0;
	}
	//创建一个线程用于判断端口号是否有效，若无效就exit(0);
	//pthread_create(&id,NULL,(void *)judgeConn,NULL);
	//若连接成功,则进入函数
	mainMenu();
	return 0;
}

void mainMenu()
{
	char buf[1024];
	
	while(1){
		printf("%s","\033[1H\033[2J"); 
		printf ("----------诚----------------\n");
		printf ("--------------毅------------\n");
		printf ("|\t1 名媛认证\t   |\n");	//注册 
		printf ("|\t2 名媛上线\t   |\n");	//登入 
		printf ("|\t3 重新认证\t   |\n");	//找回密码 
		printf ("|\t4 贬回凡人\t   |\n");	//注销账号 
		printf ("|\t5 名媛小憩\t   |\n");	//关闭窗口 
		printf ("|\t\t\t   |\n");
		printf ("----------名----------------\n");
		printf ("--------------媛------------\n");
		printf ("请选择："); 
		scanf("%s",buf);
		//发送选项 
		if(send(curSockfd,buf,1024,0)<0)
			perror("send");
		if(strcmp(buf,"1")==0){
			myReg();
		}else if(strcmp(buf,"2")==0){
			myLog();
		}else if(strcmp(buf,"3")==0){
			//reSet(); 
		}else if(strcmp(buf,"4")==0){
			myDes();
		}else if(strcmp(buf,"5")==0){
			exit(0);
		}else if(strcmp(buf,"set")==0){
			setState();
		}else{
			printf("\t输入有误,请重新输入\n"); 
			sleep(1);
		}
	}
	return;
}

//用户功能页面 
void userMenu()
{
	int num,len;
	char buf[32],msg[64],sum[32],numbuf[32];
	while(1){
		if(recv(curSockfd,buf,32,0)<0)
			perror("recv_num");
		num = atoi(buf);
		if(recv(curSockfd,buf,32,0)<0)
			perror("recv_num");
		len = atoi(buf);
		printf("%s","\033[1H\033[2J"); 
		printf ("----------\33[34m菜\33[0m-----------------\n");
		printf ("--------------\33[34m单\33[0m-------------\n");
		printf ("|\t#  验证消息%d\t   |\n",num);
		printf ("|\t@  未读消息%d\t   |\n",len);
		printf ("|\t*  刷新消息键\t   |\n");
		printf ("|\t1  罗列闺蜜团\t   |\n");
		printf ("|\t2  闺蜜私房话\t   |\n");
		printf ("|\t3  名媛拼夕夕\t   |\n");
		printf ("|\t4  给闺蜜转账\t   |\n");
		printf ("|\t5  首冲大礼包\t   |\n");
		printf ("|\t6  图大伙一乐\t   |\n");
		printf ("|\t7  拼手气时刻\t   |\n");
		printf ("|\t8  寻找真闺蜜\t   |\n");
		printf ("|\t10  查询余额\t   |\n");
		printf ("|\texit 退出登入\t   |\n");
		printf ("----------\33[34m功\33[0m-----------------\n");
		printf ("--------------\33[34m能\33[0m-------------\n");
		printf ("请选择："); 
		scanf("%s",buf);
		
		strcpy(msg,"c");
		strcat(msg,buf);
		
		if(send(curSockfd,msg,64,0)<0)
			perror("send");
		
		if(strcmp(buf,"1")==0){
			listFriends();
			printf("任意键退出:");
			//突然发现scanf是最简单的阻塞函数
			scanf("%s",buf);
		}else if(strcmp(buf,"2")==0){
			priChat();
		}else if(strcmp(buf,"3")==0){
			groChat();
		}else if(strcmp(buf,"4")==0){
			userTran();
		}else if(strcmp(buf,"5")==0){
			userPush();
		}else if(strcmp(buf,"6")==0){
			sendPack(); 
		}else if(strcmp(buf,"7")==0){
			listPack();
			grabPack();
		}else if(strcmp(buf,"8")==0){
			makeFriends();
			sleep(1);
		}else if(strcmp(buf,"10")==0){
			if(recv(curSockfd,sum,32,0)<0)
				perror("recv_sum");
			printf("\t账户余额:￥%s\n",sum);
			sleep(1);
		}else if(strcmp(buf,"exit")==0){
			return;
		}else if(strcmp(buf,"#")==0){
			if(recv(curSockfd,numbuf,32,0)<0)
				perror("recv_num");
			num = atoi(numbuf);
			if(num==0){
				printf("\t暂时没人要加你\n");
				sleep(1);
				continue;
			}
			//处理消息
			addFriends(num); 
		}else if(strcmp(buf,"@")==0){
			if(recv(curSockfd,numbuf,32,0)<0)
				perror("recv_num");
			len = atoi(numbuf);
			if(len==0){
				printf("\t消息已清空\n");
				sleep(1);
				continue;
			} 
			listHistory();
			printf("任意键退出:");
			//突然发现scanf是最简单的阻塞函数
			scanf("%s",buf);
		}else if(strcmp(buf,"*")==0){
			
		}else if(strcmp(buf,"show")==0){
			printf("\t已列出所有记录\n"); 
			sleep(1);
		}else{
			printf("\t输入有误,请重新输入\n"); 
			sleep(1);
		}
	}
	return;
}

//----------------------------交互类函数-------------------------------- 
void myReg()
{
	int i;
	char buf[32],pwd[32],code[8],name[32],number[32];
	//手机号 
	while(1){
		printf("输入(手机号):");
		scanf("%s",number);

		//判断位数 
		if(strlen(number)==11){ 
			if(send(curSockfd,number,32,0)<0)
				perror("send");
			break;
		}
		printf ("--------------------------------------------------\n");
		printf ("|\t\t#system msg:\033[31m手机号有误\033[0m\t\t |\n");
		printf ("--------------------------------------------------\n");
		printf("请重新");
	} 
	//验证码
	while(1){
		//接收号码
		if(recv(curSockfd,code,8,0)<0)
			perror("recv");
		
		printf("%s","\033[1H\033[2J"); 
		printf ("-------------------------------------------------\n");
		printf("|\t#system msg:短信验证码:\033[33m%s\033[0m\t\t|\n",code);
		printf ("-------------------------------------------------\n");
		printf("请输入(验证码):");
		scanf("%s",buf);
		if(strcmp(buf,code)==0){ 
			if(send(curSockfd,"same",32,0)<0)
				perror("send");
			break;
		}
		
		printf ("--------------------------------------------------\n");
		printf ("|\t\t#system msg:\033[31m验证码有误\033[0m\t\t |\n");
		printf ("|\t#system msg:即将自动获取新验证码!\t |\n");
		printf ("--------------------------------------------------\n");
		if(send(curSockfd,"error",32,0)<0)
			perror("send");
		sleep(2);//休眠两秒
	} 
	printf("%s","\033[1H\033[2J"); 
	printf ("-----------------------------------------\n");
	printf("|\t\t\033[34m名媛认证中心\033[0m\t\t|\n");
	printf ("-----------------------------------------\n");
	//姓名 
	while(1){
		printf("输入(名媛名):");
		scanf("%s",name); 
		if(send(curSockfd,name,32,0)<0)
			perror("send");
		//等待接收
		if(recv(curSockfd,buf,32,0)<0)
			perror("recv");
		//判断存在与否 
		if(strcmp(buf,"exist")!=0)
			break;
		//提示重新 
		printf("%s被占用\n请重新",name);
	}
	//设置密码
	for(i=0;i<3;i++){
		printf("%s","\033[1H\033[2J"); 
		printf ("-------------------------------------------------\n");
		printf ("|\t=￣ω￣=剩余\033[33m%d\033[0m次输入机会=￣ω￣=\t\t|\n",3-i);
		printf ("-------------------------------------------------\n");
		printf ("请输入(新密码):");
		scanf  ("%s",pwd);
		printf ("请确认(新密码):");
		scanf  ("%s",buf); 
		
		if(strcmp(pwd,buf)==0)
			break;
		
		printf("\t\t\t\033[31m!!!ERROR!!!\033[0m\n\n");
		sleep(1); 
	}
	
	if(i==3){
		if(send(curSockfd,"system[]exit",32,0)<0)
			perror("send");
		printf("%s","\033[1H\033[2J"); 
		printf ("--------------------------------------------------------\n");
		printf("|\t#system msg:\033[35m还想冒充名媛？老早觉得你有问题 拜！\033[0m|\n");
		printf ("--------------------------------------------------------\n");
		sleep(1);
		return; 
	}
	
	if(send(curSockfd,pwd,32,0)<0)
			perror("send");
	
	//wait oprate
	if(recv(curSockfd,buf,32,0)<0)
		perror("recv");
	
	if(strcmp(buf,"exist")==0){
		printf("( ′?ω?)?(._.`) : \t终究是晚了一步,%s被占用了\n",name);
		sleep(3);
	}else if(strcmp(buf,"success")==0){
		printf("%s","\033[1H\033[2J"); 
		printf("\n\n");
		printf("\t\33[32mヾ(≧O≦)〃\33[0m嗷~ : 成功打入名媛内部了欸！\n");
		printf("\33[32m( ′?ω?)?(._.`)\33[0m : 以后也要加油做一个名媛哦！\n");
		sleep(2);
	}
	return;
}

void myLog()
{
	int i;
	char buf[32],pwd[32],name[32];
	//获取名字
	while(1){
		printf("输入(名媛名):");
		scanf("%s",name);
		//send name
		if(send(curSockfd,name,32,0)<0)
			perror("send");
		
		//recv match
		if(recv(curSockfd,buf,32,0)<0)
			perror("recv");
			
		if(strcmp(buf,"exist")==0)
			break;
		printf("%s","\033[1H\033[2J");
		printf ("---------------------------------------------------------\n");
		printf("|\t\33[32mo(一︿一+)o想用假名号混入群聊？o(一︿一+)o\33[0m\t|\n");
		printf ("---------------------------------------------------------\n");
		printf("请重新");
	}
	//比对密码
	for(i=0;i<3;i++){
		printf("%s","\033[1H\033[2J"); 
		printf ("-----------------------------------------\n");
		printf ("|\t=￣ω￣=剩余\033[33m%d\033[0m次输入机会=￣ω￣=\t|\n",3-i);
		printf ("-----------------------------------------\n");
		printf ("请输入(密码):");
		scanf  ("%s",pwd);
		
		if(send(curSockfd,pwd,32,0)<0)
			perror("send");
		
		if(recv(curSockfd,buf,32,0)<0)
			perror("recv");
			
		if(strcmp(buf,"consistent")==0)
			break;
		
		printf("\t\t\t\033[31m!!!ERROR!!!\033[0m\n\n");
		sleep(1); 
	}
	if(i==3){
		printf("%s","\033[1H\033[2J"); 
		printf ("--------------------------------------------------------\n");
		printf("|\t#system msg:\033[35m还想冒充名媛？老早觉得你有问题 拜！\033[0m|\n");
		printf ("--------------------------------------------------------\n");
		sleep(1);
		return; 
	}
	
	//判断在线与否 
	if(recv(curSockfd,buf,32,0)<0)
		perror("recv buf");
		
	if(strcmp(buf,"on-line")==0){
		printf("\n\t已在其他设备登录!\n");
		sleep(2);
		return;
	}
	strcpy(buf,"");
	sprintf(buf,"%d",getpid());
	if(send(curSockfd,buf,32,0)<0)
		perror("send_pid");
	printf("%s","\033[1H\033[2J"); 
	printf (" --------------------------------------------\n");
	printf("|\33[33m ( *￣▽￣)((≧︶≦*)登入成功,即将跳转用户页面\33[0m|\n");
	printf (" --------------------------------------------\n");
	sleep(1);
	//Step4:跳转功能页面 
	userMenu();
	
	return;
}

//注销
void myDes(void)
{
	char buf[32],name[32];
	printf("请输入(用户名):");
	scanf("%s",name);
	if(send(curSockfd,name,32,0)<0)
		perror("send_name");
		
	if(recv(curSockfd,buf,32,0)<0)
		perror("recv_judge");
	
	if(strcmp(buf,"unexist")==0){
		printf("\t该用户不存在\n");
		sleep(1); 
		return;
	}else if(strcmp(buf,"on_line")==0){
		printf("\t该用户已在其他设备登录\n");
		sleep(1);
		return;
	}
	
	printf("请输入(管理员密码):");
	scanf("%s",buf);
	
	if(send(curSockfd,buf,32,0)<=0)
		perror("send_root");
		
	if(strcmp(buf,"root")!=0){
		printf("\t密码有误\n");
		sleep(1);
		return;
	} 
	
	if(recv(curSockfd,buf,32,0)<0)
		perror("recv_outcome");
	
	if(strcmp(buf,"ok")==0){
		printf("\t%s用户已被成功销毁\n",name);
		sleep(1);
	}else{
		printf("\t未知错误\n");
		sleep(1);
	}
	
	return;
}
 
//添加好友 
void makeFriends()
{
	char fname[32]="",buf[32],msg[32];
	
	printf("请输入(闺蜜名):");
	scanf("%s",buf); 
	
	strcpy(fname,"n");
	strcat(fname,buf);
	
	if(send(curSockfd,fname,32,0)<0)
		perror("send_fidname");
	
	if(recv(curSockfd,buf,32,0)<0)
		perror("recv");
		
	if(strcmp(buf,"had")==0){
		printf("你们已经是好友啦!\n");
		return;
	}else if(strcmp(buf,"unexist")==0){
		printf("她还没出生呢!\n");
		return;	
	}else if(strcmp(buf,"same")==0){
		printf("自己加自己，人格分裂？\n");
		return;	
	}
	
	printf("请输入(添加好友申请):");
	scanf("%s",buf);
	
	if(strlen(buf)>32){
		printf("\n\t\t字数超出限制!\n");
		strcpy(msg,"m");
		strcat(msg,"<'xyq'>|<'xyq'>");
		if(strlen(buf))
		if(send(curSockfd,msg,64,0)<0)
			perror("send");
		sleep(1);
		return;
	}
	
	strcpy(msg,"m");
	strcat(msg,buf);
	if(strlen(buf))
	if(send(curSockfd,msg,64,0)<0)
		perror("send");
		
	printf("\n\t\t发送成功!\n");
	sleep(1);
	return;
}

//验证 
void addFriends(int num)
{
	int i;
	char buf[128],msg[32],name[32];
	for(i=0;i<num;i++){
		if(recv(curSockfd,name,32,0)<0)
			perror("recv_name");
		if(recv(curSockfd,msg,32,0)<0)
			perror("recv_msg");
		
		printf("%d %s:%s\n",i+1,name,msg);
	}
	printf("请选择(消息序号):");
	scanf("%s",buf);
	strcpy(msg,"c");
	strcat(msg,buf);
	if(send(curSockfd,msg,32,0)<0)
		perror("send_msg");
	
	if(recv(curSockfd,buf,32,0)<0)
		perror("recv_outcome");
	//printf("buf=%s\n",buf);
	if(strcmp(buf,"ok")==0){
		printf("\t添加成功\n");
	}else if(strcmp(buf,"error")==0){
		printf("\t输入错误\n");
	}else if(strcmp(buf,"unexist")==0){
		printf("\t该用户已注销\n");
	}else if(strcmp(buf,"had")==0){
		printf("\t你们已经是好友啦\n");
	}else{
		printf("\t未知错误\n");
	}
	sleep(1);
	return;
}

void listHistory()
{
	int i,num,len;
	char buf[128];
	if(recv(curSockfd,buf,32,0)<0)
		perror("recv");
	num = atoi(buf);
	if(recv(curSockfd,buf,32,0)<0)
		perror("recv");
	len = atoi(buf);
	//printf("num=%d len=%d\n",num,len);
	for(i=0;i<num;i++){
		memset(buf,0,sizeof(buf));
		if(recv(curSockfd,buf,128,0)<0)
			perror("recv");
		printf("%s\n",buf);
		//sleep(1);
	}
	//
	for(i=0;i<len;i++){
		if(recv(curSockfd,buf,32,0)<0)
			perror("recv");
		printf("@ %s\n",buf);
	}
	//printf("here?\n");
	return;
}

//罗列红包
void listPack()
{
	int i,len;
	int level;
	char buf[32];
	char name[32],bless[32],timeMsg[32];	
	printf("%s","\033[1H\033[2J"); 
	printf("\t\33[34m----------\33[0m\33[41m红包群\33[0m\33[34m----------\33[0m\n");
	if(recv(curSockfd,buf,32,0)<0)
		perror("recv_len");
	len = atoi(buf);
	if(len==0){
		printf("\33[0m\n\t\33[44m                          \33[0m\n");
		printf("\33[0m\t\33[44m                          \33[0m\n");
		printf("\33[0m\t\33[44m        \33[48m 空空如也 \33[44m        \33[0m\n");
		printf("\33[0m\t\33[44m                          \33[0m\n");
		printf("\33[0m\t\33[44m                          \33[0m\n\n");
		sleep(2);
		return;
	}
	for(i=0;i<len;i++){
		//时间文本 
		if(recv(curSockfd,timeMsg,32,0)<0)
			perror("recv_time");
		//等级
		if(recv(curSockfd,buf,32,0)<0)
			perror("recv_level");
//测试点(60s一级)
		level = atoi(buf)/180; 
		//名字
		if(recv(curSockfd,name,32,0)<0)
			perror("recv_name");
		//祝语 
		if(recv(curSockfd,bless,32,0)<0)
			perror("recv_bless");
		
		showPack(i,timeMsg,level,name,bless);
	}
	return;
}

//充值 
void userPush()
{
	float sum;
	char buf[32],msg[32];
	printf("请输入(金额):");
	scanf("%f",&sum);
	strcpy(msg,"f"); 
	if(sum<=0||sum>10000){
		printf("\t金额超出限制\n");
		strcat(msg,"error");
		if(send(curSockfd,msg,32,0)<0)
			perror("send_sum");
		sleep(1);
		return;
	}
	sprintf(buf,"%.2f",sum);
	strcat(msg,buf); 
	if(send(curSockfd,msg,32,0)<0)	
		perror("send");
		
	if(recv(curSockfd,buf,32,0)<0)
		perror("send");
		
	if(strcmp(buf,"ok")==0)
		printf("\t充值成功\n");
	else
		printf("\t充值失败\n");
	sleep(1);
	return;
	
}

//用户转账
void userTran()
{
	int i,num;
	float sum,temp;
	char buf[32],msg[32],name[32];
	listFriends();
	printf("请输入(转账对象):");
	scanf("%s",name);
	
	strcpy(msg,"n");
	strcat(msg,name);
	if(send(curSockfd,msg,32,0)<0)
		perror("send");
	//
	if(recv(curSockfd,buf,32,0)<0)
		perror("recv");
		
	if(strcmp(buf,"error")==0){
		printf("\t没有该好友\n");
		sleep(1);
		return;
	}
	
	sum = atof(buf);
	
	printf("请输入(金额):￥");
	scanf("%s",buf);
	temp = atof(buf);
	sprintf(buf,"%.2f",temp);
	strcpy(msg,"f");
	if(temp>sum){
		printf("\t余额不足\n");
		strcat(msg,"error");
		if(send(curSockfd,msg,32,0)<0)
			perror("send");
		sleep(1);
		return;
	}else if(temp<0){
		printf("\t金额有误\n");
		strcat(msg,"error");
		if(send(curSockfd,msg,32,0)<0)
			perror("send");
		sleep(1);
		return;
	}
	strcat(msg,buf);
	if(send(curSockfd,msg,32,0)<0)
		perror("send");
	
	if(recv(curSockfd,buf,32,0)<0)
		perror("recv");
	
	if(strcmp(buf,"ok")==0)
		printf("\t转账成功\n");
	else
		printf("\t好友已注销\n");
		
	sleep(1);
	
	return;
}

void priChat()
{
	int i,j,k;
	int len,realLen,msgLen;//消息长度
	int count; 
	int flag;
	int fsock;
	int state;
	char buf[32],name[32],msg[128],sendMsg[128];
	
	pthread_t id;
	
	//Step 1:接收好友 
	listFriends();
	printf("请输入(聊天对象):");
	scanf("%s",buf);
	
	//Step 2:封装发送 
	strcpy(name,"n");
	strcat(name,buf);
	if(send(curSockfd,name,32,0)<0)
		perror("send");
	strcpy(name,buf);//名字存入name中 
	
	//Step 3:接收判断
	if(recv(curSockfd,buf,32,0)<0)
		perror("recv_judge");
	
	if(strcmp(buf,"none")==0){
		printf("\t闺蜜团中查无此人\n");
		sleep(1);
		return;
	}
	
	//Step 4:首轮显示
	if(recv(curSockfd,buf,32,0)<0)
		perror("recv_state");
	state = atoi(buf);
	if(recv(curSockfd,buf,32,0)<0)
		perror("recv_fsock");
	fsock = atoi(buf);

	
	printf("%s","\033[1H\033[2J"); 
	printf(" __________________________________________\n");
	if(state == 0)
		printf("|\t\t%s \33[31m离线\33[0m...\t\t   |\n",name);
	else
		printf("|\t\t%s \33[32m在线\33[0m...\t\t   |\n",name);
	printf("|^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^|\n");
	
	if(recv(curSockfd,buf,32,0)<0)
		perror("recv_len");
	len = atoi(buf);
	
	if(!len)
		printf("\t消息记录为空\n");
	else{
		for(i=0;i<len;i++){
			//获取消息 
			if(recv(curSockfd,msg,128,0)<0)
				perror("recv_msg");
			
			count = 0;
			msgLen = testLen(msg);
			realLen = strlen(msg);
			
			if(msgLen<43){
				printf("|");
				for(j=1;j<=realLen;j++)
					printf("%c",msg[j]);
				while(42-(msgLen++))
					printf(" ");
			}else{
				j=1;//正常的字数输出
				k=1;
				count=0;
				printf("|");
				
				while(k<realLen){
					if(msg[k] & 0x80)
						count++;
					else
						j++;
					printf("%c",msg[k++]);
					if(count==3){
						count=0;
						j+=2;
					}
					if(count==0&&(j>40&&j<43)){
						j = 43-j;
						while(j--)
							printf(" ");
						printf("|\n|");
					}
				}
				j = 41-j;
				while(j--)
					printf(" ");
			}
			if(i==len-1)
				break;
			printf("|\n|\t\t\t\t\t   |\n");
		}
		printf("|\n|__________________________________________|\n");
		printf("|\t\t\t\t\t   |\n");
	}
	
	//启动接收线程 
	pthread_create(&id,NULL,(void *)showMsg,NULL); 
	
	printf(":");
	scanf("%s",msg);
	printf("[\33[33m发送成功]\33[0m\n");
	while(1){
		msgLen = strlen(msg);
		
		if(strcmp(msg,"exit")==0){
			pthread_cancel(id);
			if(send(curSockfd,"mexit",8,0)<0)
				perror("send_exit");
			//printf("\t准备退出\n");
			return;
		} 
		
		if(msgLen>99)
			printf("\t超出字数限制\n");
		else{
			strcpy(sendMsg,"m");
			strcat(sendMsg,msg);
			if(send(curSockfd,sendMsg,128,0)<0)
				perror("send_msg");
		}
		printf("\n:");
		scanf("%s",msg);
		printf("[\33[33m发送成功]\33[0m\n");
	}
	pthread_cancel(id);
	return;
}

//群聊 
void groChat(void)
{
	int len;
	char msg[256],sendMsg[256];
	pthread_t id;
	printf("%s","\033[1H\033[2J"); 
	printf("\t\33[34m----------\33[32m名媛聊天室\33[34m----------\33[0m\n");
	pthread_create(&id,NULL,(void *)showGro,NULL);
	
	
	printf("\33[s");//保存位置
	printf("\33[19;1H");//下到底部
	while(1){
		printf("\33[20;1H\33[K");
		printf("\33[21;1H\33[K");
		printf("\33[22;1H\33[K");
		printf("\33[23;1H\33[K");
		printf("\33[19;1H\33[K");
		printf(":");
		scanf("%s",msg);
		if(strcmp(msg,"exit")==0){
			pthread_cancel(id);
			if(send(curSockfd,"mexit",8,0)<0)
				perror("send_exit");
			return;
		} 
		len = strlen(msg);
		if(len>128)
			printf("\t超出字数限制\n");
		else{
			strcpy(sendMsg,"m");
			strcat(sendMsg,msg);
			if(send(curSockfd,sendMsg,256,0)<0)
				perror("send_msg");
			printf("[\33[32m%s \33[0m]\n",FSCG);
			sleep(1);
		}
	}
	pthread_cancel(id);
	return;
}

//发红包 
void sendPack()
{
	int num;
	float sum;
	char buf[32],msg[32];
	//输入金额
	while(1){
		printf("\33[0m请输入(金额):\33[32m");
		scanf("%f",&sum);
		if(sum>0)
			break;
		printf("\33[0m\t\t\33[41m金额有误\33[0m\n\n");
	}
	
	sprintf(buf,"%.2f",sum);
	strcpy(msg,"f");
	strcat(msg,buf);
	if(send(curSockfd,msg,32,0)<0)
		perror("send_sum");
	if(recv(curSockfd,buf,32,0)<0)
		perror("recv");
	if(strcmp(buf,"error")==0){
		printf("\33[0m\t\t\33[45m余额不足\33[0m\n\n");
		sleep(1);
		return;
	}
	//输入个数
	while(1){
		printf("\33[0m请输入(个数):\33[32m");
		scanf("%s",buf);
		num = atoi(buf);
		if(num<=0){
			printf("\33[0m\t\t\33[41m搁这玩呢\33[0m\n\n");
			continue; 
		}
		if(sum/num>0.009999)
			break;
		printf("\33[0m\t\t\33[45m钱不够分\33[0m\n\n");
	}
	strcpy(msg,"i");
	sprintf(buf,"%d",num);
	strcat(msg,buf);
	if(send(curSockfd,msg,32,0)<0)
		perror("send_num");
	printf("\33[0m\t\33[46mTips:红包将被拆分成%d个\33[0m\n\n",num);
	//输入祝语 
	while(1){
		memset(buf,0,sizeof(buf));
		printf("请输入(祝语):");
		scanf("%s",buf);
		if(testLen(buf)<14)
			break;
		printf("\33[0m\t\t\33[45m字数超限\33[0m\n\n");
	}
	strcpy(msg,"m");
	strcat(msg,buf);
	if(send(curSockfd,msg,32,0)<0)
		perror("send_msg");
	
	if(recv(curSockfd,buf,32,0)<0)
		perror("recv_out");
	if(strcmp(buf,"ok")==0)
		printf("\33[0m\t\t\33[42m发送成功\33[0m\n\n");
	else
		printf("\33[0m\t\t\33[41m网络超时\33[0m\n\n");
	sleep(1);
	return;
}

void grabPack()
{
	int len,index;
	char buf[32],msg[32];
	if(recv(curSockfd,buf,32,0)<0)
		perror("recv_len");
		
	len = atoi(buf);
	
	if(len==0)
		return;
	printf("\n");
	while(1){
		memset(buf,0,strlen(buf));
		printf("输入(红包序号):");
		scanf("%s",buf);
		index = atoi(buf);
		if(index>=0&&index<len)
			break;
		printf("\t\33[41m输入有误\33[0m\n");
		
	}
	strcpy(msg,"i");
	strcat(msg,buf);
	if(send(curSockfd,msg,32,0)<0)
		perror("send_index");
	
	if(recv(curSockfd,buf,32,0)<0)
		perror("recv_out");
	printf("\n      ");
	if(strcmp(buf,"over")==0)
		printf("\t\33[41m红包已失效\33[0m\n");
	else if(strcmp(buf,"had")==0)
		printf("\t\33[41m人心不足蛇吞象\33[0m\n");
	else if(strcmp(buf,"end")==0)
		printf("\t\33[46m没抢上热乎的\33[0m\n");
	else
		printf("\t\33[41m恭喜抢到%s元\33[0m\n",buf);
	sleep(1);
	return;
}

//---------------------------------------------------------------------

int testLen(char msg[])
{
	int j=0,count=0,msgLen=0;
	for(j=0;msg[j];j++){
		if(msg[j] & 0x80)
			count++;
		else
			msgLen++;
		if(count==3){
			count=0;
			msgLen+=2;
		}
	}
	msgLen--;
	return msgLen;
}

//强制修改状态 
void setState()
{
	char name[32];
	printf("需要强制下线的用户:");
	scanf("%s",name);
	
	if(send(curSockfd,name,32,0)<0)
		perror("send");
	return;
}

//输出闺蜜列表
void listFriends()
{
	int i=1,num;
	char buf[32],fidname[32];

	//Step1:接收好友数
	if(recv(curSockfd,buf,32,0)<0)
		perror("recv_num");
	num = atoi(buf); 
	if(num==0){
		printf("#system msg：无一无靠欸！\n");
		return;
	}
	//printf("num=%d\n",num);
	//Step2：while(num--)	recv(fidname);	printf(difname);
	printf("   ---\33[32m好友-列表\33[0m---\n");
	while(num--){
		if(recv(curSockfd,fidname,32,0)<0)
			perror("recv_fidname");
		printf("      %d %s\n",i++,fidname);
	}
	return;
}


void showMsg()
{
	char buf[1024];
	while(1){
		if(recv(curSockfd,buf,128,0)<0)
			perror("recv_msg");
		printf("\t\33[32m%s\33[0m\n\n",buf);
	}
	return;
}

void showGro()
{
	int i;
	int count = 0;
	char buf[1024];
	while(1){
		if(recv(curSockfd,buf,256,0)<0)
			perror("recv_msg");
		count ++;
		printf("\33[u");
		printf("\t\33[44m%s\33[0m\n\n",buf);
		if(count>7){
			sleep(2);
			printf("%s","\033[1H\033[2J"); 
			printf("\t\t\33[33m（\33[32m清屏幕结束\33[33m)\33[0m\n");
			printf("\t\33[34m----------\33[32m名媛聊天室\33[34m----------\33[0m\n");
			count=0;
		}
		printf("\33[s");
		printf("\33[19;1H\n");//下到底部
	}
	return;
}

void showPack(int index,char timeMsg[],int level,char name[],char bless[])
{
	int i,j,k,l,mod,len=12,wide=14;
	char re[8] = "\33[0m";
	mod = level%10;
	if(mod>7)
		mod -= 5;
		
	printf("\n\t%s%s%s\n",ft[mod%2+2],timeMsg,re);
	
	
	printf("%s    %s %slv%d%s %s\n",bg[mod],re,bg[1],level,re,name);
	printf("%s    %s\n",bg[mod],re);
	
	for(i=0;i<len;i++){
		printf("\t");
		
		//输出左边框 
		if(i==0){
			if(index<10)
				printf("%s%s0%d%s%s",bg[0],ft[6],index,re,bg[1]);
			else
				printf("%s%s%d%s%s",bg[0],ft[6],index,re,bg[1]);
		}else{
			printf("%s  %s",bg[0],bg[1]);
		}
		
		//输出正文
		if(i==0||i==len-1){
			printf("\33[40m");
			for(j=0;j<wide;j++)
				printf(" ");
			printf("\33[0m");
		}else if(i<4&&i>0){
			for(j=2;j<2*i;j++)
				printf(" ");
			printf("%s  %s",bg[0],bg[1]);
			k = wide-j;
			for(;j<k;j++)
				printf(" ");
			printf("%s  %s",bg[0],bg[1]);
			for(j=2;j<2*i;j++)
				printf(" ");
		}else if(i==4){
			for(j=1;j<wide/2;j++)
				printf(" ");
			printf("%s  %s",bg[3],bg[1]);
			for(j+=4;j<wide+3;j++)
				printf(" ");
		}else if(i==7){
			l = testLen(bless);
			for(j=0;j<(wide-l)/2;j++)
				printf(" ");
			printf("%s",bless);
			for(j+=l;j<wide-1;j++)
				printf(" ");
		}else{
			for(j=0;j<wide;j++)
				printf(" ");
		}
		
		//输出右侧
		printf("%s  %s\n",bg[0],re);
	}

}
