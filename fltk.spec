#
# "$Id: fltk.spec,v 1.1.2.9 2001/05/07 14:51:10 easysw Exp $"
#
# RPM spec file for FLTK.
#
# Copyright 1998-2001 by Bill Spitzak and others.
#
# This library is free software; you can redistribute it and/or
# modify it under the terms of the GNU Library General Public
# License as published by the Free Software Foundation; either
# version 2 of the License, or (at your option) any later version.
#
# This library is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
# Library General Public License for more details.
#
# You should have received a copy of the GNU Library General Public
# License along with this library; if not, write to the Free Software
# Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307
# USA.
#
# Please report all bugs and problems to "fltk-bugs@fltk.org".
#

%define version 1.0.11
%define release 1
%define prefix /usr

Summary: Fast Light Tool Kit (FLTK)
Name: fltk
Version: %{version}
Release: %{release}
Copyright: LGPL
Group: System Environment/Libraries
Source: ftp://ftp.fltk.org/pub/fltk/%{version}/fltk-%{version}-source.tar.gz
URL: http://www.fltk.org
Packager: Michael Sweet <mike@easysw.com>
# use BuildRoot so as not to disturb the version already installed
BuildRoot: /var/tmp/fltk-%{PACKAGE_VERSION}

%description
The Fast Light Tool Kit ("FLTK", pronounced "fulltick") is a LGPL'd
C++ graphical user interface toolkit for X (UNIX(r)), OpenGL(r),
and Microsoft(r) Windows(r) NT 4.0, 95, or 98. It was originally
developed by Mr. Bill Spitzak and is currently maintained by a
small group of developers across the world with a central
repository in the US.

%package devel
Summary: FLTK - development environment
Group: Development/Libraries

%description devel
Install fltk-devel if you need to develop FLTK applications.  You'll
need to install the fltk package if you plan to run dynamically linked
applications.

%prep
%setup

%build
CFLAGS="$RPM_OPT_FLAGS" CXXFLAGS="$RPM_OPT_FLAGS" LDFLAGS="$RPM_OPT_FLAGS" ./configure --prefix=%{prefix} --enable-shared

# If we got this far, all prerequisite libraries must be here.
make

%install
# these lines just make sure the directory structure in the
# RPM_BUILD_ROOT exists
rm -rf $RPM_BUILD_ROOT
mkdir -p $RPM_BUILD_ROOT

make -e prefix=$RPM_BUILD_ROOT/%{prefix} install

%clean
rm -rf $RPM_BUILD_ROOT

%files
%defattr(-,root,root)
%{prefix}/lib/libfltk*.so.*

%files devel
%defattr(-,root,root)
%{prefix}/bin/fluid
%{prefix}/include/FL
%{prefix}/include/Fl
%{prefix}/lib/libfltk*.so
%{prefix}/lib/libfltk*.a
%{prefix}/man/*
%{prefix}/share/doc/fltk/*

#
# End of "$Id: fltk.spec,v 1.1.2.9 2001/05/07 14:51:10 easysw Exp $".
#
