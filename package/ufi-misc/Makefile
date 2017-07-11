# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.

include $(TOPDIR)/rules.mk

PKG_NAME:=ufi-misc
PKG_RELEASE:=1

include $(INCLUDE_DIR)/package.mk

define Package/ufi-misc
  SUBMENU:=ufi-misc
  TITLE:=UUWIFi misc tools
  DEPENDS:=@TARGET_ramips_mt7628 \
  	+coreutils-timeout \
  	+coreutils-sleep
  CATEGORY:=Base system
endef

define Build/Prepare
	mkdir -p $(PKG_BUILD_DIR)
endef

define Build/Compile
	$(TARGET_CC) $(TARGET_CFLAGS) $(EXTRA_CFLAGS) -Wall -Werror -o $(PKG_BUILD_DIR)/gpio src/gpio.c ; \
	$(TARGET_CC) $(TARGET_CFLAGS) $(EXTRA_CFLAGS) -Wall -Werror -o $(PKG_BUILD_DIR)/pinmux src/pinmux.c ; \
	$(TARGET_CC) $(TARGET_CFLAGS) $(EXTRA_CFLAGS) -Wall -Werror -o $(PKG_BUILD_DIR)/usleep src/usleep.c ; \
	$(TARGET_CC) $(TARGET_CFLAGS) $(EXTRA_CFLAGS) -Wall -Werror -o $(PKG_BUILD_DIR)/refclk src/refclk.c
endef

define Package/ufi-misc/install
	$(INSTALL_DIR) $(1)/opt/ejoin_fs/bin
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/pinmux $(1)/opt/ejoin_fs/bin/pinmux
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/refclk $(1)/opt/ejoin_fs/bin/refclk
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/usleep $(1)/opt/ejoin_fs/bin/usleep
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/gpio $(1)/opt/ejoin_fs/bin/gpio
endef

$(eval $(call BuildPackage,ufi-misc))
