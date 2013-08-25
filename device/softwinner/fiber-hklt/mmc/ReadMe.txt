制作SD卡/TF卡启动镜像方法：
1.将cardboot.zip解压，
2.解压出的文件替换掉上层目录下的文件(删除解压后的文件和文件夹)，
3.重新编译打包即可

1.unzip cardboot.zip
2.cp init.sun6i.rc recovery.fstab vold.fstab ../
3.make installclean;make -j8;pack

制作emmc卡启动镜像方法：
1.将emmcboot.zip解压，
2.解压出的文件替换掉上层目录下的文件(删除解压后的文件和文件夹)，
3.重新编译
4.修改lichee/tools/pack/chips/sun6i/configs/android/fiber-evb/sys_config.fex，改动如下：

    storage_type = 2
    nand0_used = 0
    nand1_used = 0
    [mmc2_para]
    sdc_used = 1
    sdc_detmode = 3
	
	如果使用lpddr的板子加上
    lpddr dram_clk = 240
    dram_type = 6
    dcdc5_vol = 1200 


制作电阻屏镜像法：
1.将fiber-evb-rtp-device.tgz解压，解压后的文件替换掉上层目录下的文件
2.将fiber-evb-rtp-config.tgz解压，解压后的sysconfig文件替换掉lichee/tools/pack/sun6i/configs/fiber-evb下的文件
3.重新编译打包

