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
#define OP_a (1L<<0 )  // 列出包括隐含文件在内的所有文件
#define OP_A (1L<<1 )  // 同-a，但不显示"."和".."
#define OP_c (1L<<2 )  // with -lt: 显示并按ctime排序 (更改时间);with -l: 显示ctime，按name排序; none: 按ctime排序
#define OP_d (1L<<3 )  // 列出目录本身，而不是它们的内容
#define OP_S (1L<<4 )  // 按文件大小排序
#define OP_F (1L<<5 )  // 在每个文件名后附上一个字符以阐明该文件的类型，"*"表明可执行的一般文件;"/"表明目录;"@"表明符号连接;"|"表明FIFOs;"="表明sockets
#define OP_g (1L<<6 )  // 同-l，但不显示拥有者
#define OP_G (1L<<7 )  // 如有-l，不显示组
#define OP_h (1L<<8 )  // 易读的格式
#define OP_i (1L<<9 )  // 显示inode
#define OP_l (1L<<10)  // 长格式显示
#define OP_m (1L<<11)  // 横向输出文件名，并以","作分格符
#define OP_n (1L<<12)  // 如-l，但要列出数字用户和组id
#define OP_o (1L<<13)  // 同-l，但不列出组信息
#define OP_p (1L<<14)  // 添加/指示符到目录
#define OP_Q (1L<<15)  // 把输出的文件名用双引号括起来
#define OP_r (1L<<16)  // 逆序排列
#define OP_R (1L<<17)  // 递归显示子目录内的文件
#define OP_s (1L<<18)  // 打印每个文件的分配大小，以块为单位
#define OP_t (1L<<19)  // 以时刻排序，默认mtime
#define OP_u (1L<<20)  // with -lt: 显示并按atime排序 (更改时间);with -+l: 显示atime，按name排序; none: 按atime排序
#define OP_1 (1L<<21)  // 一行只输出一个文件

/* Longformat控制 */
#define LONG    (1L<<0) // 长格式输出
#define OWNER   (1L<<1) // 输出拥有者信息
#define GROUP   (1L<<2) // 输出组信息
#define NUM     (1L<<3) // 信息按id输出

#define MAX_FILE_ONCE 128   // 一次最多有128个file参数
#define MAX_FILE_NAME 1024   // 每个file名称最长为1024
#define MAX_FILE_NUM  1024  // 假设一个文件夹最多1024个文件

int     cmp(char *file1, char *file2, int op);
void    order(char (*file)[MAX_FILE_NAME], int filenum, int op);
void    list_file(char (*param)[MAX_FILE_NAME], int *file, int filenumn, int op);
void    list_dir(char (*param)[MAX_FILE_NAME], int *dir, int dirnum);
void    list_content_dir(char *dirname, char *path);
int    list_single(char *name);
void    list_attr(struct stat fstat);
void    info(char (*filename)[MAX_FILE_NAME], int filenum);

int options = 0;
int longformat = 6; // 0110
char cwd[MAX_FILE_NAME];


int main(int argc, char **argv){

    int paramnumber = 0, filenum = 0, dirnum = 0;

    char param[MAX_FILE_ONCE][MAX_FILE_NAME];
    int file[MAX_FILE_ONCE] = {0};
    int dir[MAX_FILE_ONCE] = {0};

    struct stat fstat;

    if(argc > MAX_FILE_ONCE){
        printf(L_RED "Error: Too many file to print once!");
        return 1;
    }

    for(int i=1; i<argc; i++){// 顺序扫描输入的参数
        if(argv[i][0] == '-'){// 输入的是Option
            for(int j=1; j<strlen(argv[i]); j++){
                switch(argv[i][j]){
                    case 'a': options |= OP_a; break;
                    case 'A': options |= OP_A; break;
                    case 'c': options |= OP_c; break;
                    case 'd': options |= OP_d; break;
                    case 'F': options |= OP_F; break;
                    case 'g': options |= OP_g; longformat |= LONG; longformat &= (~OWNER); break;
                    case 'G': options |= OP_G; longformat &= ~OWNER; break;
                    case 'h': options |= OP_h; break;
                    case 'i': options |= OP_i; break;
                    case 'l': options |= OP_l; longformat |= LONG; break;
                    case 'm': options |= OP_m; break;
                    case 'n': options |= OP_n; longformat |= NUM; break;
                    case 'o': options |= OP_o; longformat |= LONG; longformat &= (~GROUP); break;
                    case 'p': options |= OP_p; break;
                    case 'Q': options |= OP_Q; break;
                    case 'r': options |= OP_r; break;
                    case 'R': options |= OP_R; break;
                    case 's': options |= OP_s; break;
                    case 't': options |= OP_t; break;
                    case 'u': options |= OP_u; break;
                    case '1': options |= OP_1; break;
                    default: printf(L_RED "Error: Unknown option!" NONE); return 1;
                }
            }
        }
        else{// 输入的是文件路径
            if(strlen(argv[i]) >= MAX_FILE_NAME){
                printf(L_RED "Error: Path too long!" NONE);
                return 1;
            }
            strcpy(param[paramnumber++], argv[i]);
        }
    }

    if(paramnumber == 0){// 没有文件输入，默认为当前目录
        strcpy(param[paramnumber++], "./");
    }

    /* 遍历所有param并判断是文件还是目录 */
    for(int i=0; i<paramnumber; i++){
        if(lstat(param[i], &fstat) == -1){// 文件或目录不存在
            printf(L_RED "myls: cannot access '%s': No such file or directory\n" NONE, param[i]);
            return 1;
        }
        /* 分别处理文件和目录，但如果有-d选项，则目录也认为是文件 */
        if((options & OP_d) || !S_ISDIR(fstat.st_mode)){
            file[filenum++]=i;
        }else{
            dir[dirnum++]=i;
        }
    }

    if(getcwd(cwd, sizeof(cwd)) == NULL){
        printf(L_RED "Error: CWD too long!\n" NONE);
        return 1;
    }

    /* 先处理文件类型的输出 */
    list_file(param, file, filenum, 0);

    /* 在处理目录类型的输出 */
    list_dir(param, dir, dirnum);
}

/**
 * @brief 文件间的比较
 * 
 * @param op 0--按名称
 *           1--按ctime
 *           2--按atime
 *           3--按mtime
 *           4--按大小
 */
int cmp(char *file1, char *file2, int op){
    if(op == 0) // 字典序
        return strcmp(file1, file2);
    
    /* 获取时间信息 */
    struct stat fstat1;
    struct stat fstat2;
    lstat(file1, &fstat1);
    lstat(file2, &fstat2);

    switch(op){
        case 1: return fstat2.st_ctime - fstat1.st_ctime;
        case 2: return fstat2.st_atime - fstat1.st_atime;
        case 3: return fstat2.st_mtime - fstat1.st_mtime;
        case 4: return fstat2.st_size - fstat1.st_size;
        default: printf(L_RED "Internal error: cmp\n" NONE); while(1);
    }
}


/**
 * @brief 对文件进行排序
 * 
 * @param file 文件列表
 * @param filenum 文件数量
 * @param op 0--按名称排列
 *           1--按ctime排列
 *           2--按atime排列
 *           3--按mtime排列
 *           4--按大小排列
 */
void order(char (*file)[MAX_FILE_NAME], int filenum, int op){
    char tmp[MAX_FILE_NAME];
    int i, j;
    /* 使用插排进行排序 */
    if(options & OP_r){ // 逆序排列
        for(i=1;i<filenum;i++){
            strcpy(tmp, file[i]);
            for(j=i; j>0 && cmp(file[j-1], tmp, op)<0; j--)
                strcpy(file[j], file[j-1]);
            strcpy(file[j], tmp);
        }
    }else{
        for(i=1;i<filenum;i++){
            strcpy(tmp, file[i]);
            for(j=i; j>0 && cmp(file[j-1], tmp, op)>0; j--)
                strcpy(file[j], file[j-1]);
            strcpy(file[j], tmp);
        }
    }
}


/**
 * @brief 按顺序列出文件
 * 
 * @param param 
 * @param file 
 * @param filenum
 */
void list_file(char (*param)[MAX_FILE_NAME], int *file, int filenum, int op){
    if(filenum <= 0){
        return;
    }
    /* 开始处理 */
    char filename[filenum][MAX_FILE_NAME];
    if(!op){
        for(int i=0; i<filenum; i++){
            strcpy(filename[i], param[file[i]]);
        }
    }else{
        for(int i=0; i<filenum; i++){
            strcpy(filename[i], param[i]);
        }
    }

    if(op){
        /* 统计块数*/
        struct stat fstat;
        int totalblk = 0;
        for(int i=0; i<filenum; i++){
            lstat(filename[i], &fstat);
            totalblk += fstat.st_blocks;
        }
        if(longformat & LONG)
            printf("total:  %-6d\n", totalblk);
    }

    /* 处理排序 */
    if(options & OP_t){ // 按时间排序
        if(options & OP_c) // 按ctime
            order(filename, filenum, 1);
        else if(options & OP_u) // 按atime
            order(filename, filenum, 2);
        else // 按mtime
            order(filename, filenum, 3);
    }else if(!(options & OP_l)){
        if(options & OP_c) // 按ctime
            order(filename, filenum, 1);
        else if(options & OP_u) // 按atime
            order(filename, filenum, 2);
        else
            order(filename, filenum, 0); // 字典序
    }else if(options & OP_S){
        order(filename, filenum, 4);    // 按大小
    }else{
        order(filename, filenum, 0);    // 字典序
    }

    info(filename, filenum); // 进行输出
    putchar('\n');
}

/**
 * @brief 按顺序列出目录内的内容
 * 
 * @param param 
 * @param dir 
 * @param dirnum 
 */
void list_dir(char (*param)[MAX_FILE_NAME], int *dir, int dirnum){
    if(dirnum <= 0){
        return;
    }

    for(int i=0; i<dirnum; i++){
        list_content_dir(param[dir[i]], "");
        chdir(cwd);
    }
}

void list_content_dir(char *dirname, char *path){
    /* 开始处理 */
    DIR *dp;
    struct dirent *entry;
    struct stat fstat;

    char filename[MAX_FILE_NUM][MAX_FILE_NAME], _path[MAX_FILE_NAME];
    int dirRev[MAX_FILE_NUM];
    int filenum = 0, dirRnum = 0;

    if((dp=opendir(dirname)) == NULL){
        printf(L_RED "myls: cannot access '%s': No such file or directory\n" NONE, dirname);
        return;
    }

    strcpy(_path, path);
    chdir(dirname);
    filenum = dirRnum = 0;
    while((entry = readdir(dp)) != NULL){
        strcpy(filename[filenum], entry->d_name);
        filenum += 1;
    }
    closedir(dp);

    printf("%s:\n", dirname);
    list_file(filename, NULL, filenum, 1);
    putchar('\n');

    /* 处理递归输出 */
    if(options & OP_R){
        for(int i=0; i<filenum; i++){
            lstat(filename[i], &fstat);
            if(S_ISDIR(fstat.st_mode)){
                dirRev[dirRnum++] = i;
            }
        }
    }

    if(dirRnum){
        strcat(_path, dirname);
        if(_path[strlen(_path)-1] != '/')
            strcat(_path, "/");
    }
    for(int i=0; i<dirRnum; i++){
        /* 处理不输出的目录 */
        if(filename[dirRev[i]][0] == '.'){
            if(filename[dirRev[i]][1] == '\0' || filename[dirRev[i]][1] == '.')
                continue;
            if(!(options & OP_a) && !(options & OP_A)){
                continue;
            }
        }
        printf("%s", _path);
        list_content_dir(filename[dirRev[i]], _path);
        chdir("..");
    }
}

static void trans(double num){
    static char unit[] = {'B', 'K', 'M', 'G', 'T', 'P'};
    int u=0;
    while(num>=0x400){
        u += 1;
        num /= 0x400;

        if(u == 5) break; // 过大
    }
    if(!u)
        printf("%6d%c", (int)num, unit[u]);
    else
        printf("%6.1f%c", num, unit[u]);
}

/**
 * @brief 列出一个单项
 * 
 * @param name 该项的名称
 * @return int 0表示正常输出，1表示未输出
 */
int list_single(char *name){
    struct stat fstat;
    //printf("list_single: %s\n", name);
    /* 处理不输出的文件 */
    if(name[0] == '.'){
        if(!(options & OP_a)){
            if(!(options & OP_A))
                return 1;
            else if(name[1] == '\0' || name[1] == '.')
                return 1;
        }
    }
    lstat(name, &fstat);
    /* 输出inode */
    if(options & OP_i) 
        printf("%8llu ", fstat.st_ino);

    /* 输出block size */
    if(options & OP_s){
        printf("%4lld ", fstat.st_blocks);
    }

    /* 是否长格式 */
    if(longformat & LONG){
        list_attr(fstat);
    }

    /* 是否加引号输出名称 */
    if(options & OP_Q){
        if (S_ISDIR(fstat.st_mode)){ //目录
            printf(L_CYAN "\"%s\"" NONE, name);
            if(options & OP_F)
                putchar('/');
        }
        else if (S_ISLNK(fstat.st_mode)){ //符号链接
            printf(L_PURPLE "\"%s\"" NONE, name);
            if(options & OP_F)
                putchar('@');
        }
        else if (S_ISSOCK(fstat.st_mode)){ // socket文件
            printf(L_BLUE "\"%s\"" NONE, name);
            if(options & OP_F)
                putchar('=');
        }
        else if (S_ISFIFO(fstat.st_mode)){ // 管道文件
            printf("\"%s\"", name);
            if(options & OP_F)
                putchar('|');
        }
        else if (fstat.st_mode & S_IXUSR){ // 一般可执行文件
            printf(L_GREEN "\"%s\"" NONE, name);
            if(options & OP_F)
                putchar('*');
        }
        else{
            printf("\"%s\"", name);
        }
    }else{
        if (S_ISDIR(fstat.st_mode)){ //目录
            printf(L_CYAN "%s" NONE, name);
            if(options & OP_F)
                putchar('/');
        }
        else if (S_ISLNK(fstat.st_mode)){ //符号链接
            printf(L_PURPLE "%s" NONE, name);
            if(options & OP_F)
                putchar('@');
        }
        else if (S_ISSOCK(fstat.st_mode)){ // socket文件
            printf(L_BLUE "%s" NONE, name);
            if(options & OP_F)
                putchar('=');
        }
        else if (S_ISFIFO(fstat.st_mode)){ // 管道文件
            printf("%s", name);
            if(options & OP_F)
                putchar('|');
        }
        else if (fstat.st_mode & S_IXUSR){ // 一般可执行文件
            printf(L_GREEN "%s" NONE, name);
            if(options & OP_F)
                putchar('*');
        }
        else{
            printf("%s", name);
        }
    }
    
    return 0;
}

void list_attr(struct stat fstat){
    struct passwd *pwd; //用户名
    struct group *grp;  //组名
    char time[30];      //时间信息

    /* 打印文件类型 */
    if (S_ISDIR(fstat.st_mode)) //目录
        printf("d");
    else if (S_ISLNK(fstat.st_mode)) //符号链接
        printf("l");
    else if (S_ISREG(fstat.st_mode)) //普通文件
        printf("-");
    else if (S_ISCHR(fstat.st_mode)) //字符设备
        printf("c");
    else if (S_ISBLK(fstat.st_mode)) //块设备
        printf("b");
    else if (S_ISFIFO(fstat.st_mode)) //管道文件
        printf("f");
    else if (S_ISSOCK(fstat.st_mode)) //socket文件
        printf("s");

    /* 打印user权限 */
    putchar( fstat.st_mode & S_IRUSR ? 'r' : '-' );
    putchar( fstat.st_mode & S_IWUSR ? 'w' : '-' );
    putchar( fstat.st_mode & S_IXUSR ? 'x' : '-' );
    /* 打印group权限 */
    putchar( fstat.st_mode & S_IRGRP ? 'r' : '-' );
    putchar( fstat.st_mode & S_IWGRP ? 'w' : '-' );
    putchar( fstat.st_mode & S_IXGRP ? 'x' : '-' );
    /* 打印other权限 */
    putchar( fstat.st_mode & S_IROTH ? 'r' : '-' );
    putchar( fstat.st_mode & S_IWOTH ? 'w' : '-' );
    putchar( fstat.st_mode & S_IXOTH ? 'x' : '-' );
    putchar(' ');

    /* 打印链接数 */
    printf("%2d", fstat.st_nlink);
    putchar(' ');

    /* 打印用户名与组名 */
    pwd = getpwuid(fstat.st_uid);
    grp = getgrgid(fstat.st_gid);
    if(longformat & OWNER){
        if(longformat & NUM) printf("%-6u ",fstat.st_uid);
        else printf("%-6s ", pwd->pw_name);
    }
    if(longformat & GROUP){
        if(longformat & NUM) printf("%-6u ",fstat.st_gid);
        else printf("%-6s ", grp->gr_name);
    }

    /* 打印文件的大小 */
    if(options & OP_h){
        trans(fstat.st_size);
    }else{
        printf("%lld", fstat.st_size);
    }
    putchar(' ');
    
    /* 打印时间 */
    if(options & OP_c)
       strcpy(time, ctime(&fstat.st_ctime));
    else if(options & OP_u)
       strcpy(time, ctime(&fstat.st_atime));
    else
       strcpy(time, ctime(&fstat.st_mtime)); //默认mtime

    time[strlen(time) - 1] = '\0'; //去掉'\n'
    printf(" %s ", time+4);
}

void info(char (*filename)[MAX_FILE_NAME], int filenum){
    struct stat fstat;
    int totalblk = 0;

    /* 逐一控制输出 */
    for(int i=0; i<filenum; i++){
        if(!list_single(filename[i])){
            /* 结尾分割的控制 */
            if(i == filenum-1) continue; // 最后一个跳过即可
            if((longformat & LONG) || (options & OP_1))
                printf("\n");
            else if(options & OP_m)
                printf(", ");
            else printf(" ");
        }
    }
}