Summary:        ats-analyzer backend
Name:           ats-analyzer
Version:        0.1.9
Release:        1%{?dist}
License:        Proprietary
Group:          Applications/Multimedia
URL:            http://http://www.niitv.ru/
Source0:        %{name}-%{version}.tar.gz
BuildRoot:      %{_tmppath}/%{name}

BuildRequires:  gstreamer1-devel >= 1.6.0
BuildRequires:  gstreamer1-plugins-bad-free-devel

%description

%prep

%setup -q -c -T -a 0

%build
ls
cd ./%{name}
make
make clean

%install
%{__mkdir_p} '%{buildroot}%{_bindir}'
%{__mkdir_p} '%{buildroot}/usr/lib64/gstreamer-1.0'

cp ./%{name}/build/ats3-backend %{buildroot}%{_bindir}
cp ./%{name}/build/libvideoanalysis.so %{buildroot}%{_libdir}/gstreamer-1.0
cp ./%{name}/build/libaudioanalysis.so %{buildroot}%{_libdir}/gstreamer-1.0

%files
%{_bindir}/ats3-backend
%{_libdir}/gstreamer-1.0/libvideoanalysis.so
%{_libdir}/gstreamer-1.0/libaudioanalysis.so

%changelog

