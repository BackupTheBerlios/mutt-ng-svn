Summary: A text mode mail user agent.
Name: muttng
Version: 20050306
Release: 1
Serial: 1
License: GPL
Group: Applications/Internet
Source: http://mutt-ng.berlios.de/snapshots/%{name}-%{version}.tar.gz
Url: http://mutt-ng.berlios.de/
Requires: slang >= 0.99.38, smtpdaemon, webclient, mailcap, gettext
Buildroot: %{_tmppath}/%{name}-%{version}-%{release}-root
%{!?nossl:BuildPrereq: openssl-devel}
%{!?nokerberos:BuildPrereq: krb5-devel}
BuildPrereq: /usr/sbin/sendmail slang-devel /usr/bin/automake-1.4

%description
Mutt-ng is a text-mode mail user agent. Mutt-ng supports color, threading,
arbitrary key remapping, and a lot of customization.

You should install Mutt-ng if you have used it in the past and you prefer
it, or if you are new to mail programs and have not decided which one
you are going to use.

%prep
# %setup -n %{name}-%{version} -q -a 2
%setup -n %{name}-%{version} -q

%build
autoconf ./configure.in > ./configure
chmod 700 ./configure
export -n LINGUAS
CFLAGS="$RPM_OPT_FLAGS" ./configure --prefix=%{_prefix} \
	--with-sharedir=/etc --sysconfdir=/etc \
	--with-docdir=%{_docdir}/mutt-%{version} \
	--with-mandir=%{_mandir} \
	--with-infodir=%{_infodir} \
	--enable-pop --enable-imap \
	--with-sasl \
%{!?nossl:--with-ssl} \
%{!?nokerberos:--with-gss=/usr/kerberos} \
	--disable-warnings --with-slang --disable-domain \
	--disable-flock --enable-fcntl \
	--enable-buffy-size
make

%install
rm -rf $RPM_BUILD_ROOT
%makeinstall sharedir=$RPM_BUILD_ROOT/etc \
  sysconfdir=$RPM_BUILD_ROOT/etc \
  docdir=$RPM_BUILD_ROOT%{_docdir}/%{name}-%{version} \
  install
mkdir -p $RPM_BUILD_ROOT/etc/X11/applnk/Internet

# we like GPG here
cat contrib/gpg.rc >> \
	$RPM_BUILD_ROOT/etc/Muttngrc
grep -5 "^color" contrib/sample.muttrc >> \
	$RPM_BUILD_ROOT/etc/Muttngrc
# and we use aspell

cat >> $RPM_BUILD_ROOT/etc/Muttngrc <<EOF
# use aspell
set ispell="/usr/bin/aspell --mode=email check"

EOF

%find_lang %{name}

%clean
rm -rf $RPM_BUILD_ROOT

%files -f %{name}.lang
%defattr(-,root,root)
%config /etc/Muttngrc
%config /etc/muttng-mime.types
%doc ABOUT-NLS BEWARE ChangeLog* COPYRIGHT GPL INSTALL NEWS README* TODO* UPGRADING 
%doc doc/applying-patches.txt doc/*-notes.txt doc/PGP-Notes.txt doc/manual* contrib/* 
%{_bindir}/muttng
%{_bindir}/muttngbug
%attr(2755,root,mail) %{_bindir}/muttng_dotlock
%{_bindir}/fleang
%{_bindir}/pgpringng
%{_bindir}/pgpewrapng
%{_bindir}/smime_keysng
%{_mandir}/man1/muttng.*
%{_mandir}/man1/muttngbug.*
%{_mandir}/man1/muttng_dotlock.*
%{_mandir}/man1/muttng-flea.*
%{_mandir}/man1/fleang.*
%{_mandir}/man5/muttngrc.*
%{_mandir}/man5/muttng-mbox.*

%changelog
* Sun Mar 06 2005 Iain Lea <iain@bricbrac.de>  1:20050306-1
- initial muttng.spec file
