include $(TOPDIR)/rules.mk

PKG_NAME:=dhcpext
PKG_RELEASE:=1

PKG_BUILD_DIR:=$(BUILD_DIR)/$(PKG_NAME)

include $(INCLUDE_DIR)/package.mk
include $(INCLUDE_DIR)/cmake.mk

define Package/dhcpext
	SECTION:=utils
	CATEGORY:=Utilities
	DEPENDS:=+libpthread
	TITLE:=dhcpext
endef

define Package/dhcpext/description
	Mesh net DHCP extensions.
endef

define Build/Prepare
	mkdir -p $(PKG_BUILD_DIR)
	$(CP) ./src/* $(PKG_BUILD_DIR)/
endef

define Package/dhcpext/install
	$(INSTALL_DIR) $(1)/bin
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/dhcpext $(1)/bin/
endef

$(eval $(call BuildPackage,dhcpext))

