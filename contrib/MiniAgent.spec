### 1.The introduction section
Name:           MiniAgent
Version:        %{?pkg_version}
Release:        %{?pkg_release}
Summary:        Just a RPM example
License:        GPLv2+ and BSD
BuildRequires:  gcc,make
Source0:        %{name}-%{version}-%{release}.tar.bz2
BuildRoot:      %_topdir/BUILDROOT

%description
It is a RPM example.

#--- 2.The Prep section
%prep
%setup -q -n %{name}-%{version}-%{release}
#%patch0 -p1

#--- 3.The Build Section
%build
mkdir build && cd build && cmake .. -DPROJECT_COMMIT_ID="%{commitid}"
make %{?_smp_mflags}
#echo %{_sysconfdir}

%install
#/usr/bin/
install -d -m 0751 %{buildroot}/%{_sbindir}
cd build && make install DESTDIR=%{buildroot}
#--- 4.1 scripts section
%pre
#echo "pre" >> /tmp/foobar.txt
%post
#echo "post" >> /tmp/foobar.txt
%preun
#echo "preun" >> /tmp/foobar.txt
%postun
#echo "postun" >> /tmp/foobar.txt

#--- 5.clean section
%clean
rm -rf %{buildroot}

#--- 6.file section
%files
%defattr(-,root,root,-)
%attr(755, root, root) %{_bindir}/MiniAgent

#--- 7.chagelog section
%changelog
* Fri Dec 28 2012 foobar foobar@kidding.com - 1.0.0-1
- Initial version
