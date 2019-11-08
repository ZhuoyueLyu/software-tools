#include <stdio.h>

int main() {
    char phone[11];
    int num;
    scanf("%s %d", phone, &num);
    
    if(num == -1){
        printf("%s\n", phone);
        return 0;
    }else if(0 <= num && num  <= 9){
        printf("%c\n", phone[num]);
        return 0;
    }
    else if(num < -1 || num > 9){
        printf("ERROR\n");
        return 1;
    }
   
}