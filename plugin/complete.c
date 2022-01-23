#include <stdio.h>
 
int main() {
  int i = 0;
  int j = i + 5;
  
  struct Point {
    int x;
    int y;
    int z;
  }

  struct Point pt1 = { 10, 10, 20 };
 
  struct Point pt2 = { 10, 20, 30 };

  int x = pt2.x;
}



