include Makefile.include
DEPS= Makefile.include Makefile samba30-3.0.37-configure.in.patch \
      samba30-3.0.37-CVE-2010-2063.patch \
      samba30-3.0.37-silence-receive-warning.patch \
      samba30-3.0.37-vfs_default.c.patch samba30-3.0.37-wle-fix.patch \
      samba_android.patch

# lib name, version
LIBNAME=samba
VERSION=3.0.37
SOURCE=$(LIBNAME)-$(VERSION)
ARCHIVE=$(SOURCE).tar.gz

# configuration settings
# android does not really support LFS but we can force it
# in libsmbclient by defining off_t to off64_t and changing # libsmbclient.h to match. Defining _OFF_T_DEFINED_ allows
# us to change the typedef of off_t in android's sys/type.h.
# Then xbmc will correctly access smb shares with LFS enabled
# using the altered libsmbclient.h.
ifeq ($(OS),android)
export CFLAGS+=-D_FILE_OFFSET_BITS=64 -D_OFF_T_DEFINED_ -Doff_t=off64_t -Dlseek=lseek64
ANDROID_OFF_T_FIX=sed -ie "s|off_t|off64_t|g" "$(PLATFORM)/source/include/libsmbclient.h"
endif

CONFIGURE= ./configure --prefix=$(PREFIX)\
		--host=arm-linux-androideabi \
        --disable-cups \
        --enable-static \
        --disable-shared \
        --disable-pie \
        --disable-iprint \
        --disable-fam \
        --without-cluster-support \
        --without-cifsupcall \
        --without-ads \
        --without-ldap \
        --with-included-popt \
        --with-included-iniparser \
        --without-sys-quotas \
        --without-krb5 \
        --without-automount \
        --without-sendfile-support \
        --with-libiconv=$(STAGING_DIR) \
        --without-cifsmount \
        --without-winbind \

LIBDYLIB=$(PLATFORM)/source/bin/libsmbclient.a

CLEAN_FILES=$(ARCHIVE) $(PLATFORM)

all: .installed

.downloaded:
	if [ ! -f $(ARCHIVE) ] ;then wget $(BASE_URL)/$(ARCHIVE); fi
	touch $@

.patched: .downloaded
	rm -rf $(PLATFORM); mkdir -p $(PLATFORM)
	$(ARCHIVE_TOOL) $(ARCHIVE_TOOL_FLAGS) $(ARCHIVE) -C $(PLATFORM)
	cd $(PLATFORM); patch -p1 < ../samba30-3.0.37-configure.in.patch
	cd $(PLATFORM); patch -p1 < ../samba30-3.0.37-CVE-2010-2063.patch
	cd $(PLATFORM); patch -p1 < ../samba30-3.0.37-silence-receive-warning.patch
	cd $(PLATFORM); patch -p1 < ../samba30-3.0.37-vfs_default.c.patch
	cd $(PLATFORM); patch -p1 < ../samba30-3.0.37-wle-fix.patch
	cd $(PLATFORM); patch -p0 < ../samba_android.patch
	$(ANDROID_OFF_T_FIX)
	touch $@

.configured: .patched
	cd $(PLATFORM)/source && ./autogen.sh
	cp config.* $(PLATFORM)/source; cd $(PLATFORM)/source; $(CONFIGURE)
	touch $@

$(LIBDYLIB): .configured
	$(MAKE) -j16 -C $(PLATFORM)/source headers
	mkdir -p $(PLATFORM)/source/bin
	$(MAKE) -j16 -C $(PLATFORM)/source libsmbclient
	$(MAKE) -j16 -C $(PLATFORM)/source libaddns

.installed: $(LIBDYLIB)
	find -name *.a -exec cp {} ../android/ \;
	touch $@

clean:
	$(MAKE) -C $(PLATFORM)/source clean
	rm -f .installed

distclean::
	rm -rf $(PLATFORM) .installed


cmdtest:
	@echo $(CURDIR)
