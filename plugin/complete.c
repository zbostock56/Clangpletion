#include <stdio.h>
 
int call(int x);

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
  
  int ten = call(i);
}

int call(int x) {
  return 10;
}

