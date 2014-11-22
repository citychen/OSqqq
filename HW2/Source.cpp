//#include <cstdlib>
#include <stdlib.h>
//#include <cstring.h>
#include <stdio.h>
//#include <string>
#include <ncurses.h>
#include <unistd.h>
#include <pthread.h>
#include <termios.h> 
#include <fcntl.h>

pthread_mutex_t mutex;  //�O�@�C���m����
pthread_mutex_t mutex2;  //�O�@���Y��m����
pthread_mutex_t mutex3;
pthread_cond_t threshold_cv;

int frog_x=18;  //�C�쪺X�y��
int frog_y=19;  //�C�쪺Y�y��
int frog_v;  //�C�쪺�V�q
int mood_x[20][32];   //���Y��X�y��  [�ĴX�Ӥ��Y][�ĴX���Y�W����m����]
int mood_y[8];    //���Y��y�y�� [�ĴX�Ӥ��Y]
int mood_v[8];    //���Y���t�� [�ĴX�Ӥ��Y]
int game = 1;   //�C���~��
int kbhit(void)
{
	struct termios oldt, newt;
	int ch;
	int oldf;
	tcgetattr(STDIN_FILENO, &oldt);
	newt = oldt;
	newt.c_lflag &= ~(ICANON | ECHO);
	tcsetattr(STDIN_FILENO, TCSANOW, &newt);
	oldf = fcntl(STDIN_FILENO, F_GETFL, 0);
	fcntl(STDIN_FILENO, F_SETFL, oldf | O_NONBLOCK);
	ch = getchar();
	tcsetattr(STDIN_FILENO, TCSANOW, &oldt);
	fcntl(STDIN_FILENO, F_SETFL, oldf);
	if (ch != EOF)
	{
		ungetc(ch, stdin);
		return 1;
	}
	return 0;
}


void gameover(int c)
{
  WINDOW *win;
  win = newwin(5, 30, LINES / 2 - 3, COLS / 2 - 15);/* �إߤ@�ӷs����, �䤤LINES,COLS */
  box(win, '|', '-');                     /* �� curses ���w��, �Y�ù���/�C��*/
  
  game = 0;
  if (c==0)   //�A��F ����
  {
	mvwaddstr(win, 1, 2, "it is a simple game ");
	mvwaddstr(win, 2, 2, "and you can not complete it");
	mvwaddstr(win, 3, 2, "you are a LOSER !!!!!!!");
  }
  else if (c == 2) //�A�n�D���}  ����
  {
	mvwaddstr(win, 1, 2, "what !!! you want to leave it ? ");
	mvwaddstr(win, 2, 2, "fine you go !  BYE BYE");
	mvwaddstr(win, 3, 2, "fuck you !!!!!!!");
  }
  else               //c==1  �AĹ�F  ����
  {
	mvwaddstr(win, 1, 2, "you win ! ");
    mvwaddstr(win, 2, 2, "you win !you win !");
    mvwaddstr(win, 3, 2, "you are a WINER !!!!!!!"); 
  }
  touchwin(win);
  wrefresh(win);
  sleep(3);
  endwin();
  exit(0);
}

void Explanation()
{
  WINDOW *win1;
  win1 = newwin(5, 50, 5, 5);/* �إߤ@�ӷs����, �䤤LINES,COLS */
  box(win1, '|', '-');                     /* �� curses ���w��, �Y�ù���/�C��*/
  mvwaddstr(win1, 1, 2, "W is up , S is down");
  mvwaddstr(win1, 2, 2, "A is left , D is right");
  mvwaddstr(win1, 3, 2, "Q is exit , then input ant key"); 
  touchwin(win1);        /* wrefresh() �e�� touchwin() */
  wrefresh(win1);
  getch();              /* �����N���������� */
  touchwin(stdscr);

}

void background()
{ int x, y=10;
  for (x = 4; x < 36; x++)
  {
	 mvprintw(y,x,"|");
  }
  mvprintw(y,x," END");
  refresh();
   
	 y = 19;
  for (x = 4; x < 36; x++)
  { if (x!=18)
	  mvprintw(y, x, "|");
    else
	  mvprintw(y, x, "0");
  }
  mvprintw(y, x, " START");
  refresh();
}

void initial()
{
	initscr();
	cbreak();
	nonl();
	noecho();
	intrflush(stdscr,FALSE);
	keypad(stdscr,TRUE);
	refresh();

	background();
}



//�C�찻�����Y�W������(�O���b���Y�W) �� �H�� ��� �H�� �O�_�o�� 
void* frog_act(void* t)
{
	while (game)
	{	pthread_mutex_lock(&mutex); //�W�� �O�@�C��y��
	    mvprintw(21, 10, "forg_x = %d frog_y=%d", frog_x, frog_y); //�L�X�{�b�C��y��
	    refresh();

		if( (frog_x < 4) || (frog_x>35))  //�C��W�L���k��ɦ���
		{
		  mvprintw(8, 8, " fail1");
		  refresh();
		  gameover(0);
		}
		if (frog_y == 9)      //��F���I
		{ gameover(1);
		}
		pthread_mutex_lock(&mutex2); //�W�� �O�@���Y�y��
		for (int i = 0;i<8;i++) //����frog����@����Y�F
		{			
			if (frog_y == mood_y[i])//����frog�O�_�쨺�@����Y�F
			{	int have = 0;
				for (int k = 0; k<32; k++)  //�����O�_�w�g���b�W���F
				{
					if (mood_x[i][k] == 2)
					{
						have = k;  //���b���@����Y�W
					}
				}
				if (!have) //�٨S���b���Y�W
				{   //�����O�_���\������Y�W
					if ((mood_x[i][frog_x - 4]) == 1) //���\���W�h
					{ //�n���b���Y�W�F
						mood_x[i][frog_x - 4] = 2;
					}
					else if (mood_x[i][frog_x - 4] == 0)
					{ //���Ѹ���e�h�F
						mvprintw(8, 8, " fail1");
						refresh();
						gameover(0);
					}
				}
			    else //�w�g���b���Y�W
				{ if ( (frog_x-(have+4)) < 0) //�C��Q�b���Y�W��������
				  { if (mood_x[i][have-1]==0)
				    {
					  mvprintw(8, 18, " fail2");
					  refresh();
					  gameover(0);  //���� ����e�h�F
				    }
				    else if (mood_x[i][have - 1] == 1) //�C��b���Y�W�������ʦ��\
					{  mood_x[i][have - 1] = 2;
				       mood_x[i][have] = 1;
				    }
					else  //�X��
					{	mvprintw(22, 8, " fail");
				    	refresh();
					}
				  }
			  	  else if ((frog_x - (have + 4)) > 0) //�C��b���Y�W���k����
				  { if (mood_x[i][have + 1] == 0)
				    {
					   mvprintw(8, 8, " fail3");
					   refresh();
					   gameover(0);  //���� ����e�h�F
					}
					else if (mood_x[i][have + 1] == 1) //�C��b���Y�W���k���ʦ��\
					{
						mood_x[i][have + 1] = 2;
						mood_x[i][have] = 1;
					 }
					else  //�X��
					{
					  mvprintw(8, 8, " fail4");
					  refresh();
					}
				  }
				  else  //�C���ۤ��Y��
				  { }
				  
				}
				
			}
		}	
		pthread_mutex_unlock(&mutex2); //����
		pthread_mutex_unlock(&mutex); //����
		usleep(50000);
	}
	
	pthread_exit(NULL);
}

//���Y���� �����C��O�_�ӤF�Ϊ̨��F
void* mood_act(void* t)  //t �� �ĴX�����Y���s��
{  //��l���ܼ�
	int y = (long)t;  //�ǤJ�ĴX�����Y
	mood_y[y] = y+11;  //�ĴX�����Y�ഫ�����Yy�y��
	int i = 0;         //�j����ܼ�  �L�N�q
	int choice;        //��ܭ��@�Ӫ�l��
	switch ((y%3))
	{
	  case 0:choice = 0;
		     mood_v[y]=1;
		     break;
	  case 1:choice = 6;
		     mood_v[y] = -1;
		     break;
	  case 2:choice = 3;
		     mood_v[y] = 2;
		     break;
	  default:break;
	}

	//��l�Ƥ��Y���A ��choice ��ܪ�l�ƪ����A
	for (i = 0; i < 32; i++) //�P�O i �� ���S�����Y 
	{
		if ( ( ( ( (i+choice)%32) /4 ) % 2 ) == 0)
		{ mood_x[y][i] = 1;  //�����Y
		}
		else
		{ mood_x[y][i] = 0;  //�L���Y
		}
	}
	
	while (game)
	{   pthread_mutex_lock(&mutex); //�W��O�@�C��
		pthread_mutex_lock(&mutex2); //�W��O�@���Y
	    
		//�ܰʤ��Y��m
		int temp[32];
		for (i = 0; i < 32; i++)
		{
			temp[i] = mood_x[y][i];
		}
		for (i = 0; i < 32; i++)
		{
			if (mood_v[y] == -1)
				mood_x[y][(i + mood_v[y] + 32) % 32] = temp[i];
			else
				mood_x[y][(i + mood_v[y]) % 32] = temp[i];
		}

		//�L�X���Y
		for (i = 0; i < 32; i++)
		{   //�P�O i �� ���S�����Y 
			if (mood_x[y][i] == 1)//��
			{ mvprintw(mood_y[y], i + 4, "-");
			  refresh();
			}
			else if (mood_x[y][i] == 0) //�L
			{ mvprintw(mood_y[y], i + 4, " ");
			  refresh();
			}
			else  if (mood_x[y][i] == 2)  //�C��b���Y�W
			{
				
				//�C�쨫�F�S
				if (frog_y == mood_y[y])  //�٨S��
				{ mvprintw(mood_y[y], i + 4, "0");  //���X�C��
				  frog_x=frog_x+mood_v[y];               //�C���ۤ��Y��
				  refresh();
				}
				else
				{
					mvprintw(mood_y[y], i + 4, "-"); //�L�^���Y
				    refresh();
					mood_x[y][i] = 1;   //�C�쨫�F
				}
				
			}
			else
			{ }
		}
    
		pthread_mutex_unlock(&mutex2);//������Y
		pthread_mutex_unlock(&mutex);//����C�� 
		usleep(500000);
	}
	pthread_exit(NULL);
}
//�������s�H����ܫC��b�_���I������
void* control(void* t)
{
	while (1)
	{
		if (kbhit())
		{
			switch (getchar())
			{
				pthread_mutex_lock(&mutex);  //�W�� �O�@�C��y��
				 
			case 'w':    /* �P�_�O�_"��"��Q���U */
			case 'W':  if (frog_y == 19) //�p�G�C��b�_�I���W�� �n���ӯ����a���٭�
					   {
						mvprintw(frog_y , frog_x, "|");
					   }
					   mvprintw(--frog_y, frog_x, "0"); //�L�C��
					   refresh();
					   break;
			case 's':  /* �P�_�O�_"��"��Q���U */
			case 'S': if (frog_y == 19)
					  {  //�w�g��̤U���F �T��U
					  }
					  else
					  {
						  if (frog_y == 10)  //�C���̤W�h�n���U  �n���ӯ����a���٭�					  {
						  {
							  mvprintw(frog_y, frog_x, "|");
						  }
						  mvprintw(++frog_y, frog_x, "0"); //�L�C��
					  }
					  
			          refresh();
					  break;
			case 'd':
			case 'D': if ((frog_y == 19) || (frog_y == 10))  //�b�_�I�M���I���L�C��
					  {
						  mvprintw(frog_y, frog_x++, "|0"); //�L�C��
					  }
					  else
				      {
					      mvprintw(frog_y, frog_x++, "0"); //�L�C��
				      }
				      refresh();
				      break;   /* �P�_�O�_"��"��Q���U */
			case 'a':
			case 'A': if ((frog_y == 19) || (frog_y == 10))  //�b�_�I�M���I���L�C��
	                  {
		               mvprintw(frog_y, --frog_x, "0|"); //�L�C��
	                  }   /* �P�_�O�_"��"��Q���U */
				      else
				      {
					   mvprintw(frog_y, frog_x--, "0"); //�L�C��
				      }
				       refresh();
				       break;
	        case 'q':                  /* �P�_�O�_ q ��Q���U */
	        case 'Q':
		             gameover(2);
		             
	        case '\t': Explanation();               /* �P�_�O�_ TAB ��Q���U �������X*/

		             break;
	        default:         /* �p�G���O�S��r��*/
		             break;
			}

		}
		   
			pthread_mutex_unlock(&mutex);  //����
			usleep(1000);
		
	}
	pthread_exit(NULL);

}

int main()
{ int i, rc;
  int t1 = 18, t2 = 2, t3 = 3;
  pthread_t threads[10]; 
  pthread_attr_t attr;
   
  /* Initialize mutex and condition variable objects */
  pthread_mutex_init(&mutex, NULL);
  pthread_mutex_init(&mutex2, NULL);
  pthread_mutex_init(&mutex3, NULL);
  pthread_cond_init(&threshold_cv, NULL);

  initial();  //��l��

  /* For portability, explicitly create threads in a joinable state */
  pthread_attr_init(&attr);
  pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_JOINABLE);
  pthread_create(&threads[0], &attr, mood_act, (void *)0);
  pthread_create(&threads[1], &attr, mood_act, (void *)1);
  pthread_create(&threads[2], &attr, mood_act, (void *)2);
  pthread_create(&threads[3], &attr, mood_act, (void *)3);
  pthread_create(&threads[4], &attr, mood_act, (void *)4);
  pthread_create(&threads[5], &attr, mood_act, (void *)5);
  pthread_create(&threads[6], &attr, mood_act, (void *)6);
  pthread_create(&threads[7], &attr, mood_act, (void *)7);
  pthread_create(&threads[8], &attr, frog_act, (void *)8);
  pthread_create(&threads[9], &attr, control, (void *)9);


  for (i = 0; i < 10; i++) {
	  pthread_join(threads[i], NULL);
  }
  
  /* Clean up and exit */
  pthread_attr_destroy(&attr);
  pthread_mutex_destroy(&mutex);
  pthread_cond_destroy(&threshold_cv);
  exit(0);


}