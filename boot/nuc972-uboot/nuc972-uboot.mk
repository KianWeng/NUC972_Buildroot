################################################################################
#
# pmon
#
################################################################################
#NUC972_UBOOT_VERSION = $(call qstrip,$(BR2_BOOT_NUC972_UBOOT_VERSION))
NUC972_UBOOT_SITE = $(call qstrip,$(BR2_BOOT_NUC972_UBOOT_CUSTOM_LOCAL_PATH)) 
NUC972_UBOOT_SITE_METHOD = local
NUC972_UBOOT_DIR_PATH = output/build/nuc972-uboot

#NUC972_UBOOT_LICENSE = GPLv2+
#NUC972_UBOOT_LICENSE_FILES = README

NUC972_UBOOT_INSTALL_IMAGES = YES

define NUC972_UBOOT_BUILD_CMDS
	cd $(NUC972_UBOOT_DIR_PATH) && make nuc970_yytd_config && make
endef

define NUC972_UBOOT_INSTALL_IMAGES_CMDS
	cp $(NUC972_UBOOT_DIR_PATH)/u-boot.bin $(BINARIES_DIR) 
endef

$(eval $(generic-package))

ifeq ($(BR2_TARGET_NUC972_UBOOT),y)
# we NEED a board name unless we're at make source	
ifeq ($(filter source,$(MAKECMDGOALS)),)

ifeq ($(NUC972_UBOOT_SITE),)
$(error NO NUC972 uboot custom source site set. Check your BR2_BOOT_NUC972_UBOOT_CUSTOM_LOCAL_PATH setting)
endif

endif

endif
