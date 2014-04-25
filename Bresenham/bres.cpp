//Name: Jesse Johnson
//File: Bresenham_Algorithm.cpp
//Transy Fall 2013
//CS 3014

////This program will demonstrate Bresenham's algorithmm. It will prompt for coordinates of two cartesian points and then list the coordinates of the pixels that will be shaded to best represent the line. 


#include <iostream>
#include <cmath>
using namespace std;


void print(int x, int y);


////
//main program///
///
int main(){
    
////all variables////
    int x1, y1, x2, y2; //coordiate variables
    int row, col;	//marker for rows and columns

         //rate of change of x and y and Bresenham function int
    int dy, dx, dy2, dx2, g, f;
    char again;//user input variable to either quit program or repeat
    bool end = false;//loop parameter for main task


    //give instructions
    cout<<endl<<endl<<"Hello! Welcome to a demonstration of the Bresenham Algorithm!"
           <<endl<<"Input values in this coordiate format: (x1, y1), (x2, y2)"
           <<endl<<endl;
    
    //Begin task loop. Break when user quits program
    while (!end){
          
          //prompt for inputs 
          cout<<endl<<endl<<"x1: ";
          cin>>x1;
          cout<<endl<<"y1: ";
          cin>> y1;
          cout<<endl<<"x2: ";
          cin>> x2;
          cout<<endl<<"y2: ";
          cin>> y2;
          cout<<endl<<endl;
          
          //calculate variable values
          dy = abs(y2 - y1);
          dx = abs(x2 - x1);
          dy2 = 2 * dy;
          dx2 = 2 * dx;  
          
          if (x2 >= x1){
                 col = x1;
                 row = y1;
          }else{
                 col = x2;
                 row = y2;
          }      

          g = dy2 - dx;
          f = dx2 - dy;
    
          //print initial point coord
          
	  print(col, row);
          //Begin algorithm loop. Break when final row has been reached.
          
          while (!(col == x2 && row == y2)){
		
                    if (g > 0){
                          g = g + dy2 - dx2;
                          if (y2 > y1){
                                 ++row;
                          }else{
                                 --row;
                          }
                    }
                    else{
                                 g = g + dy2;
                    }   
                    if (f > 0){
                          ++col;
                          f = f + dx2 - dy2;
                    }
                    else{
                          f = f + dx2;
                    }
		    print(col, row);
}
          
          
          //"again" char variable set to some garbage value
          again = 'j'; 
          cout<<endl<<endl<<endl<<"Would you like to try new coordinates?";
          while (again != 'y' && again != 'n'){                            
                cout<<"(y/n): ";
                cin>> again;
          }
          if (again == 'n'){ 
                    end = true;
          }
    }  
                

return 0;
}


////functions

void print(int x, int y){
     cout<<"("<<x<<", "<<y<<")"<<endl;
}
