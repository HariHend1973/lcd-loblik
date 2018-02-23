lcd-loblik (tested on raspberry pi 2/lede 17.01.4)

repacked to lede/openwrt package based on source code https://bitbucket.org/loblik/lcd.git

git clone https://github.com/HariHend1973/lcd-loblik.git cp lcd-loblik to package/utils/ make defconig make menuconfig select Utilities -> lcd-loblik (press m) -> save

make V=99

binary lcd-loblik_0.1-1_arm_cortex-a7_neon-vfpv4.ipk will be at bin//packages/arm_cortex-a7_neon-vfpv4/base/ (i'm using raspberry bcm2709)
