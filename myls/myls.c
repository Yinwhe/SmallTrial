#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <errno.h> 
#include <time.h>
#include <pwd.h>
#include <grp.h>
#include <dirent.h>

/* 颜色设置 */
#define NONE        "\e[0m"     //清除颜色，即之后的打印为正常输出，之前的不受影响
#define BLACK       "\e[0;30m"  //深黑
#define L_BLACK     "\e[1;30m"  //亮黑，偏灰褐
#define RED         "\e[0;31m"  //深红，暗红
#define L_RED       "\e[1;31m"  //鲜红
#define GREEN       "\e[0;32m"  //深绿，暗绿
#define L_GREEN     "\e[1;32m"  //鲜绿
#define BROWN       "\e[0;33m"  //深黄，暗黄
#define YELLOW      "\e[1;33m"  //鲜黄
#define BLUE        "\e[0;34m"  //深蓝，暗蓝
#define L_BLUE      "\e[1;34m"  //亮蓝，偏白灰
#define PURPLE      "\e[0;35m"  //深粉，暗粉，偏暗紫
#define L_PURPLE    "\e[1;35m"  //亮粉，偏白灰
#define CYAN        "\e[0;36m"  //暗青色
#define L_CYAN      "\e[1;36m"  //鲜亮青色
#define GRAY        "\e[0;37m"  //灰色
#define WHITE       "\e[1;37m"  //白色，字体粗一点，比正常大，比bold小
#define BOLD        "\e[1m"     //白色，粗体

/* Options控制 */
#define OP_a 1<<0   // 列出包括隐含文件在内的所有文件
#define OP_A 1<<1   // 同-a，但不显示"."和".."
#define OP_c 1<<2   // with -lt: 显示并按ctime排序 (更改时间);with -l: 显示ctime，按name排序; none: 按ctime排序
#define OP_d 1<<3   // 列出目录本身，而不是它们的内容
#define OP_f 1<<4   // 不排序，启用-aU，禁用-ls --color
#define OP_F 1<<5   // 在每个文件名后附上一个字符以阐明该文件的类型，"*"表明可执行的一般文件;"/"表明目录;"@"表明符号连接;"|"表明FIFOs;"="表明sockets
#define OP_g 1<<6   // 同-l，但不显示拥有者
#define OP_G 1<<7   // 如有-l，不显示组
#define OP_h 1<<8   // 易读的格式
#define OP_i 1<<9   // 显示inode
#define OP_l 1<<10  // 长格式显示
#define OP_m 1<<11  // 横向输出文件名，并以","作分格符
#define OP_n 1<<12  // 如-l，但要列出数字用户和组id
#define OP_o 1<<13  // 同-l，但不列出组信息
#define OP_p 1<<14  // 添加/指示符到目录
#define OP_Q 1<<15  // 把输出的文件名用双引号括起来
#define OP_r 1<<16  // 逆序排列
#define OP_R 1<<17  // 递归显示子目录内的文件
#define OP_s 1<<18  // 打印每个文件的分配大小，以块为单位
#define OP_t 1<<19  // 以时刻排序，默认mtime
#define OP_u 1<<20  // with -lt: 显示并按atime排序 (更改时间);with -l: 显示atime，按name排序; none: 按atime排序
#define OP_1 1<<21  // 一行只输出一个文件

#define MAX_FILE_ONCE 100   // 一次最多有100个file参数
#define MAX_FILE_NAME 256   // 每个file名称最长为100

int options = 0;

int main(int argc, char **argv){

    int filenum = 0;

    char file[MAX_FILE_ONCE][MAX_FILE_NAME];

    for(int i=1; i<argc; i++){// 顺序扫描输入的参数
        if(argv[i][0] == '-'){// 输入的是Option
            for(int j=1; j<strlen(argv[i]); j++){
                switch(argv[i][j]){
                    case 'a': options |= OP_a; break;
                    case 'A': options |= OP_A; break;
                    case 'c': options |= OP_c; break;
                    case 'd': options |= OP_d; break;
                    case 'f': options |= OP_f; break;
                    case 'F': options |= OP_F; break;
                    case 'g': options |= OP_g; break;
                    case 'G': options |= OP_G; break;
                    case 'h': options |= OP_h; break;
                    case 'i': options |= OP_i; break;
                    case 'm': options |= OP_m; break;
                    case 'n': options |= OP_n; break;
                    case 'o': options |= OP_o; break;
                    case 'p': options |= OP_p; break;
                    case 'Q': options |= OP_Q; break;
                    case 'r': options |= OP_r; break;
                    case 'R': options |= OP_R; break;
                    case 's': options |= OP_s; break;
                    case 't': options |= OP_t; break;
                    case 'u': options |= OP_u; break;
                    case '1': options |= OP_1; break;
                    default: printf(L_RED "Error: Unknown option!" NONE); return 0;
                }
            }
        }
        else{// 输入的是文件路径
            if(strlen(argv[i]) >= MAX_FILE_NAME){
                printf(L_RED "Error: Path too long!" NONE); return 0;
            }
            strcpy(file[filenum++], argv[i]);
        }
    }

    if(filenum == 0){
        strcpy(file[filenum++], "./");
    }
}