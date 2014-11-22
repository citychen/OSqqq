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

pthread_mutex_t mutex;  //保護青蛙位置的鎖
pthread_mutex_t mutex2;  //保護木頭位置的鎖
pthread_mutex_t mutex3;
pthread_cond_t threshold_cv;

int frog_x=18;  //青蛙的X座標
int frog_y=19;  //青蛙的Y座標
int frog_v;  //青蛙的向量
int mood_x[20][32];   //木頭的X座標  [第幾個木頭][第幾木頭上的位置分布]
int mood_y[8];    //木頭的y座標 [第幾個木頭]
int mood_v[8];    //木頭的速度 [第幾個木頭]
int game = 1;   //遊戲繼續
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
  win = newwin(5, 30, LINES / 2 - 3, COLS / 2 - 15);/* 建立一個新視窗, 其中LINES,COLS */
  box(win, '|', '-');                     /* 為 curses 內定值, 即螢幕行/列數*/
  
  game = 0;
  if (c==0)   //你輸了 結束
  {
	mvwaddstr(win, 1, 2, "it is a simple game ");
	mvwaddstr(win, 2, 2, "and you can not complete it");
	mvwaddstr(win, 3, 2, "you are a LOSER !!!!!!!");
  }
  else if (c == 2) //你要求離開  結束
  {
	mvwaddstr(win, 1, 2, "what !!! you want to leave it ? ");
	mvwaddstr(win, 2, 2, "fine you go !  BYE BYE");
	mvwaddstr(win, 3, 2, "fuck you !!!!!!!");
  }
  else               //c==1  你贏了  結束
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
  win1 = newwin(5, 50, 5, 5);/* 建立一個新視窗, 其中LINES,COLS */
  box(win1, '|', '-');                     /* 為 curses 內定值, 即螢幕行/列數*/
  mvwaddstr(win1, 1, 2, "W is up , S is down");
  mvwaddstr(win1, 2, 2, "A is left , D is right");
  mvwaddstr(win1, 3, 2, "Q is exit , then input ant key"); 
  touchwin(win1);        /* wrefresh() 前需 touchwin() */
  wrefresh(win1);
  getch();              /* 按任意鍵關閉視窗 */
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



//青蛙偵測木頭上的移動(記錄在木頭上) 及 以及 邊界 以及 是否得勝 
void* frog_act(void* t)
{
	while (game)
	{	pthread_mutex_lock(&mutex); //上鎖 保護青蛙座標
	    mvprintw(21, 10, "forg_x = %d frog_y=%d", frog_x, frog_y); //印出現在青蛙座標
	    refresh();

		if( (frog_x < 4) || (frog_x>35))  //青蛙超過左右邊界死掉
		{
		  mvprintw(8, 8, " fail1");
		  refresh();
		  gameover(0);
		}
		if (frog_y == 9)      //到達終點
		{ gameover(1);
		}
		pthread_mutex_lock(&mutex2); //上鎖 保護木頭座標
		for (int i = 0;i<8;i++) //偵測frog到哪一行木頭了
		{			
			if (frog_y == mood_y[i])//偵測frog是否到那一行木頭了
			{	int have = 0;
				for (int k = 0; k<32; k++)  //偵測是否已經站在上面了
				{
					if (mood_x[i][k] == 2)
					{
						have = k;  //站在哪一格木頭上
					}
				}
				if (!have) //還沒站在木頭上
				{   //偵測是否成功站到木頭上
					if ((mood_x[i][frog_x - 4]) == 1) //成功站上去
					{ //登錄在木頭上了
						mood_x[i][frog_x - 4] = 2;
					}
					else if (mood_x[i][frog_x - 4] == 0)
					{ //失敗跳到河去了
						mvprintw(8, 8, " fail1");
						refresh();
						gameover(0);
					}
				}
			    else //已經站在木頭上
				{ if ( (frog_x-(have+4)) < 0) //青蛙想在木頭上往左移動
				  { if (mood_x[i][have-1]==0)
				    {
					  mvprintw(8, 18, " fail2");
					  refresh();
					  gameover(0);  //失敗 跳到河去了
				    }
				    else if (mood_x[i][have - 1] == 1) //青蛙在木頭上往左移動成功
					{  mood_x[i][have - 1] = 2;
				       mood_x[i][have] = 1;
				    }
					else  //出錯
					{	mvprintw(22, 8, " fail");
				    	refresh();
					}
				  }
			  	  else if ((frog_x - (have + 4)) > 0) //青蛙在木頭上往右移動
				  { if (mood_x[i][have + 1] == 0)
				    {
					   mvprintw(8, 8, " fail3");
					   refresh();
					   gameover(0);  //失敗 跳到河去了
					}
					else if (mood_x[i][have + 1] == 1) //青蛙在木頭上往右移動成功
					{
						mood_x[i][have + 1] = 2;
						mood_x[i][have] = 1;
					 }
					else  //出錯
					{
					  mvprintw(8, 8, " fail4");
					  refresh();
					}
				  }
				  else  //青蛙跟著木頭動
				  { }
				  
				}
				
			}
		}	
		pthread_mutex_unlock(&mutex2); //解鎖
		pthread_mutex_unlock(&mutex); //解鎖
		usleep(50000);
	}
	
	pthread_exit(NULL);
}

//木頭移動 偵測青蛙是否來了或者走了
void* mood_act(void* t)  //t 為 第幾號木頭的編號
{  //初始化變數
	int y = (long)t;  //傳入第幾號木頭
	mood_y[y] = y+11;  //第幾號木頭轉換成木頭y座標
	int i = 0;         //迴圈用變數  無意義
	int choice;        //選擇哪一個初始化
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

	//初始化木頭狀態 用choice 選擇初始化的狀態
	for (i = 0; i < 32; i++) //判別 i 格 有沒有木頭 
	{
		if ( ( ( ( (i+choice)%32) /4 ) % 2 ) == 0)
		{ mood_x[y][i] = 1;  //有木頭
		}
		else
		{ mood_x[y][i] = 0;  //無木頭
		}
	}
	
	while (game)
	{   pthread_mutex_lock(&mutex); //上鎖保護青蛙
		pthread_mutex_lock(&mutex2); //上鎖保護木頭
	    
		//變動木頭位置
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

		//印出木頭
		for (i = 0; i < 32; i++)
		{   //判別 i 格 有沒有木頭 
			if (mood_x[y][i] == 1)//有
			{ mvprintw(mood_y[y], i + 4, "-");
			  refresh();
			}
			else if (mood_x[y][i] == 0) //無
			{ mvprintw(mood_y[y], i + 4, " ");
			  refresh();
			}
			else  if (mood_x[y][i] == 2)  //青蛙在木頭上
			{
				
				//青蛙走了沒
				if (frog_y == mood_y[y])  //還沒走
				{ mvprintw(mood_y[y], i + 4, "0");  //劃出青蛙
				  frog_x=frog_x+mood_v[y];               //青蛙跟著木頭走
				  refresh();
				}
				else
				{
					mvprintw(mood_y[y], i + 4, "-"); //印回木頭
				    refresh();
					mood_x[y][i] = 1;   //青蛙走了
				}
				
			}
			else
			{ }
		}
    
		pthread_mutex_unlock(&mutex2);//解鎖木頭
		pthread_mutex_unlock(&mutex);//解鎖青蛙 
		usleep(500000);
	}
	pthread_exit(NULL);
}
//偵測按鈕以及顯示青蛙在起終點的移動
void* control(void* t)
{
	while (1)
	{
		if (kbhit())
		{
			switch (getchar())
			{
				pthread_mutex_lock(&mutex);  //上鎖 保護青蛙座標
				 
			case 'w':    /* 判斷是否"↑"鍵被按下 */
			case 'W':  if (frog_y == 19) //如果青蛙在起點往上跳 要把原來站的地方還原
					   {
						mvprintw(frog_y , frog_x, "|");
					   }
					   mvprintw(--frog_y, frog_x, "0"); //印青蛙
					   refresh();
					   break;
			case 's':  /* 判斷是否"↓"鍵被按下 */
			case 'S': if (frog_y == 19)
					  {  //已經到最下面了 禁止往下
					  }
					  else
					  {
						  if (frog_y == 10)  //青蛙到最上層要往下  要把原來站的地方還原					  {
						  {
							  mvprintw(frog_y, frog_x, "|");
						  }
						  mvprintw(++frog_y, frog_x, "0"); //印青蛙
					  }
					  
			          refresh();
					  break;
			case 'd':
			case 'D': if ((frog_y == 19) || (frog_y == 10))  //在起點和終點的印青蛙
					  {
						  mvprintw(frog_y, frog_x++, "|0"); //印青蛙
					  }
					  else
				      {
					      mvprintw(frog_y, frog_x++, "0"); //印青蛙
				      }
				      refresh();
				      break;   /* 判斷是否"→"鍵被按下 */
			case 'a':
			case 'A': if ((frog_y == 19) || (frog_y == 10))  //在起點和終點的印青蛙
	                  {
		               mvprintw(frog_y, --frog_x, "0|"); //印青蛙
	                  }   /* 判斷是否"←"鍵被按下 */
				      else
				      {
					   mvprintw(frog_y, frog_x--, "0"); //印青蛙
				      }
				       refresh();
				       break;
	        case 'q':                  /* 判斷是否 q 鍵被按下 */
	        case 'Q':
		             gameover(2);
		             
	        case '\t': Explanation();               /* 判斷是否 TAB 鍵被按下 說明跳出*/

		             break;
	        default:         /* 如果不是特殊字元*/
		             break;
			}

		}
		   
			pthread_mutex_unlock(&mutex);  //解鎖
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

  initial();  //初始化

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