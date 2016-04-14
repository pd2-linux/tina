#修改2016-34-14 增加分区相关说明，删除target_sys中usr.img部分，增加大容量升级说明
#修改2016-3-23  初始版本
#author：henrisk

一、分区定义：
    boot分区：存内核镜像
    rootfs分区：基础系统镜像分区（/lib, /bin, /etc, /sbin等非/usr的路径，wifi支持环境，alsa支持环境、OTA环境）
    extend分区：扩展系统镜像分区（/usr 应用分区）
    上面三个分区为升级分区

    private分区：存储SN号分区
    misc分区：系统状态、刷机状态分区
    UDISK分区：用户数据分区（/mnt/UDISK）
    overlayfs分区：存储overlayfs覆盖数据
    上面分区为不升级分区

二、分区大小注意事项：
    分区大小在方案使用sys_partition.fex中定义
    size的算法：如 8192/2/1024 = 4M
    
    a）配置boot分区大小，boot分区大小需要依赖内核配置，需要小于等于sys_partition.fex中定义的boot标签的定义：
    如： 
        [partition]
            name         = boot
            size         = 8192
            downloadfile = "boot.fex"
            user_type    = 0x8000

        boot分区镜像大小的设定：
        make menuconfig
            Target Images  --->
                *** Image Options ***
                (4) Boot filesystem partition size (in MB)
    
    b）rootfs分区的大小，不需要通过make menuconfig去设定，直接根据镜像大小修改分区文件即可。
        1）对于一些小容量flash的方案（如16M），需在/bin 下存放联网逻辑程序、版本控制程序、下载镜像程序、播报语音程序以及语音文件（这些文件在编译时应该install到/bin或者/lib下）
       可以在固件编译完后，查看bin/sunxi(sun5i)/下rootfs.img的大小在决定sys_partition.fex中rootfs分区的大小，如
       \*0*/ $ ll bin/sun5i/rootfs.img
        -rw-r--r-- 1 heweihong heweihong  1835008  4月 14 16:44 bin/sun5i/rootfs.img

        2）对于大容量flash的方案（如128M以上，或者有足够的flash空间存相关镜像），不需要1）中那些OTA额外的程序，直接查看rootfs.img的大小设定分区文件即可。

    c）extend分区的大小，需要考虑多个方面：
        1）编译后 usr.img的大小
        2）make_ota_image后initramfs镜像的大小
        如：
        \*0*/ $ ll build_dir/target-arm_cortex-xxxxxxxx/linux-sun5i（linux-sunxi）/
        -rw-r--r--  1 heweihong heweihong   479232  4月 14 16:44 usr.squashfs
        -rwxr-xr-x  1 heweihong heweihong  5510192  4月 14 16:44 zImage-initramfs*
        取两个最大值，并增加一些余量即可
        
        并把这个值设置为initramfs镜像的大小
        make menuconfig
            Target Images  --->
                *** Image Options ***
                (8) Boot-Recovery initramfs filesystem partition size (in MB)

    d）其他分区如private、misc等使用默认的大小即可
    e）剩下的空间全部自动分配进入UDISK分区
    
    特别注意：这些分区大小不能通过OTA去修改的，所以对于大容量flash的方案，应该在满足分区条件限制（如上面adc三点）的情况下留有足够的余量，满足后续OTA增加内容的需求。
    对于小容量flash的方案，需要在增加内容是调节相关分区的大小。

三、misc-upgrade升级
1. misc-upgrade 是基于小容量flash方案重新划分分区后，以misc分区、extend分区为媒介设计的OTA方案

2. OTA镜像包SDK编译说明：(SDK根目录)
   环境变量：
   source scripts/setenv.sh
   
   编译命令：
   make_ota_image (在新版本代码已经成功编译出烧录固件的环境的基础上，打包OTA镜像)
   make_ota_image --force (重新编译新版本代码，然后再打包OTA镜像)
   
3. OTA镜像包说明：
    \*0*/ $ ll bin/sunxi（sun5i）/ota/ 
    ?????? 20856
    -rw-rw-r-- 1 heweihong heweihong  5731339  3?? 23 15:48 ramdisk_sys.tar.gz
    -rw-rw-r-- 1 heweihong heweihong 10335244  3?? 23 15:48 target_sys.tar.gz
    -rw-rw-r-- 1 heweihong heweihong  5116895  3?? 23 15:48 usr_sys.tar.gz
    
    三个tar包就是OTA的镜像包
    ramdisk_sys.tar.gz：ramdisk镜像（要升级内核分区、rootfs分区时使用，防止烧写过程掉电，导致机器变砖）
    target_sys.tar.gz： 系统镜像（升级内核分区、rootfs分区）
    usr_sys.tar.gz：    应用分区镜像（升级extend分区，只需要使用这个镜像）
    
4. 小机端OTA升级命令：
    必选参数：-f -p 二选一
    aw_upgrade_process.sh -f 升级完整系统（内核分区、rootfs分区、extend分区、使用ramdisk_sys.tar.gz target_sys.tar.gz usr_sys.tar.gz）
    aw_upgrade_process.sh -p 升级应用分区（extend分区，使用usr_sys.tar.gz）
    
    可选参数: -l
    aw_upgrade_process.sh -p(-f) -l /mnt/UDISK/misc-upgrade
    
    a）对于大容量flash方案可以使用本地镜像，如主程序下载好三个镜像后（ramdisk、target、usr），存在/mnt/UDISK/misc-upgrade中，调用上的命令，
    对于自动烧写分区，就算期间掉电，重启后升级程序也能自动完成烧写，不需要依赖网络。
    b）对于小容量flas方案不能使用-l参数，升级区间出错重启后，还需要根据相关的联网下载程序获取镜像（见第5点说明）
    
5. 脚本接口说明：
    对于小容量flash的方案，需要实现下面钩子脚本
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
    aw_upgrade_process.sh -p 主动升级应用分区的模式下，返回0开始写分区  1不写分区
    aw_upgrade_process.sh -f 不理会这个返回值
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
        #set version or others
        reboot -f
    }

    -f调用顺序： 
            check_network_vendor ->
              upgrade_start_vendor ->             
                download_image_vendor (ramdisk_sys.tar.gz)->
                  内部烧写、清除镜像逻辑（不让已经使用镜像占用内存） ->
                    download_image_vendor(target_sys.tar.gz) ->
                      内部烧写、清除镜像逻辑（不让已经使用镜像占用内存） ->
                        download_image_vendor(usr_sys.tar.gz) ->
                          内部烧写、清除镜像逻辑（不让已经使用镜像占用内存） ->
                            upgrade_finish_vendor
    -p调用顺序
            check_network_vendor ->
              download_image_vendor (usr_sys.tar.gz) ->
                upgrade_start_vendor ->             
                  检测返回值，烧写 ->
                    upgrade_finish_vendor
                    