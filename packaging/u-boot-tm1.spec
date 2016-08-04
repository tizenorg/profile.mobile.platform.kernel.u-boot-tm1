Name: u-boot-sprd
Summary: Bootloader for Embedded boards based on ARM processor
Version: 2010.12
Release: 0
Group: System/Kernel
License: GPL-2.0+
ExclusiveArch: %{arm}
URL: http://www.denx.de/wiki/U-Boot
Source0: %{name}-%{version}.tar.bz2
Source1001: packaging/u-boot-tm1.manifest 

%if "%{tizen_target_name}" != "TM1"
ExcludeArch: %{arm}
%endif

%description
bootloader for Embedded boards based on ARM processor

%package -n u-boot-tm1
Summary: A bootloader for Embedded system
Group: System/Kernel

%description -n u-boot-tm1
A boot loader for embedded systems.
Das U-Boot is a cross-platform bootloader for embedded systems,
used as the default boot loader by several board vendors.  It is
intended to be easy to port and to debug, and runs on many
supported architectures, including PPC, ARM, MIPS, x86, m68k, NIOS,
and Microblaze.

%ifarch %{arm}
%global use_mmc_storage 1
%endif

%prep
%setup -q

%build
cp %{SOURCE1001} .
make distclean
make tizen_tm1_config

make %{?_smp_mflags} HOSTCC="gcc $RPM_OPT_FLAGS" HOSTSTRIP=/bin/true tools

%if 1%{?use_mmc_storage}
make %{?_smp_mflags} HOSTCC="gcc $RPM_OPT_FLAGS" CONFIG_ENV_IS_IN_MMC=y env
%else
make %{?_smp_mflags} HOSTCC="gcc $RPM_OPT_FLAGS" env
%endif

export PATH="$PATH:tools"
make %{?_smp_mflags} EXTRAVERSION=`echo %{vcs} | sed 's/.*u-boot.*#\(.\{9\}\).*/-g\1-TIZEN.org/'`

# Sign u-boot-multi.bin - output is: u-boot-mmc.bin
chmod 755 tools/mkimage_signed.sh
mkimage_signed.sh u-boot.bin "tizen_tm1"

%install
rm -rf %{buildroot}

# u-boot installation
mkdir -p %{buildroot}/boot/u-boot
install -d %{buildroot}/boot/u-boot
install -m 755 u-boot.bin %{buildroot}/boot/u-boot
install -m 755 u-boot-mmc.bin %{buildroot}/boot/u-boot

%clean

%files -n u-boot-tm1
%manifest u-boot-tm1.manifest
%defattr(-,root,root,-)
/boot/u-boot
