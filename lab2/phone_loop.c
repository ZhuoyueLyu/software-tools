#include <stdio.h>

int main() {
    char phone[11];
    int num;
    int result = 0;
    scanf("%s", phone);
    while(scanf("%d", &num) != EOF){
    
    if(num == -1){
        printf("%s\n", phone);
    }else if(0 <= num && num  <= 9){
        printf("%c\n", phone[num]);
    }
    else if(num < -1 || num > 9){
        printf("ERROR\n");
        result = 1;
    }
}
return result;
}