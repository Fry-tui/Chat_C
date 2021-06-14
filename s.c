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
//���ѽṹ��
struct Friend{
	int len;
	int state;
	char chatMsg[128][128];
	char fname[32];
}; 
//�ṹ��
struct User{
	char name[32];
	char password[32];
	char telenumber[16];
	//״̬��Ϣ 
	time_t start;			
	time_t time; 			//����ʱ�� 
	int num;				//���Ѹ��� 
	int state;				//����״̬ 
	int sockfd;				//sockfd�� 
	int gState;				//Ⱥ��״̬ 
	char pid[32];			//������� 
	float sum;				//�˻���� 
	//char friends[100][32];	
	//�������� 
	struct Friend friends[32];
	int addNum;				//δ����Ϣ 
	char addMsg[1024][32];	//������� 
	char addName[1024][32];	//��Ӻ��� 
	
	int len;
	char hisMsg[1024][128];//δ����Ϣ 
	//sem[0]andmsg[0]����ѡ����Ϣ 
	sem_t sem[1024];		//�ź���
	char msg[1024][1024];	//��Ϣ��¼ 
	//��0λ���ڴ��ѡ������ 
}; 

struct Redp{
	time_t time;		//����ʱ�� 
	
	int num;			//��ָ��� 
	int len;			//�������û���Ŀ 
	float sum;			//������ 
	char bless[32];		//���ף�� 
	char user[1024][32];	//�������û� 
	char timeMsg[32];	//ʱ����Ϣ 
	struct User host;	//������� 
};

//�̲߳���:����̴߳���ʱֻ�ܴ�һ������������ 
struct Para{
	pthread_t id;
	char name[32];
};


//����ڵ� 
typedef struct LnodeU{
	struct User user;
	struct LnodeU *next;
}LnodeU,*LinklistU;

typedef struct LnodeR{
	struct Redp redp;
	struct LnodeR *next;
}LnodeR,*LinklistR;

//--------------------------ȫ�ֱ���--------------------------------------- 
LinklistU U;	//��������ͷ�ڵ� 
LinklistR R;	//�������ͷ�ڵ� 
//------------------------------------------------------------------------- 

//==========================��������=======================================
//�˵��ຯ�� 
void reactReg(int);				//��Ӧע��
void reactLog(int); 			//��Ӧ����
void reactDes(int);				//�����û� 
void reactUserMenu(int); 		//��Ӧ�û��˵� 
void reactMainMenu(int *);		//��Ӧ��ҳ��˵� 
//���� 
void priChat(int);				//˽�� 
void groChat(int);				//Ⱥ�� 
void rtChat(int *);				//ʵʱ�Ի� 
void reactAdd(int);				//������� 
void reactSet(int);				//ǿ���޸� 
void listPack(int);				//���к�� 
void reactPush(int);			//�û���ֵ
void reactTran(int);			//�û�ת�� 
void reactSend(int);			//���ͺ�� 
void InitLink(void);			//��ʼ����
void readFile(void);			//��ȡ�ļ� 
void listUser(void);			//�����û�
void writeFile(char);			//д���ļ�
void listFriends(int);			//�������
void listHistory(int);			//�����Ϣ 
void reactMakeFriends(int); 	//������� 
void reactMsg(struct Para *);	//������Ϣ
void addNode(char,struct User,struct Redp);//��ӽڵ� 
//����
int findName(char[]);				//�����û�
int findState(char[]);				//��ѯ״̬
int findSockfd(char[]);				//��ѯsock 
int findChatState(char[],char[]);	//��ѯ״̬ 
char *myRecv(int);					//��װ����
char *findPwd(char[]);				//�������� 
void rmSem(int);					//�����ź� 
void Inspection(void); 				//�����Ч�� 
void setGState(int,int);			//�޸�Ⱥ��״̬ 
void setState(char[],int);			//�޸�״̬ 
void setPid(char[],char[]);			//�޸Ľ��̺� 
void setSockfd(char[],int);			//�޸�sock�� 
void saveHis(char[],char[]);		//����his 
void setChatState(char[],int);		//����״̬λ 
void rmArr(char[][32],int,int);		//�Ƴ�Ԫ�� 
void saveMsg(char[],char[],char[]);	//����msg 
void addTime(char[],time_t,time_t);	//����ʱ�� 
//========================================================================= 

int main(int argc,char *argv[])
{
	int *p;
	int sockfd;		//��ŷ�����̵�sockfd
	int newSockfd;	//���¿ͻ��˽������ӵ�sockfd�� 
	
	pthread_t id;	//�̺߳� 
	
	//�׽������ӵ�׼������ 
	int addrlen;
	struct sockaddr_in serv_addr,my_addr;
    serv_addr.sin_port = atoi(argv[1]);
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    memset(serv_addr.sin_zero,0,8);
     
    //��ʼ������
	InitLink();
	//��ȡ�����ļ�
	readFile();
    
    printf("%s","\033[1H\033[2J"); 
    printf("---����������---\n"); 
    //��ȡsockfd[������̵�sockfd:����] �ж�sockfd 
    sockfd = socket(AF_INET,SOCK_STREAM,0);
	if(sockfd<0)
		perror("socket");
	//��
	if(bind(sockfd,(struct sockaddr *)&serv_addr,sizeof(struct sockaddr))<0)
		perror("bind");
	//����
	if(listen(sockfd,100)<0)
		perror("listen");
	//�ȴ����� 
	while(1){
		newSockfd = accept(sockfd,(struct sockaddr *)&my_addr,&addrlen);
		if(newSockfd<0)
            perror("accept");
        printf("#system msg:\t�ͻ���%d",newSockfd);
	    printf(" \033[32m�ѿ�������\033[0m\n");
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
	//�Կͻ��˵����˵�������Ӧ 
	int len;
	char buf[1024];
	
	//printf("���߳�Ϊ%d�ͻ��˷���\n",*sockfd);
	while(1){
		//printf("������\n");
		memset(buf,0,sizeof(buf));
		//����ѡ�� 
		strcpy(buf,myRecv(*sockfd));
		
		//�ж�ѡ�� 
		if(strcmp(buf,"1")==0){
			reactReg(*sockfd);
		}else if(strcmp(buf,"2")==0){
			reactLog(*sockfd);
		}else if(strcmp(buf,"3")==0){
			//reactSet();
		}else if(strcmp(buf,"4")==0){
			reactDes(*sockfd); 
		}else if(strcmp(buf,"5")==0){
			//�����ÿͻ���
			close(*sockfd);
	        printf("#system msg:\t�ͻ���%d",*sockfd);
	        printf(" \033[33m�ѶϿ�����\033[0m\n");
	        pthread_exit(0);
		}else if(strcmp(buf,"ls")==0){
			listUser();
		}else if(strcmp(buf,"set")==0){
			//����״̬
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
	//���պ�����ֻ��� 
	strcpy(user.telenumber,myRecv(sockfd));
	
	//��ȡ��֤�벢���� 
	while(1){
		srand(time(NULL));
		num = rand()%9000+1000;
		sprintf(buf,"%d",num);//ת����char���� 
		if(send(sockfd,buf,8,0)<0)
			perror("send");
		
		//���ձȶ���Ϣ 
		strcpy(buf,myRecv(sockfd));
		
		//�˳����� 
		if(strcmp(buf,"same")==0)
			break;
	}
	//�����û��� 
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
	
	//������û�����
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
	
	//�������
	if(send(sockfd,"success",32,0)<0)
		perror("send");
	
	printf("#system msg:%s�����Ѽ���\n",user.name);
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
		
	
	//��������״̬
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
 
 	
	//Step1�����ո��û�����ʱ��Ӧ�ͻ��˵Ľ��̺� 
	addTime(name,0,time(NULL));
	strcpy(pid,myRecv(sockfd));
	setPid(name,pid);		//�޸��û����̺� 
	setState(name,1);		//�޸ĵ���״̬ 
	setSockfd(name,sockfd);	//�޸�sockfd 
	setChatState(name,0);	//��������״̬λ 
	writeFile('U');			//д���ļ� 
	
	//Step2��׼���̲߳����ṹ��(linux�нṹ��ָ����Ҫ���ٿռ����ʹ��)
	temp = (struct Para *)malloc(sizeof(struct Para));
	strcpy(temp->name,name);//д���û���
	temp->id = pthread_self();//д�����߳��̺߳� 
	//printf("��ʼid = %lu\n",pthread_self());
	
	//Step3��������Ӧ��Ϣ���߳� ��������� 
	pthread_create(&id,NULL,(void *)reactMsg,(void *)temp);
	
	//Step4�����봦���û��˵����� 
	reactUserMenu(sockfd);
	
	//Step5�����û��˵��˳����һϵ�����߲���
	/*[�Ҿ��İɣ�д��reactUserMenu()�������ù淶һ�㣬
	����Ҫ�ĺ���������������������] */ 
	addTime(name,time(NULL),0);
	setGState(sockfd,0);	//����Ⱥ������ 
	setState(name,0);		//����¼״̬����λ���� 
	setPid(name,"-1");		//���û����̺���Ϊ-1 
	setSockfd(name,-1);		//���û�sockfd����Ϊ-1 
	writeFile('U');			//д�뱾���ļ� 
	pthread_cancel(id);		//�رս����߳�
	
	//Step6����ʾ�˳� 
	printf("#system msg:\t\33[34m%s\33[0m �������˳����룡\n",name);
	
	//�˻����˵� 
	return;
}

//�����û�
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
			
			//�رպ��ѹ�ϵ
			for(i=0;i<u->user.num;i++){
				q = U;
				while(q){
					if(strcmp(q->user.name,u->user.friends[i].fname)==0){
						for(j=0;j<q->user.num;j++){
							if(strcmp(q->user.friends[j].fname,u->user.name)==0){
								//�����ʾ 
								//#���͵�hisMsg 
								strcpy(msg,"# ");
								strcat(msg,u->user.name);
								strcat(msg,"���� ��ע���˻�");
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
			
			//�����к��ʧЧ
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

//������Ϣ 
void reactMsg(struct Para *temp)
{
	//int opt=1;
	int i,j,k,len,sockfd;
	char buf[1024],msg[1024];
	LinklistU u = U->next;
	//�����û���ִ����Ӧ���� 
	while(u){
		//�жϽڵ��Ƿ�Ϊ����ڵ� 
		if(strcmp(u->user.name,temp->name)==0){
			//�ҵ��ڵ��,�Ƚ�sockfd��ȡ���� 
			sockfd = u->user.sockfd;
			//ѭ��������Ϣ 
			while(1){
				//���buf[���Ե����Ҿ���û��Ҫ] 
				memset(buf,0,sizeof(buf));
				//��ȡ��Ϣ 
				len = recv(u->user.sockfd,buf,1024,0);
				//printf("sockfd\tbuf\tlen\n");
				//printf("%d\t|%s|\t%d\n",u->user.sockfd,buf,len);
				//���ʣ������set-1��ʱ�򲻾��ղ�������
				//����-1��ʱ�� len�ͻ���-1�����Կ���len<0���ж������ǿ����������Ӧ 
				if(len<0){ 
					//perror("#reactMsg_recv");�����Լ�����ʾ�ˣ����Ϳ��Բ����� 
					//����������ǿ�����ߵ�ʱ���ҿ���ɱ�����Ľ���
					//setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, (const void *)&opt, sizeof(opt));
					//printf("len<0\n");
					//Step1���رն˿� 
					close(sockfd);//��һ��д����shutdown(sockfd,SHUT_RDWR);
					
					//Step2��׼��ɱ���ͻ����߳�ָ��
					strcpy(buf,"kill ");
					strcat(buf,u->user.pid);
				
					//Step3�������ͻ��˵Ľ���
					system(buf); 
					
					//Step4���Ե�ǰ�û�ִ�����߲��� 
					u->user.state = 0;
					u->user.gState = 0;
					
					//printf("��������\n");
					for(k=0;k<u->user.num;k++){
						u->user.friends[k].state=0;
					}
					setSockfd(u->user.name,-1);
					setPid(u->user.name,"-1");
					if(u->user.start>0){
						u->user.time = u->user.time+time(NULL)-u->user.start;
						u->user.start=0;
					}
					writeFile('U');//���浽���� 
					//printf("%s->%d\n",u->user.name,u->user.gState);
					//Step4���ر����߳� 
					pthread_cancel(temp->id);
					
					//Step5����ʾ��� 
					printf("#system msg:\t\33[34m%s\33[0m �����ѱ�ǿ�����ߣ�\n",u->user.name);
			        printf("#system msg:\t�ͻ���%d",sockfd);
			        printf(" \033[33m�ѶϿ�����\033[0m\n");
			        
			        //Step6���رո��߳�/�����߳�/���߳�/reactMsg 
					pthread_exit(NULL);
				}else if(len == 0){
					//ִ�����߲���
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
					//�ر����߳� 
					pthread_cancel(temp->id);
					//printf("id = %lu\n",temp->id);
					
					//�رն˿� ����Ӧ����ʾ��� 
					close(sockfd);
					printf("#system msg:\t\33[34m%s\33[0m ����ǿ�����ߣ�\n",u->user.name);
			        printf("#system msg:\t�ͻ���%d",sockfd);
			        printf(" \033[33m�ѶϿ�����\033[0m\n");
			        
			        //�رյ�ǰ�߳� 
					pthread_exit(NULL);
				}
				
				//������Ϣ
				j = strlen(buf);
				memset(msg,0,1024);
				for(i=1;i<j;i++){
					msg[i-1] = buf[i];
				} 
				//�ȶ���Ϣ���� 
				if(strcmp(buf,"ǿ�����߿ͻ��˵Ľӿ�")==0){
					//����ֻ��ͨ������Ϣ�����ƿͻ��� 
					//��ʲô�취����ֱ�ӻص�userMenu 
					//Ŀǰû�й� 
					//��ʵҲ���еģ��ѿͻ��˵Ľ��̽����������½������� 
				}else if(buf[0]=='c'){
					//ѡ������Ϣ 
					strcpy(u->user.msg[0],msg);
					sem_post(&u->user.sem[0]);
				}else if(buf[0]=='f'){
					//���������
					strcpy(u->user.msg[3],msg);
					sem_post(&u->user.sem[3]); 
				}else if(buf[0]=='n'){
					//��������Ϣ
					strcpy(u->user.msg[1],msg);
					sem_post(&u->user.sem[1]); 
				}else if(buf[0]=='m'){
					//��֤��Ϣ
					strcpy(u->user.msg[2],msg);
					sem_post(&u->user.sem[2]);
				}else if(buf[0]=='i'){
					//��֤��Ϣ
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
		printf("�̼߳���ʧ�ܣ�\n");
		pthread_cancel(temp->id);
	}
}

//��Ӧ�û��˵� 
void reactUserMenu(int sockfd)
{
	int i,j;
	int *p;
	char buf[1024],temp[32],num[32];
	LinklistU u = U->next;
	/*
	�������߳�����״̬�����ж��Ƿ��б���ʱ�ر� 
	while(1){
		sleep(3);
		printf("���߳�ing...\n");
	}
	*/
	while(u){
		if(u->user.sockfd==sockfd){
			//����������־
			rmSem(sockfd); 
			/*
			p = (int *)malloc(sizeof(int));
			sem_getvalue(&u->user.sem[0],p);//��ȡsemֵ
			while(*p){
				sem_wait(&u->user.sem[0]);//����ֵ������
				*p = *p-1;
			}
			sem_getvalue(&u->user.sem[1],p);//��ȡsemֵ
			while(*p){
				sem_wait(&u->user.sem[1]);//����ֵ������
				*p = *p-1;
			}
			*/
			
			while(1){
				//����δ����Ϣ���� 
				sprintf(buf,"%d",u->user.addNum);
				if(send(sockfd,buf,32,0)<0)
					perror("send_addNum");
				sprintf(buf,"%d",u->user.len);
				if(send(sockfd,buf,32,0)<0)
					perror("send_len");
				//�ȴ���Ϣ�ź��� 
				sem_wait(&u->user.sem[0]);
				//��ȡ���յ�����Ϣ 
				strcpy(buf,u->user.msg[0]);
				if(strcmp(buf,"1")==0){
					//�����û����ݵ�ʱ�򣬽������������ֱ�Ӵ���ȥ 
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
		printf("# system msg:�˿��쳣\n");
		return;
	}
	return;
}

//+++++++++++++++++++++++++++++++++��������++++++++++++++++++++++++++++++++
//��װ���պ��� 
char *myRecv(int sockfd)
{
	int len;
	static char buf[1024];
	LinklistU u = U->next;
	memset(buf,0,sizeof(buf));
	//�������� 
	len = recv(sockfd,buf,1024,0);
	if(len<0)
		perror("recv");
	else if(len==0){
		//����ǰ�������û����룬��Ҫǿ������
		while(u){
			if(u->user.sockfd==sockfd){
				u->user.state=0;
				u->user.sockfd=-1;
				u->user.gState = 0;
				if(u->user.start>0){
					u->user.time = time(NULL)+u->user.time-u->user.start;
					u->user.start = 0;
				}
				//printf("��������\n");
				writeFile('U');
				//printf("%s->%d\n",u->user.name,u->user.gState);
				break;
			}
			u = u->next;
		}
		 
		close(sockfd);
	    printf("#system msg:\t�ͻ���%d",sockfd);
	    printf(" \033[33m�ѶϿ�����\033[0m\n");
        pthread_exit(0);
	}
	return buf;
}

//�����û� 
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

//��ѯ����
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

//��ѯ״̬ 
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

//��ѯ����״̬
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

//��ȡsockfd�� 
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

//�޸�״̬
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

//�޸�Ⱥ��״̬
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

//�޸�sockfd 
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

//�޸Ľ��̺�
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

//����״̬λȫ������ 
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

//���õ���ʱ��
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

//�Ƴ�����Ԫ�� 
void rmArr(char arr[][32],int index,int len)
{
	for(;index<len-1;index++)
		strcpy(arr[index],arr[index+1]);
	return;
}

//�����ź� 
void rmSem(int sockfd)
{
	int *p,i;
	LinklistU u = U->next;
	while(u){
		if(u->user.sockfd==sockfd){
			for(i=0;i<1024;i++){
				p = (int *)malloc(sizeof(int));
				sem_getvalue(&u->user.sem[i],p);//��ȡsemֵ
				while(*p){
					sem_wait(&u->user.sem[i]);//����ֵ������
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

//������Ϣ
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
			//��δ����Ϣ����ʱ���������
			strcpy(u->user.hisMsg[u->user.len++],msg); 
			return;
		}
		u = u->next;
	}
	return;
}

//haveuser����
//time = 0���� 
//sum = 0������

//��ѯ����ʱ����Ҫ�˻�
//������ʱ���˻�
//�к����ʱ���˻� 
//�����Ч��
void Inspection()
{
	char msg[128],sum[32];
	LinklistR r = R->next;
	LinklistU u = U->next;
	while(r){
//���Ե�:3�����˻� 
		if(r->redp.time==0){
			r = r->next;
			continue;
		}
		if(time(NULL)-r->redp.time>SPIN){
			printf("��⵽��\n"); 
			u = U->next;
			while(u){
				if(strcmp(u->user.name,r->redp.host.name)==0){
					u->user.sum+=r->redp.sum;
					if(r->redp.sum!=0){	
						strcpy(msg,"#");
						strcat(msg," ����˻� ");
						sprintf(sum,"%.2f",r->redp.sum);
						strcat(msg,sum);
						strcat(msg,"Ԫ"); 
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
//--------------------------------���ܺ���---------------------------------- 
//��ʼ��������
void InitLink()
{
	//Ϊͷ�ڵ㿪�ٿռ�
	U = (LinklistU)malloc(sizeof(LnodeU));
	R = (LinklistR)malloc(sizeof(LnodeR));
	//��ÿ�������β��ָ��NULL 
	U->next = NULL;
	R->next = NULL; 
}

//д�뱾���ļ�
void writeFile(char type)
{
	FILE *fp;
	LinklistU u = U->next;
	LinklistR r = R->next;
	if(type=='U'){
		//������������ 
		fp = fopen("./data/user.dat","w+b");
		while(u){
			fwrite(&u->user,sizeof(struct User),1,fp);
			u = u->next;
		}
	}else if(type=='R'){
		//���������� 
		fp = fopen("./data/redp.dat","w+b");
		while(r){
			fwrite(&r->redp,sizeof(struct Redp),1,fp);
			r = r->next;
		}
	}else{
		printf("�޷�ʶ��Ҫ�������������\n");
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
	//���ݴ�������ѡ�����
	if(type == 'U'){
		//����û��ڵ� 
		while(u->next) 
			u = u->next;
		p = (LinklistU)malloc(sizeof(LnodeU));
		p->user = user;
		u->next = p;
		p->next = NULL;
	}else if(type == 'R'){
		//��Ӻ���ڵ� 
		while(r->next)
			r = r->next;
		q = (LinklistR)malloc(sizeof(LnodeR));
		q->redp = redp;
		r->next = q;
		q->next = NULL;
	}else{
		//�������� 
		printf("Error:Type\n");
	}

	return;
}

//��ȡ�����ļ� 
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
	printf("|����\t����\t�ֻ���\t\t���\tsockfd\t pid\t����\\����   ������ |\n");
	while(u){
		printf("|%s\t%s\t%s\t%.2f\t  %d\t%s\t",u->user.name,u->user.password,u->user.telenumber,u->user.sum,u->user.sockfd,u->user.pid);
		if(u->user.state)
			printf("\033[32m ����\033[0m");
		else
			printf("\033[31m ����\033[0m");
		printf("\t      %d\t   |\n",u->user.num);
		//temp = (int *)malloc(sizeof(int));
		//sem_getvalue(&u->user.sem[1],temp);
		//printf("|�ź�����%d ѡ�%s\t\t\t\t\t\t\t   |\n",*temp,u->user.msg[1]);
		u = u->next;
	}
	printf("----------------------------------------------------------------------------\n");
	return;
}

//�����б�->�ͻ��� 
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

//��Ӻ��� 
void reactMakeFriends(int sockfd)
{
	int i;
	char buf[32],name[32],fname[32];
	LinklistU u = U->next,p = U->next;
	while(u){
		if(u->user.sockfd==sockfd){
			sem_wait(&u->user.sem[1]);
			strcpy(fname,u->user.msg[1]);
			//�ж��Ƿ��Ѿ��Ǻ��� 
			for(i=0;i<u->user.num;i++){
				if(strcmp(u->user.friends[i].fname,fname)==0){
					if(send(sockfd,"had",32,0)<0)
						perror("send");
					return; 
				}
			}
			//�ж��Ƿ���� 
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
			
			//�����������ʾ���� 
			if(send(sockfd,"ok",32,0)<0)
				perror("send");
			
			//��ȡ ��������	
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

//������� 
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
					//����Է��û����ڣ����ߵĺ��������໥����
					while(p){
						//�Է��û� 
						if(strcmp(p->user.name,u->user.addName[i])==0){
							p->user.friends[p->user.num].state=0;
							strcpy(p->user.friends[p->user.num++].fname,u->user.name);
							strcpy(msg,"# ");
							strcat(msg,u->user.name);
							strcat(msg," ��ͨ��������֤");
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

//��ֵ 
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

//�û�ת��
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
						//�Է��û� 
						if(strcmp(p->user.name,u->user.addName[i])==0){
							strcpy(p->user.friends[p->user.num++].fname,u->user.name);
							strcpy(msg,"#");
							strcat(msg,u->user.name);
							strcat(msg," ��ͨ��������֤");
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
							strcat(msg," ����ת�� ��");
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

//δ����Ϣ 
void listHistory(int sockfd)
{
	int i,len=0,num=0,j,k,arr[1024]={0};
	char buf[32],name[1024][32],addMsg[1024][128],temp[32]="none";
	LinklistU u = U->next;
	Inspection();
	//printf("��ʼ��\n");
	while(u){
		if(u->user.sockfd==sockfd){
			//������Ϣ����  
			for(i=0;i<u->user.len;i++){
				//printf("msg:%s\n",u->user.hisMsg[i]);
				//��#���͵���Ϣ��ӵ�addMsg�� 
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
			//�����֤�����Ϣ
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

//���к��
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
		//����ʱ���ı�
		if(send(sockfd,r->redp.timeMsg,32,0)<0)
			perror("send_timeMsg");
		//���͵ȼ�
		i = r->redp.host.time;
		sprintf(buf,"%d",i);
		if(send(sockfd,buf,32,0)<0)
			perror("send_time");
		//��������
		if(send(sockfd,r->redp.host.name,32,0)<0)
			perror("send_name");
		//����ף��
		if(send(sockfd,r->redp.bless,32,0)<0)
			perror("send_bless"); 
		
		r=r->next;
	}
	return;
}

void priChat(int sockfd)
{
	int i,j,k,l;
	int len=0;//��Ϣ���г��� 
	int index;//�������� 
	int flag; //����״̬λ 
	int state;//����״̬ 
	int fsock;//����sockfd 
	char buf[32],name[32]="none",fname[32],showMsg[32][128],msg[128],sendMsg[128];
	LinklistU u = U->next,q = U->next; 
	while(u){
		if(u->user.sockfd==sockfd){	
			//Step 1:��������б�
			listFriends(sockfd);
			//Step 2:��ȡ�������
			sem_wait(&u->user.sem[1]);
			strcpy(fname,u->user.msg[1]);
			
			for(index=0;index<u->user.num;index++){
				if(strcmp(fname,u->user.friends[index].fname)==0){
					u->user.friends[index].state = 1; 
					
//---------------------//����ᱻ���£������������� 
					//��ȡ����sockfd
					fsock = findSockfd(fname); 
					
					//��ʾ�ɹ��������� 
					if(send(sockfd,"ok",32,0)<0)
						perror("send_ok");
				//	printf("fname = %s\n",fname);	
					l = 0;
					for(i=0;i<u->user.len;i++){
						//Step 5:���hisMsg[32] 
						if(u->user.hisMsg[i][0]=='$'){
							j=1;
							k=0;
							memset(name,0,strlen(name));
							while(u->user.hisMsg[i][j]!=':')
								name[k++]=u->user.hisMsg[i][j++];
							//printf("name = %s\n",name);
							if(strcmp(name,fname)==0){
								strcpy(showMsg[len++],u->user.hisMsg[i]);
								//��Ϣ��������� 
								
								//������Ҫ�����浽��ʷ��¼
								saveMsg(u->user.name,fname,u->user.hisMsg[i]);
								continue;
							} 
						}
						strcmp(u->user.hisMsg[l++],u->user.hisMsg[i]);
					}
					u->user.len = l;
					writeFile('U');
					//printf("i = %d\tlen = %d\n",i,len);
					//Step 6:��ѯ��ʷ��Ϣ 
					//i,j,k,name���� 
					if(len==0)
						for(;len<6&&len<u->user.friends[index].len;len++)
							strcpy(showMsg[len],u->user.friends[index].chatMsg[u->user.friends[index].len+len-6]);
					
					//Step 7:���ַ��� 
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
					//Step 8:��ʼ���� 
					while(1){
						sem_wait(&u->user.sem[2]);
						strcpy(msg,u->user.msg[2]);
						if(strcmp(msg,"exit")==0){
							//printf("�˳������\n");
							u->user.friends[index].state=0;
							return;
						}
						//��װ��Ϣ
						strcpy(sendMsg,"$");
						strcat(sendMsg,u->user.name);
						strcat(sendMsg,":");
						strcat(sendMsg,msg);
						
						//�������״̬λ 
						flag = findChatState(fname,u->user.name);
						//printf("\t��װ��:%s,����״̬:%d\n",sendMsg,flag);
						if(flag == 1){
							//�رս����߳� 
							//����
							fsock = findSockfd(fname); 
							if(send(fsock,sendMsg,128,0)<0)
								perror("send_msg");
							//���浽��ʷ��Ϣ
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
			//Step 4:�޺���->�˳�  none 
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

//Ⱥ��
void groChat(int sockfd)
{
	char buf[256],msg[256];
	LinklistU u = U->next,p = U->next;
	while(u){
		if(u->user.sockfd == sockfd){
			u->user.gState = 1;
			
			while(1){
				//��֪ϵͳ����Ⱥ��, 
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
			//Step1:��ȡ��� 
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
			//Step2:��ȡ���� 
			sem_wait(&u->user.sem[4]);
			strcpy(buf,u->user.msg[4]);
			rp.num = atoi(buf);
			//Step3:��ȡף�� 
			sem_wait(&u->user.sem[2]);
			strcpy(rp.bless,u->user.msg[2]);
			
			//Step4:�������� 
			//���µȼ�
			if(u->user.start>0){
				u->user.time = time(NULL)-u->user.start+u->user.time;
				u->user.start = time(NULL);
			} 
			rp.host = u->user;
			
			//Step5:����б� 
			rp.len=0;
			for(i=0;i<1024;i++)
				memset(rp.user[i],0,sizeof(rp.user[i]));
				
			//Step6:����ʱ���ı���д��ʱ�� 
			time(&rp.time);
			strcpy(rp.timeMsg,asctime(gmtime(&rp.time)));
			
			//Step7:�۳����
			u->user.sum -= rp.sum;
			
			//Step8:��ӽڵ� 
			addNode('R',user,rp);
			
			//Step9:���������� 
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
			
			//�жϺ���Ƿ��ѱ����� 
			if(r->redp.num==0){
				if(send(sockfd,"end",32,0)<0)
					perror("send_end");
				return;
			}
			
			//Step1:�ж�ʧЧ
			if(r->redp.time==0){
				if(send(sockfd,"over",32,0)<0)
					perror("send_over");
				return;
			}
//���Ե�180s 
			//Step2:�����Ч��
			if(time(NULL)-r->redp.time>SPIN){
				q = U->next;
				while(q){
					if(strcmp(q->user.name,r->redp.host.name)==0){
						q->user.sum+=r->redp.sum;
						//# ����˻� %.2fԪ
						strcpy(msg,"#");
						strcat(msg," ����˻� ");
						sprintf(sum,"%.2f",r->redp.sum);
						strcat(msg,sum);
						strcat(msg,"Ԫ"); 
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
			
			//Step3:�ж�ռ��
			for(i=0;i<r->redp.len;i++){
				if(strcmp(r->redp.user[i],u->user.name)!=0)
					continue;
				
				if(send(sockfd,"had",32,0)<0)
					perror("send_had");
				return;
			}
			
			//Step4:�ж���� [������ȥ]
			if(r->redp.sum==0||r->redp.num==0){
				if(send(sockfd,"end",32,0)<0)
					perror("send_end");
				return;
			}
			
			memset(buf,0,sizeof(buf));
			//�����
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

//��Ӧǿ������ 
void reactSet(int sockfd)
{
	char name[32];
	LinklistU u = U->next;
	strcpy(name,myRecv(sockfd));
	setSockfd(name,-1);
	setState(name,0);
	return;
}

