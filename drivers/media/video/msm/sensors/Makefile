GCC_VERSION      := $(shell $(CONFIG_SHELL) $(PWD)/scripts/gcc-version.sh $(CROSS_COMPILE)gcc)
EXTRA_CFLAGS += -Idrivers/media/video/msm
EXTRA_CFLAGS += -Idrivers/media/video/msm/io
EXTRA_CFLAGS += -Idrivers/media/video/msm/eeprom
EXTRA_CFLAGS += -Idrivers/media/video/msm/csi
obj-$(CONFIG_MSM_CAMERA_SENSOR) += msm_sensor.o
obj-$(CONFIG_T4K04) += t4k04_v4l2.o  #ZTEBSP yuxin add for P865G01 PRJ sensors,2013.01.17
obj-$(CONFIG_HI542) += hi542_v4l2.o
obj-$(CONFIG_AR0542) += ar0542_v4l2.o  #ZTEBSP yuxin add for P865G01 use AR0542 2013.03.04
obj-$(CONFIG_OV2675) += ov2675_v4l2.o  #ZTEBSP yuxin add for P865E02 PRJ sensors,2012.10.22
obj-$(CONFIG_SP0A28) += sp0a28_v4l2.o
obj-$(CONFIG_OV5647) += ov5647_v4l2.o
obj-$(CONFIG_OV8825) += ov8825_v4l2.o
obj-$(CONFIG_AR0543) += ar0543_v4l2.o
obj-$(CONFIG_IMX074) += imx074_v4l2.o
obj-$(CONFIG_S5K3L1YX) += s5k3l1yx.o
obj-$(CONFIG_IMX091) += imx091.o
obj-$(CONFIG_OV2720) += ov2720.o
obj-$(CONFIG_MT9M114) += mt9m114_v4l2.o
obj-$(CONFIG_S5K4E1) += s5k4e1_v4l2.o
obj-$(CONFIG_MT9E013) += mt9e013_v4l2.o
obj-$(CONFIG_WEBCAM_OV9726) += ov9726_v4l2.o
obj-$(CONFIG_OV5647_TRULY_CM6868) += ov5647_truly_cm6868_v4l2.o
obj-$(CONFIG_OV5648_TRULY_CM8352) += ov5648_truly_cm8352_v4l2.o
obj-$(CONFIG_OV7692) += ov7692_v4l2.o
obj-$(CONFIG_OV7695) += ov7695_v4l2.o
obj-$(CONFIG_OV7695_RAW) += ov7695_raw_v4l2.o
obj-$(CONFIG_MT9V115) += mt9v115_v4l2.o
obj-$(CONFIG_VX6953) += vx6953.o
obj-$(CONFIG_A8140) += a8140_v4l2.o
obj-$(CONFIG_S5K3H2_SUNNY_Q8S02E) += s5k3h2_sunny_q8s02e_v4l2.o
#ZTEBSP zhangzhao add for v50 cameras,2012.02.18 ++
#ZTEBSP zhangqi add for ov7692 ov5640 jt8ev5 compatible ,please don't change the sequence of those objs start
obj-$(CONFIG_OV5640) += ov5640_v4l2.o
obj-$(CONFIG_MT9D115) += mt9d115_v4l2.o
obj-$(CONFIG_S5K5CA) += s5k5ca_v4l2.o
obj-$(CONFIG_JT8EV5) += jt8ev5_v4l2.o
obj-$(CONFIG_OV7692) += ov7692_v4l2.o
#ZTEBSP zhangqi add for ov7692 ov5640 jt8ev5 compatible ,please don't change the sequence of those objs end
#ZTEBSP zhangzhao add for v50 cameras,2012.02.18--