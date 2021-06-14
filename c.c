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
//--------------------------ȫ�ֱ���--------------------------------------- 
int curSockfd;						//��ŵ�ǰ���̵�sock��
struct sockaddr_in serv_addr;		//��������ַ�ṹ��(Ӧ���ǰ�) 
//------------------------------------------------------------------------- 
char ft[][8]={
	"\33[30m","\33[31m","\33[32m","\33[33m","\33[34m","\33[35m",
	"\33[36m","\33[37m","\33[38m","\33[39m"
};
char bg[][8]={
	"\33[40m","\33[41m","\33[42m","\33[43m","\33[44m","\33[45m",
	"\33[46m","\33[47m","\33[48m","\33[49m"
};
//==========================��������=======================================
int testLen(char []);			//�������� 
void myReg(void);				//ע�� 
void myLog(void);				//����
void myDes(void);				//ע��  
void priChat(void);				//˽��
void groChat(void);				//Ⱥ�� 
void showMsg(void);				//����
void showGro(void);				//���� 
void mainMenu(void);			//���˵�
void grabPack(void);			//����� 
void userMenu(void);			//�û�ҳ�� 
void sendPack(void); 			//���ͺ�� 
void userPush(void);			//�û���ֵ 
void userTran(void);			//�û�ת��
void setState(void);			//����״̬  
void listPack(void);			//���к�� 
void addFriends(int);			//��֤��Ϣ
void makeFriends(void);			//��Ӻ���
void listFriends(void);			//��������б�
void listHistory(void);			//�����ʷ��Ϣ 
void showPack(int,char[],int,char[],char[]);//�����ʽ 
//========================================================================= 

int main(int argc,char *argv[])
{
	int *p;
	int sockfd;		//���socket��������ֵ����ʶ��ǰ���̵�sockfd 
	pthread_t id;
	//socket�׽��ֵ�׼������ 
	serv_addr.sin_port = atoi(argv[2]);
	serv_addr.sin_family = AF_INET;
	serv_addr.sin_addr.s_addr = inet_addr(argv[1]);
	memset(serv_addr.sin_zero,0,8);
    //��ȡsockfd		�ж��Ƿ�ɹ�		��sockfd��ֵ��curSockfd 
	sockfd = socket(AF_INET,SOCK_STREAM,0);
	if(sockfd<0)
		perror("sockfd");
	curSockfd = sockfd;
	//��ʼ����,���ж��Ƿ�ɹ� 
	if(connect(sockfd,(struct sockaddr *)&serv_addr,sizeof(struct sockaddr))<0){
		perror("connect");
		return 0;
	}
	//����һ���߳������ж϶˿ں��Ƿ���Ч������Ч��exit(0);
	//pthread_create(&id,NULL,(void *)judgeConn,NULL);
	//�����ӳɹ�,����뺯��
	mainMenu();
	return 0;
}

void mainMenu()
{
	char buf[1024];
	
	while(1){
		printf("%s","\033[1H\033[2J"); 
		printf ("----------��----------------\n");
		printf ("--------------��------------\n");
		printf ("|\t1 ������֤\t   |\n");	//ע�� 
		printf ("|\t2 ��������\t   |\n");	//���� 
		printf ("|\t3 ������֤\t   |\n");	//�һ����� 
		printf ("|\t4 ��ط���\t   |\n");	//ע���˺� 
		printf ("|\t5 ����С�\t   |\n");	//�رմ��� 
		printf ("|\t\t\t   |\n");
		printf ("----------��----------------\n");
		printf ("--------------��------------\n");
		printf ("��ѡ��"); 
		scanf("%s",buf);
		//����ѡ�� 
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
			printf("\t��������,����������\n"); 
			sleep(1);
		}
	}
	return;
}

//�û�����ҳ�� 
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
		printf ("----------\33[34m��\33[0m-----------------\n");
		printf ("--------------\33[34m��\33[0m-------------\n");
		printf ("|\t#  ��֤��Ϣ%d\t   |\n",num);
		printf ("|\t@  δ����Ϣ%d\t   |\n",len);
		printf ("|\t*  ˢ����Ϣ��\t   |\n");
		printf ("|\t1  ���й�����\t   |\n");
		printf ("|\t2  ����˽����\t   |\n");
		printf ("|\t3  ����ƴϦϦ\t   |\n");
		printf ("|\t4  ������ת��\t   |\n");
		printf ("|\t5  �׳�����\t   |\n");
		printf ("|\t6  ͼ���һ��\t   |\n");
		printf ("|\t7  ƴ����ʱ��\t   |\n");
		printf ("|\t8  Ѱ�������\t   |\n");
		printf ("|\t10  ��ѯ���\t   |\n");
		printf ("|\texit �˳�����\t   |\n");
		printf ("----------\33[34m��\33[0m-----------------\n");
		printf ("--------------\33[34m��\33[0m-------------\n");
		printf ("��ѡ��"); 
		scanf("%s",buf);
		
		strcpy(msg,"c");
		strcat(msg,buf);
		
		if(send(curSockfd,msg,64,0)<0)
			perror("send");
		
		if(strcmp(buf,"1")==0){
			listFriends();
			printf("������˳�:");
			//ͻȻ����scanf����򵥵���������
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
			printf("\t�˻����:��%s\n",sum);
			sleep(1);
		}else if(strcmp(buf,"exit")==0){
			return;
		}else if(strcmp(buf,"#")==0){
			if(recv(curSockfd,numbuf,32,0)<0)
				perror("recv_num");
			num = atoi(numbuf);
			if(num==0){
				printf("\t��ʱû��Ҫ����\n");
				sleep(1);
				continue;
			}
			//������Ϣ
			addFriends(num); 
		}else if(strcmp(buf,"@")==0){
			if(recv(curSockfd,numbuf,32,0)<0)
				perror("recv_num");
			len = atoi(numbuf);
			if(len==0){
				printf("\t��Ϣ�����\n");
				sleep(1);
				continue;
			} 
			listHistory();
			printf("������˳�:");
			//ͻȻ����scanf����򵥵���������
			scanf("%s",buf);
		}else if(strcmp(buf,"*")==0){
			
		}else if(strcmp(buf,"show")==0){
			printf("\t���г����м�¼\n"); 
			sleep(1);
		}else{
			printf("\t��������,����������\n"); 
			sleep(1);
		}
	}
	return;
}

//----------------------------�����ຯ��-------------------------------- 
void myReg()
{
	int i;
	char buf[32],pwd[32],code[8],name[32],number[32];
	//�ֻ��� 
	while(1){
		printf("����(�ֻ���):");
		scanf("%s",number);

		//�ж�λ�� 
		if(strlen(number)==11){ 
			if(send(curSockfd,number,32,0)<0)
				perror("send");
			break;
		}
		printf ("--------------------------------------------------\n");
		printf ("|\t\t#system msg:\033[31m�ֻ�������\033[0m\t\t |\n");
		printf ("--------------------------------------------------\n");
		printf("������");
	} 
	//��֤��
	while(1){
		//���պ���
		if(recv(curSockfd,code,8,0)<0)
			perror("recv");
		
		printf("%s","\033[1H\033[2J"); 
		printf ("-------------------------------------------------\n");
		printf("|\t#system msg:������֤��:\033[33m%s\033[0m\t\t|\n",code);
		printf ("-------------------------------------------------\n");
		printf("������(��֤��):");
		scanf("%s",buf);
		if(strcmp(buf,code)==0){ 
			if(send(curSockfd,"same",32,0)<0)
				perror("send");
			break;
		}
		
		printf ("--------------------------------------------------\n");
		printf ("|\t\t#system msg:\033[31m��֤������\033[0m\t\t |\n");
		printf ("|\t#system msg:�����Զ���ȡ����֤��!\t |\n");
		printf ("--------------------------------------------------\n");
		if(send(curSockfd,"error",32,0)<0)
			perror("send");
		sleep(2);//��������
	} 
	printf("%s","\033[1H\033[2J"); 
	printf ("-----------------------------------------\n");
	printf("|\t\t\033[34m������֤����\033[0m\t\t|\n");
	printf ("-----------------------------------------\n");
	//���� 
	while(1){
		printf("����(������):");
		scanf("%s",name); 
		if(send(curSockfd,name,32,0)<0)
			perror("send");
		//�ȴ�����
		if(recv(curSockfd,buf,32,0)<0)
			perror("recv");
		//�жϴ������ 
		if(strcmp(buf,"exist")!=0)
			break;
		//��ʾ���� 
		printf("%s��ռ��\n������",name);
	}
	//��������
	for(i=0;i<3;i++){
		printf("%s","\033[1H\033[2J"); 
		printf ("-------------------------------------------------\n");
		printf ("|\t=���أ�=ʣ��\033[33m%d\033[0m���������=���أ�=\t\t|\n",3-i);
		printf ("-------------------------------------------------\n");
		printf ("������(������):");
		scanf  ("%s",pwd);
		printf ("��ȷ��(������):");
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
		printf("|\t#system msg:\033[35m����ð�����£���������������� �ݣ�\033[0m|\n");
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
		printf("( ��?��?)?(._.`) : \t�վ�������һ��,%s��ռ����\n",name);
		sleep(3);
	}else if(strcmp(buf,"success")==0){
		printf("%s","\033[1H\033[2J"); 
		printf("\n\n");
		printf("\t\33[32m�d(�RO�Q)��\33[0m�~ : �ɹ����������ڲ��˚G��\n");
		printf("\33[32m( ��?��?)?(._.`)\33[0m : �Ժ�ҲҪ������һ������Ŷ��\n");
		sleep(2);
	}
	return;
}

void myLog()
{
	int i;
	char buf[32],pwd[32],name[32];
	//��ȡ����
	while(1){
		printf("����(������):");
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
		printf("|\t\33[32mo(һ��һ+)o���ü����Ż���Ⱥ�ģ�o(һ��һ+)o\33[0m\t|\n");
		printf ("---------------------------------------------------------\n");
		printf("������");
	}
	//�ȶ�����
	for(i=0;i<3;i++){
		printf("%s","\033[1H\033[2J"); 
		printf ("-----------------------------------------\n");
		printf ("|\t=���أ�=ʣ��\033[33m%d\033[0m���������=���أ�=\t|\n",3-i);
		printf ("-----------------------------------------\n");
		printf ("������(����):");
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
		printf("|\t#system msg:\033[35m����ð�����£���������������� �ݣ�\033[0m|\n");
		printf ("--------------------------------------------------------\n");
		sleep(1);
		return; 
	}
	
	//�ж�������� 
	if(recv(curSockfd,buf,32,0)<0)
		perror("recv buf");
		
	if(strcmp(buf,"on-line")==0){
		printf("\n\t���������豸��¼!\n");
		sleep(2);
		return;
	}
	strcpy(buf,"");
	sprintf(buf,"%d",getpid());
	if(send(curSockfd,buf,32,0)<0)
		perror("send_pid");
	printf("%s","\033[1H\033[2J"); 
	printf (" --------------------------------------------\n");
	printf("|\33[33m ( *������)((�R��Q*)����ɹ�,������ת�û�ҳ��\33[0m|\n");
	printf (" --------------------------------------------\n");
	sleep(1);
	//Step4:��ת����ҳ�� 
	userMenu();
	
	return;
}

//ע��
void myDes(void)
{
	char buf[32],name[32];
	printf("������(�û���):");
	scanf("%s",name);
	if(send(curSockfd,name,32,0)<0)
		perror("send_name");
		
	if(recv(curSockfd,buf,32,0)<0)
		perror("recv_judge");
	
	if(strcmp(buf,"unexist")==0){
		printf("\t���û�������\n");
		sleep(1); 
		return;
	}else if(strcmp(buf,"on_line")==0){
		printf("\t���û����������豸��¼\n");
		sleep(1);
		return;
	}
	
	printf("������(����Ա����):");
	scanf("%s",buf);
	
	if(send(curSockfd,buf,32,0)<=0)
		perror("send_root");
		
	if(strcmp(buf,"root")!=0){
		printf("\t��������\n");
		sleep(1);
		return;
	} 
	
	if(recv(curSockfd,buf,32,0)<0)
		perror("recv_outcome");
	
	if(strcmp(buf,"ok")==0){
		printf("\t%s�û��ѱ��ɹ�����\n",name);
		sleep(1);
	}else{
		printf("\tδ֪����\n");
		sleep(1);
	}
	
	return;
}
 
//��Ӻ��� 
void makeFriends()
{
	char fname[32]="",buf[32],msg[32];
	
	printf("������(������):");
	scanf("%s",buf); 
	
	strcpy(fname,"n");
	strcat(fname,buf);
	
	if(send(curSockfd,fname,32,0)<0)
		perror("send_fidname");
	
	if(recv(curSockfd,buf,32,0)<0)
		perror("recv");
		
	if(strcmp(buf,"had")==0){
		printf("�����Ѿ��Ǻ�����!\n");
		return;
	}else if(strcmp(buf,"unexist")==0){
		printf("����û������!\n");
		return;	
	}else if(strcmp(buf,"same")==0){
		printf("�Լ����Լ����˸���ѣ�\n");
		return;	
	}
	
	printf("������(��Ӻ�������):");
	scanf("%s",buf);
	
	if(strlen(buf)>32){
		printf("\n\t\t������������!\n");
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
		
	printf("\n\t\t���ͳɹ�!\n");
	sleep(1);
	return;
}

//��֤ 
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
	printf("��ѡ��(��Ϣ���):");
	scanf("%s",buf);
	strcpy(msg,"c");
	strcat(msg,buf);
	if(send(curSockfd,msg,32,0)<0)
		perror("send_msg");
	
	if(recv(curSockfd,buf,32,0)<0)
		perror("recv_outcome");
	//printf("buf=%s\n",buf);
	if(strcmp(buf,"ok")==0){
		printf("\t��ӳɹ�\n");
	}else if(strcmp(buf,"error")==0){
		printf("\t�������\n");
	}else if(strcmp(buf,"unexist")==0){
		printf("\t���û���ע��\n");
	}else if(strcmp(buf,"had")==0){
		printf("\t�����Ѿ��Ǻ�����\n");
	}else{
		printf("\tδ֪����\n");
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

//���к��
void listPack()
{
	int i,len;
	int level;
	char buf[32];
	char name[32],bless[32],timeMsg[32];	
	printf("%s","\033[1H\033[2J"); 
	printf("\t\33[34m----------\33[0m\33[41m���Ⱥ\33[0m\33[34m----------\33[0m\n");
	if(recv(curSockfd,buf,32,0)<0)
		perror("recv_len");
	len = atoi(buf);
	if(len==0){
		printf("\33[0m\n\t\33[44m                          \33[0m\n");
		printf("\33[0m\t\33[44m                          \33[0m\n");
		printf("\33[0m\t\33[44m        \33[48m �տ���Ҳ \33[44m        \33[0m\n");
		printf("\33[0m\t\33[44m                          \33[0m\n");
		printf("\33[0m\t\33[44m                          \33[0m\n\n");
		sleep(2);
		return;
	}
	for(i=0;i<len;i++){
		//ʱ���ı� 
		if(recv(curSockfd,timeMsg,32,0)<0)
			perror("recv_time");
		//�ȼ�
		if(recv(curSockfd,buf,32,0)<0)
			perror("recv_level");
//���Ե�(60sһ��)
		level = atoi(buf)/180; 
		//����
		if(recv(curSockfd,name,32,0)<0)
			perror("recv_name");
		//ף�� 
		if(recv(curSockfd,bless,32,0)<0)
			perror("recv_bless");
		
		showPack(i,timeMsg,level,name,bless);
	}
	return;
}

//��ֵ 
void userPush()
{
	float sum;
	char buf[32],msg[32];
	printf("������(���):");
	scanf("%f",&sum);
	strcpy(msg,"f"); 
	if(sum<=0||sum>10000){
		printf("\t��������\n");
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
		printf("\t��ֵ�ɹ�\n");
	else
		printf("\t��ֵʧ��\n");
	sleep(1);
	return;
	
}

//�û�ת��
void userTran()
{
	int i,num;
	float sum,temp;
	char buf[32],msg[32],name[32];
	listFriends();
	printf("������(ת�˶���):");
	scanf("%s",name);
	
	strcpy(msg,"n");
	strcat(msg,name);
	if(send(curSockfd,msg,32,0)<0)
		perror("send");
	//
	if(recv(curSockfd,buf,32,0)<0)
		perror("recv");
		
	if(strcmp(buf,"error")==0){
		printf("\tû�иú���\n");
		sleep(1);
		return;
	}
	
	sum = atof(buf);
	
	printf("������(���):��");
	scanf("%s",buf);
	temp = atof(buf);
	sprintf(buf,"%.2f",temp);
	strcpy(msg,"f");
	if(temp>sum){
		printf("\t����\n");
		strcat(msg,"error");
		if(send(curSockfd,msg,32,0)<0)
			perror("send");
		sleep(1);
		return;
	}else if(temp<0){
		printf("\t�������\n");
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
		printf("\tת�˳ɹ�\n");
	else
		printf("\t������ע��\n");
		
	sleep(1);
	
	return;
}

void priChat()
{
	int i,j,k;
	int len,realLen,msgLen;//��Ϣ����
	int count; 
	int flag;
	int fsock;
	int state;
	char buf[32],name[32],msg[128],sendMsg[128];
	
	pthread_t id;
	
	//Step 1:���պ��� 
	listFriends();
	printf("������(�������):");
	scanf("%s",buf);
	
	//Step 2:��װ���� 
	strcpy(name,"n");
	strcat(name,buf);
	if(send(curSockfd,name,32,0)<0)
		perror("send");
	strcpy(name,buf);//���ִ���name�� 
	
	//Step 3:�����ж�
	if(recv(curSockfd,buf,32,0)<0)
		perror("recv_judge");
	
	if(strcmp(buf,"none")==0){
		printf("\t�������в��޴���\n");
		sleep(1);
		return;
	}
	
	//Step 4:������ʾ
	if(recv(curSockfd,buf,32,0)<0)
		perror("recv_state");
	state = atoi(buf);
	if(recv(curSockfd,buf,32,0)<0)
		perror("recv_fsock");
	fsock = atoi(buf);

	
	printf("%s","\033[1H\033[2J"); 
	printf(" __________________________________________\n");
	if(state == 0)
		printf("|\t\t%s \33[31m����\33[0m...\t\t   |\n",name);
	else
		printf("|\t\t%s \33[32m����\33[0m...\t\t   |\n",name);
	printf("|^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^|\n");
	
	if(recv(curSockfd,buf,32,0)<0)
		perror("recv_len");
	len = atoi(buf);
	
	if(!len)
		printf("\t��Ϣ��¼Ϊ��\n");
	else{
		for(i=0;i<len;i++){
			//��ȡ��Ϣ 
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
				j=1;//�������������
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
	
	//���������߳� 
	pthread_create(&id,NULL,(void *)showMsg,NULL); 
	
	printf(":");
	scanf("%s",msg);
	printf("[\33[33m���ͳɹ�]\33[0m\n");
	while(1){
		msgLen = strlen(msg);
		
		if(strcmp(msg,"exit")==0){
			pthread_cancel(id);
			if(send(curSockfd,"mexit",8,0)<0)
				perror("send_exit");
			//printf("\t׼���˳�\n");
			return;
		} 
		
		if(msgLen>99)
			printf("\t������������\n");
		else{
			strcpy(sendMsg,"m");
			strcat(sendMsg,msg);
			if(send(curSockfd,sendMsg,128,0)<0)
				perror("send_msg");
		}
		printf("\n:");
		scanf("%s",msg);
		printf("[\33[33m���ͳɹ�]\33[0m\n");
	}
	pthread_cancel(id);
	return;
}

//Ⱥ�� 
void groChat(void)
{
	int len;
	char msg[256],sendMsg[256];
	pthread_t id;
	printf("%s","\033[1H\033[2J"); 
	printf("\t\33[34m----------\33[32m����������\33[34m----------\33[0m\n");
	pthread_create(&id,NULL,(void *)showGro,NULL);
	
	
	printf("\33[s");//����λ��
	printf("\33[19;1H");//�µ��ײ�
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
			printf("\t������������\n");
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

//����� 
void sendPack()
{
	int num;
	float sum;
	char buf[32],msg[32];
	//������
	while(1){
		printf("\33[0m������(���):\33[32m");
		scanf("%f",&sum);
		if(sum>0)
			break;
		printf("\33[0m\t\t\33[41m�������\33[0m\n\n");
	}
	
	sprintf(buf,"%.2f",sum);
	strcpy(msg,"f");
	strcat(msg,buf);
	if(send(curSockfd,msg,32,0)<0)
		perror("send_sum");
	if(recv(curSockfd,buf,32,0)<0)
		perror("recv");
	if(strcmp(buf,"error")==0){
		printf("\33[0m\t\t\33[45m����\33[0m\n\n");
		sleep(1);
		return;
	}
	//�������
	while(1){
		printf("\33[0m������(����):\33[32m");
		scanf("%s",buf);
		num = atoi(buf);
		if(num<=0){
			printf("\33[0m\t\t\33[41m��������\33[0m\n\n");
			continue; 
		}
		if(sum/num>0.009999)
			break;
		printf("\33[0m\t\t\33[45mǮ������\33[0m\n\n");
	}
	strcpy(msg,"i");
	sprintf(buf,"%d",num);
	strcat(msg,buf);
	if(send(curSockfd,msg,32,0)<0)
		perror("send_num");
	printf("\33[0m\t\33[46mTips:���������ֳ�%d��\33[0m\n\n",num);
	//����ף�� 
	while(1){
		memset(buf,0,sizeof(buf));
		printf("������(ף��):");
		scanf("%s",buf);
		if(testLen(buf)<14)
			break;
		printf("\33[0m\t\t\33[45m��������\33[0m\n\n");
	}
	strcpy(msg,"m");
	strcat(msg,buf);
	if(send(curSockfd,msg,32,0)<0)
		perror("send_msg");
	
	if(recv(curSockfd,buf,32,0)<0)
		perror("recv_out");
	if(strcmp(buf,"ok")==0)
		printf("\33[0m\t\t\33[42m���ͳɹ�\33[0m\n\n");
	else
		printf("\33[0m\t\t\33[41m���糬ʱ\33[0m\n\n");
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
		printf("����(������):");
		scanf("%s",buf);
		index = atoi(buf);
		if(index>=0&&index<len)
			break;
		printf("\t\33[41m��������\33[0m\n");
		
	}
	strcpy(msg,"i");
	strcat(msg,buf);
	if(send(curSockfd,msg,32,0)<0)
		perror("send_index");
	
	if(recv(curSockfd,buf,32,0)<0)
		perror("recv_out");
	printf("\n      ");
	if(strcmp(buf,"over")==0)
		printf("\t\33[41m�����ʧЧ\33[0m\n");
	else if(strcmp(buf,"had")==0)
		printf("\t\33[41m���Ĳ���������\33[0m\n");
	else if(strcmp(buf,"end")==0)
		printf("\t\33[46mû�����Ⱥ���\33[0m\n");
	else
		printf("\t\33[41m��ϲ����%sԪ\33[0m\n",buf);
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

//ǿ���޸�״̬ 
void setState()
{
	char name[32];
	printf("��Ҫǿ�����ߵ��û�:");
	scanf("%s",name);
	
	if(send(curSockfd,name,32,0)<0)
		perror("send");
	return;
}

//��������б�
void listFriends()
{
	int i=1,num;
	char buf[32],fidname[32];

	//Step1:���պ�����
	if(recv(curSockfd,buf,32,0)<0)
		perror("recv_num");
	num = atoi(buf); 
	if(num==0){
		printf("#system msg����һ�޿��G��\n");
		return;
	}
	//printf("num=%d\n",num);
	//Step2��while(num--)	recv(fidname);	printf(difname);
	printf("   ---\33[32m����-�б�\33[0m---\n");
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
			printf("\t\t\33[33m��\33[32m����Ļ����\33[33m)\33[0m\n");
			printf("\t\33[34m----------\33[32m����������\33[34m----------\33[0m\n");
			count=0;
		}
		printf("\33[s");
		printf("\33[19;1H\n");//�µ��ײ�
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
		
		//�����߿� 
		if(i==0){
			if(index<10)
				printf("%s%s0%d%s%s",bg[0],ft[6],index,re,bg[1]);
			else
				printf("%s%s%d%s%s",bg[0],ft[6],index,re,bg[1]);
		}else{
			printf("%s  %s",bg[0],bg[1]);
		}
		
		//�������
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
		
		//����Ҳ�
		printf("%s  %s\n",bg[0],re);
	}

}
