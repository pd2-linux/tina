1. misc_upgrade 是基于小容量flash方案重新划分分区后，以misc分区、extend分区为媒介设计的OTA方案

2. OTA镜像包SDK编译说明：(SDK根目录)
   环境变量：
   source scripts/setenv.sh
   
   编译命令：
   make_ota_image (在新版本代码已经成功编译出烧录固件的环境的基础上，打包OTA镜像)
   make_ota_image --force (重新编译新版本代码，然后再打包OTA镜像)
   
3. OTA镜像包说明：
    \*0*/ $ ll bin/sunxi/ota/ 
    ?????? 20856
    drwxrwxr-x 5 heweihong heweihong     4096  3?? 23 15:48 ./
    drwxr-xr-x 5 heweihong heweihong     4096  3?? 23 15:48 ../
    -rw-rw-r-- 1 heweihong heweihong   143172  3?? 23 15:47 .config.old
    drwxrwxr-x 2 heweihong heweihong     4096  3?? 23 15:48 ramdisk_sys/
    -rw-rw-r-- 1 heweihong heweihong  5731339  3?? 23 15:48 ramdisk_sys.tar.gz
    drwxrwxr-x 2 heweihong heweihong     4096  3?? 23 15:47 target_sys/
    -rw-rw-r-- 1 heweihong heweihong 10335244  3?? 23 15:48 target_sys.tar.gz
    drwxrwxr-x 2 heweihong heweihong     4096  3?? 23 15:47 usr_sys/
    -rw-rw-r-- 1 heweihong heweihong  5116895  3?? 23 15:48 usr_sys.tar.gz
    
    三个tar包就是OTA的镜像包
    ramdisk_sys.tar.gz：ramdisk镜像（要升级内核分区、rootfs分区时使用）
    target_sys.tar.gz： 完整系统镜像（升级内核分区、rootfs分区、extend分区，usr_sys.tar.gz这个包不会使用到）
    usr_sys.tar.gz：    应用分区镜像（升级extend分区，只需要使用这个镜像）
    
4. 小机端OTA升级命令：
    aw_upgrade_process.sh --force 升级完整系统（内核分区、rootfs分区、extend分区、使用ramdisk_sys.tar.gz target_sys.tar.gz）
    aw_upgrade_process.sh --post  升级应用分区（extend分区，使用usr_sys.tar.gz） 
    
5. 脚本接口说明：
    aw_upgrade_vendor.sh
    
    实现联网逻辑
    check_network_vendor(){
        return 0 联网成功（如：可以ping通OTA镜像服务器）
        return 1 联网失败
    }
    
    请求下载目标镜像， $1：ramdisk_sys.tar.gz $2：/tmp
    download_image_vendor(){
        # $1 image name  $2 DIR  $@ others
        rm -rf $2/$1
        echo "wget $ADDR/$1"
        wget $ADDR/$1 -P $2
    }
    
    开始烧写分区状态：
    aw_upgrade_process.sh --force （--post）这两种主动升级的模式下，返回0开始写分区  1不写分区 
    upgrade_start_vendor(){
        # $1 mode: upgrade_pre,boot-recovery,upgrade_post
        #return   0 -> start upgrade;  1 -> no upgrade
        #reutrn value only work in nornal mode
        #nornal mode: $NORMAL_MODE
        echo upgrade_start_vendor $1
        return 0
    }
    
    写分区完成
    upgrade_finish_vendor(){
        #set version
        write_misc -v henrisk_test_v1 -s ok
        reboot -f
    }

    调用顺序： check_network_vendor -> download_image_vendor -> download_image_vendor ... -> upgrade_start_vendor -> upgrade_finish_vendor