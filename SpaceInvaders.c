#include "tm4c123gh6pm.h"
#include "Nokia5110.h"
#include "Random.h"
#include "TExaS.h"

void DisableInterrupts(void); // Disable interrupts
void EnableInterrupts(void);  // Enable interrupts
void Timer2_Init(unsigned long period);
void Delay100ms(unsigned long count); // time delay in 0.1 seconds
char WinTest(void); 
unsigned long TimerCount;
unsigned long Semaphore;
short WinDiagRight(void);
short WinDiagLeft(void);
short WinRow(short r);
short WinCol(short c);
void printScore(void);
void printRound(void);
short isTie(void);
short Position[3][3];
short Winner=0;
short position,stepI,stepJ,player,Finished,xScore=0,oScore=0,Round=1;


	void PortF_Init(void){ volatile unsigned long delay;
  SYSCTL_RCGC2_R |= 0x00000020;     // 1) F clock
  delay = SYSCTL_RCGC2_R;           // delay   
  GPIO_PORTF_LOCK_R = 0x4C4F434B;   // 2) unlock PortF PF0  
  GPIO_PORTF_CR_R = 0x1F;           // allow changes to PF4-0       
  GPIO_PORTF_AMSEL_R = 0x00;        // 3) disable analog function
  GPIO_PORTF_PCTL_R = 0x00000000;   // 4) GPIO clear bit PCTL  
  GPIO_PORTF_DIR_R = 0x0E;          // 5) PF4,PF0 input, PF3,PF2,PF1 output   
  GPIO_PORTF_AFSEL_R = 0x00;        // 6) no alternate function
  GPIO_PORTF_PUR_R = 0x11;          // enable pullup resistors on PF4,PF0       
  GPIO_PORTF_DEN_R = 0x1F;          // 7) enable digital pins PF4-PF0        
}
void PORTE_INIT(void){
	volatile unsigned long delay;
	SYSCTL_RCGC2_R |= 0x00000010;     // 1) F clock
  delay = SYSCTL_RCGC2_R;           // delay   
  GPIO_PORTE_LOCK_R = 0x4C4F434B;   // 2) unlock PortE
  GPIO_PORTE_CR_R = 0x03;           // allow changes to PE2       
  GPIO_PORTE_AMSEL_R = 0x00;        // 3) disable analog function
  GPIO_PORTE_PCTL_R = 0x00000000;   // 4) GPIO clear bit PCTL  
  GPIO_PORTE_DIR_R = 0x02;          // 5) PE0 input, PE1 output   
  GPIO_PORTE_AFSEL_R = 0x00;        // 6) no alternate function
  GPIO_PORTE_PDR_R= 0x01;          // enable pulldown resistors on PE0       
  GPIO_PORTE_DEN_R = 0x01;          // 7) enable digital pins PE0-PE1
}
//intialize position array player variable,steps and finished
void Initialization(){
	
	 short i,j;
	 position=0; 
	 Finished=1;
	 stepI=4;
	 stepJ=2;
	 player=0; // 0 is X , and else is O
	 for(i=0;i<3;i++){
		 for(j=0;j<3;j++)
		 Position[i][j]=0;
	 }
}
//draw the screen
void Draw(){
	short i,j,x,y;
	x=84/3;
	y=48/3;
	
	for(i=0;i<3;i++){
		for(j=0;j<48;j++)Nokia5110_SetPixel(x*i,j);
	  for(j=0;j<84;j++)Nokia5110_SetPixel(j,y*i);
	}
	
	 Nokia5110_SetCursor(3,5);
	 Nokia5110_OutString("X Turn ");
	
}
//draw the screen of the game
 void set_Cursor() {
	 char positionX,positionY;
	 if( Position[position/3][position%3]==0){
		        positionX=((position%3)*stepI);
				    positionY=((position/3)*stepJ);
						Position[position/3][position%3]='_';
						Nokia5110_SetCursor(positionX,positionY);
			  	  Nokia5110_OutChar('|');
		        Nokia5110_SetCursor(positionX,positionY);
						}
		 
	 }
	 void remove_Cursor(){
		 char positionX,positionY;
		 if(Position[position/3][position%3]=='_'){
			    positionX=((position%3)*stepI);
				  positionY=((position/3)*stepJ);
					Position[position/3][position%3]=0;
					Nokia5110_SetCursor(positionX,positionY);
			  	Nokia5110_OutChar(' ');		
				}
		 
	 }

int main(void){
	
	 unsigned long SW1,SW2,SW3;  // to set switches to SW1,SW2,SW3
	
	 TExaS_Init(SSI0_Real_Nokia5110_Scope);  // set system clock to 80 MHz
	 Random_Init(1);  
 	Nokia5110_Init();
	Nokia5110_ClearBuffer(); //clear buffer
	Nokia5110_DisplayBuffer();      // draw buffer
	Nokia5110_Clear(); //Intro Screen
	Nokia5110_SetCursor( 2 , 0 );
	Nokia5110_OutString("T  i  c!");
	Nokia5110_SetCursor( 2, 1 );
	Nokia5110_OutString("T  a  c!");
	Nokia5110_SetCursor( 2 , 2 );
	Nokia5110_OutString("T  o  e!");
	Nokia5110_SetCursor(5, 3);
	Nokia5110_OutString("BY");
	Nokia5110_SetCursor(0, 4);
	Nokia5110_OutString("Hassan Essam");
	Delay100ms(5);
	printRound();
	Nokia5110_ClearBuffer();
	 PortF_Init();
	 PORTE_INIT();
	begin: //when a round ends,start over from here
	Nokia5110_DisplayBuffer();
	
	 //Delay100ms(2);
	 Nokia5110_Clear(); //clear the screen to start drawing the actual game board
	
	 Initialization(); //start intilizing
	 Nokia5110_Clear(); 
	 Delay100ms(1);
	 Draw();//start drawing the board
	 set_Cursor();
	 Nokia5110_SetCursor(0,0);

while(1){
		 SW1 = GPIO_PORTF_DATA_R&0x10;     // read PF4 into SW1
     SW2 = GPIO_PORTF_DATA_R&0x01;     // read PF0 into SW2
		 SW3 = GPIO_PORTE_DATA_R&0x01;     // read PE0 into SW3
		
	
			
			if(!(SW1)){ //If Switch one is pressed (Active Low) 
				
        remove_Cursor();				
				
				position++; //move forward
				if(position>8)position=8; //if out of bounds,don't move 
    	   while(!(GPIO_PORTF_DATA_R&0x10)); //while it is still pressed,don't do anything
				
				  set_Cursor();

			}
				
			if(!(SW2)) //if switch two is pressed (Active Low)
			{
				remove_Cursor();
				
				position--; //move bacwards
				if(position<0)position=0; //if out of bounds,don't move
				while(!(GPIO_PORTF_DATA_R&0x01)); //while it is pressed,don't do anything
				  set_Cursor();
				

			}
			if((SW3)){ //if switch 3 is pressed
				while(GPIO_PORTE_DATA_R&0x01);
				if(!(player)){ //if player X
					if(Position[position/3][position%3]=='_') //if current position in array is yet empty
					{
				  	Nokia5110_OutChar('X'); // print on screen X
				  	Position[position/3][position%3]='X'; //put in position array X (to indicate that there is already something in that place on screen)
						// increment cursor
						position++; //move forward (Pointer)
					  if(position>8)position=8; //if out of bounds,don't move
						Nokia5110_SetCursor(3,5);
						Nokia5110_OutString("O Turn"); //print that this is O player turn
						set_Cursor();
						Winner=WinTest(); //Check if player is winner
						player^=1; //XOR ..> to move control from player one to player two
					}
					
				}
					
				else{
					if(Position[position/3][position%3]=='_') //if current position in array is yet empty
						{
							Nokia5110_OutChar('O');
							Position[position/3][position%3]='O'; //put in position array O (to indicate that there is already something in that place on screen)
							// increment cursor
							position++;//move forward (Pointer)
							if(position>8)position=8;//if out of bounds,don't move

							
							Nokia5110_SetCursor(3,5);						
							Nokia5110_OutString("X Turn");//print that this is x player turn
							set_Cursor();
							
						  Winner=WinTest();	 //Check if player is winner	
							player^=1;//XOR ..> to move control from player one to player two
					}
				}		
			}
			//check for winner
			if(Winner){ 
				if(Winner=='X') {
					if(Finished){ //if play is not done.
					Nokia5110_Clear();
					Nokia5110_SetCursor(0,2);
					Nokia5110_OutString("player X Win");
						xScore++;//counter of player X score
						Delay100ms(2);
						Finished=0; //indicate that play is done
						printScore(); //print the score
						goto begin; //start over
					}
				}
				if(Winner=='O') {
					if(Finished){
					Nokia5110_Clear();
					Nokia5110_SetCursor(0,2);
					Nokia5110_OutString("player O Win");
						oScore++;
						Delay100ms(2);
						Finished=0;
						printScore();
						goto begin;
					}
				}
				if(Winner=='T'){ //Tie case
					if(Finished){
					Nokia5110_Clear();
					Nokia5110_SetCursor(0,2);
					Nokia5110_OutString("Tie");
						Delay100ms(2);
						Finished=0;
						printScore();
						goto begin;
					}
				}
			}	
		
  }

}
//Win Test Main Function
char WinTest(void){
	short i,x,y;
	for(i=0;i<3;i++){
		x=WinRow(i); //call winRow , check every Row if it has a winner
		y=WinCol(i);//call winCol , check every Coloumn if it has a winner
		if(x==1|| y==1){ 
		return'X';
		}
		else if(x==(-1) || y==(-1)){
		return 'O';}
	}
	x=WinDiagLeft(); //call WinDiagLeft,check if left diagonal has a winner 
	y=WinDiagRight();//call WinDiagRight,check if Right diagonal has a winner 
	if(x==1 || y==1){
	return 'X'; }
	if(x==(-1) || y==(-1)){
	return'O';
	}
if(isTie()==1){ //Tie case
	return 'T';
}
return 0;}
short WinRow(short r){ //check if a winner exists in Row r
	short i;
    char xy; 
	short val =0;
    for(i=0;i<3;i++) { //iterate on all elements of row r
        xy = Position[r][i];//store value in xy
        if(xy == 'X') ++val; //if value is X , then increment value of VAL counter
        if(xy == 'O') val=val-1;//if value is O , then decrement value of VAL counter
    }
    return val/3; //returns either 1=> X won, -1=>Y won
}
short WinCol(short c){ //check if a winner exists in col c
	short i;
    char xy; 
	short val =0;
    for(i=0;i<3;i++) { //iterate on all elements of col c
        xy = Position[i][c]; //store value in xy
        if(xy == 'X') ++val;//if value is X , then increment value of VAL counter
        if(xy == 'O') val=val-1; //if value is O , then decrement value of VAL counter
    }
    return val/3; //returns either 1=> X won, -1=>Y won
}
short WinDiagLeft(void){               //check if a winner exists in left diagonal
	short i;
    char xy; 
	short val =0;
    for(i=0;i<3;i++) {              //iterate on the three elements of the left diag
        xy = Position[i][i]; 
        if(xy == 'X') ++val;      //if value is X , then increment value of VAL counter
        if(xy == 'O') val=val-1; //if value is  O , then decrement value of VAL counter
    }
    return val/3;               //returns either 1=> X won, -1=>Y won
}
short WinDiagRight(void){ //check if a winner exists in right diagonal
	short i;
    char xy; 
	short val =0;
    for(i=0;i<3;i++) { //iterate on the three elements of the right diag
        xy = Position[i][2-i]; 
        if(xy == 'X') ++val;     //if value is X , then increment value of VAL counter
        if(xy == 'O') val=val-1; //if value is O , then decrement value of VAL counter
    }
    return val/3;                //returns either 1=> X won,-1=>Y won
}
short isTie(void){ //Check if Tie
short i,j,Flag=0;
	char xy;
	for(i=0;i<3;i++){ //loop the whole matrix,check if there is still an empty place
	for(j=0;j<3;j++){
		xy=Position[i][j];
		if(xy!='X' && xy!='O'){ 
		Flag++; //if there is an empty place increment the flag
		}
	}
	}
	if(Flag==0){ //if no empty place,return 1
	return 1;
	}
return 0; //if not return 0
}
void printScore(void)
{
	Nokia5110_ClearBuffer();
	Nokia5110_DisplayBuffer();      // draw buffer
	Nokia5110_Clear();
	Nokia5110_SetCursor( 3 , 0 );
	Nokia5110_OutString("Score");
	Nokia5110_SetCursor(3, 2);
	Nokia5110_OutString("X:");
	Nokia5110_SetCursor(5,  2);
	Nokia5110_OutChar ('0' + xScore);
	Nokia5110_SetCursor(3, 4);
	Nokia5110_OutString("O:");
	Nokia5110_SetCursor(5,  4);
	Nokia5110_OutChar('0' + oScore);
	Delay100ms(5);
	Winner=0;
	Round++;
	printRound();
}
void printRound(void){
	Nokia5110_ClearBuffer();
	Nokia5110_DisplayBuffer();      // draw buffer
	Nokia5110_Clear();
	Nokia5110_SetCursor( 3 , 0 );
	Nokia5110_OutString("Round");
		Nokia5110_SetCursor(9,  0);
	Nokia5110_OutChar ('0' + Round);
	Nokia5110_SetCursor(3, 3);
	Nokia5110_OutString("START!!");
	Delay100ms(5);
}


void Delay100ms(unsigned long count){unsigned long volatile time;
  while(count>0){
    time = 727240;  // 0.1sec at 80 MHz
    while(time){
				time--;
    }
    count--;
  }
}
