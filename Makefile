#
# This software is licensed under the Public Domain.
#

include $(TOPDIR)/rules.mk

PKG_NAME:=lcd-loblik
PKG_VERSION:=0.1
PKG_RELEASE:=1

PKG_BUILD_DIR := $(BUILD_DIR)/$(PKG_NAME)

include $(INCLUDE_DIR)/package.mk

define Package/lcd-loblik
	SECTION:=utils
	CATEGORY:=Utilities
	TITLE:=lcd-loblik
	URL:=https://bitbucket.org/loblik/lcd.git
endef

define Package/lcd-loblik/description
	lcd-lobliK lcd hd44780 i2c
endef


define Build/Prepare
	mkdir -p $(PKG_BUILD_DIR)
	$(CP) ./src/* $(PKG_BUILD_DIR)/
endef

define Build/Configure
# Nothing to do here for us.
# By default lcd-loblik/src/Makefile will be used.
endef

define Build/Compile
	CFLAGS="$(TARGET_CFLAGS)" CPPFLAGS="$(TARGET_CPPFLAGS)" $(MAKE) -C $(PKG_BUILD_DIR) $(TARGET_CONFIGURE_OPTS)
endef

define Package/lcd-loblik/install
	$(INSTALL_DIR) $(1)/usr/lib
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/lcd.so $(1)/usr/lib/
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/lcd.a $(1)/usr/lib/
	$(INSTALL_DIR) $(1)/usr/sbin
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/lct $(1)/usr/sbin/
endef

$(eval $(call BuildPackage,lcd-loblik))
