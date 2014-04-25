//File: bres.h
//Name: Jesse Johnson
//Transy Fall 2013
//CS 3014
//pseudocode for bres algorithm

/*
end = false

while (!end){

	get values for x1, y1, x2, y2

	dx <- |x2 - x1|
	dy <- |y2 - y1|

	dx2 <- 2 * dx
	dy2 <- 2 * dy

	if (x2 >= x1)then 
	   col <- x1
	   row <- y1
	else
	   col <- x2
	   row <- y2
	endif

	g <- dy2 - dx
	f <- dx2 - dy

	print values of column and row
   
          while ((col == x2 AND row == y2)'), do

		

                    if (g > 0),then
                          g <- g + dy2 - dx2
                          if (y2 > y1), then
                          	row <- row + 1
                          else
                          	row <- row - 1
                          endif
                    else
                                 g <- g + dy2
                    endif

                    if (f > 0), then
                          col <- col + 1
                          f <- f + dx2 - dy2
                    else
                          f <- f + dx2
                    endif
		print values of column and row      
          endwhile
          
          
          again <- 'j'
 
          print "Would you like to try new coordinates?"

          while (again != 'y' AND again != 'n'),then                            
                print "(y/n): "
                cin>> again
          endwhile

          if (again == 'n'), then 
                    end <- true
          endif

endwhile 
                


          
         
