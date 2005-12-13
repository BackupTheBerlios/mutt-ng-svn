#!/usr/bin/perl -w

use Getopt::Long;

my %features = ( # {{{

  # misc

  "make"                => { help =>    "GNU make command ('make' on Linux, 'gmake' on *BSD/SunOs)",
                             value =>   "make",
                             fmt =>     "=s" },

  "unitpp"              => { conf =>    "UNITPPDIR",
                             text =>    "Whether to use Unit++ for unit tests",
                             help =>    "Where Unit++ is installed",
                             value =>   "",
                             fmt =>     "=s"
                            },

  "ccstyle"             => { conf =>    "CCSTYLE",
                             text =>    "Which C/C++ compilers to use".
                                        " ('gcc' for GCC, 'suncc' for Sun WSPro)",
                             help =>    "C/C++ compiler flavor",
                             value =>   "gcc",
                             fmt =>     "=s"
                            },

  "ccdocs"              => { conf =>    "CCDOCS",
                             text =>    "Where GCC C++ includes are located for documentation",
                             help =>    "GCC C++ include dir",
                             value =>   "",
                             fmt =>     "=s"
                            },

  "doc-lang"            => { conf =>    "DOCLANGUAGES",
                             text =>    "In which languages to build documentation",
                             help =>    "Which languages to build docs in",
                             value =>   "all",
                             fmt =>     "=s"
                            },

  ### core ###

  "with-gettext"        => { key =>     "CORE_INTL",
                             conf =>    "GETTEXTDIR",
                             text =>    "Whether to compile in NLS support",
                             help =>    "Where gettext for NLS is installed",
                             value =>   "",
                             fmt =>     "=s",
                             group =>   [ "core" ] },

  "with-iconv"          => { key =>     "CORE_LIBICONV",
                             conf =>    "LIBICONVDIR",
                             help =>    "Where libiconv is installed",
                             text =>    "Whether to use libiconv",
                             config =>  "LIBICONVDIR",
                             value =>   "",
                             fmt =>     "=s",
                             group =>   [ "core" ] },

  ### libmuttng ###
                             
  "enable-pop"          => { key =>     "LIBMUTTNG_POP3",
                             conf =>    "WANT_POP3",
                             help =>    "Whether to compile in POP3 support",
                             value =>   "",
                             group =>   [ "libmuttng" ] },
                             
  "enable-imap"         => { key =>     "LIBMUTTNG_IMAP",
                             conf =>    "WANT_IMAP",
                             help =>    "Whether to compile in IMAP support",
                             value =>   "",
                             group =>   [ "libmuttng" ] },

  "enable-nntp"         => { key =>     "LIBMUTTNG_NNTP",
                             conf =>    "WANT_NNTP",
                             help =>    "Whether to compile in NNTP support",
                             value =>   "",
                             group =>   [ "libmuttng" ] },


  "enable-smtp"         => { key =>     "LIBMUTTNG_SMTP",
                             conf =>    "WANT_SMTP",
                             help =>    "Whether to compile in SMTP support",
                             value =>   "",
                             group =>   [ "libmuttng" ] },

  "with-qdbm"           => { key =>     "LIBMUTTNG_CACHE_QDBM",
                             conf =>    "QDBMDIR",
                             text =>    "Whether to use QDBM as cache backend",
                             help =>    "Where qdbm is installed",
                             value =>   "",
                             fmt =>     "=s",
                             group =>   [ "libmuttng" ] },

  "with-openssl"        => { key =>     "LIBMUTTNG_SSL_OPENSSL",
                             conf =>    "OPENSSLDIR",
                             text =>    "Whether to use SSL via OpenSSL",
                             help =>    "Where OpenSSL is installed",
                             value =>   "",
                             fmt =>     "=s",
                             group =>   [ "libmuttng" ] },

  "with-gnutls"         => { key =>     "LIBMUTTNG_SSL_GNUTLS",
                             conf =>    "GNUTLSDIR",
                             text =>    "Whether to use SSL via GNUTLS",
                             help =>    "Where GNUTLS is installed",
                             value =>   "",
                             fmt =>     "=s",
                             group =>   [ "libmuttng" ] }

);
### }}}

my %tests = ( # {{{

  "core" => [
              {
                lang =>  "c",
                file =>  "test_alloca.c",
                msg =>   "working alloca()",
                def =>   "CORE_HAVE_ALLOCA",
                run =>   0
              },

              {
                lang =>  "c",
                file =>  "test_mmap.c",
                msg =>   "working mmap()",
                def =>   "CORE_HAVE_MMAP",
                run =>   0
              },

              {
                lang =>  "c",
                file =>  "test_strsep.c",
                msg =>   "working strsep()",
                def =>   "CORE_HAVE_STRSEP",
                run =>   0
              },

              {
                lang =>  "c",
                file =>  "test_c99_int.c",
                msg =>   "C99 integer types",
                def =>   "CORE_HAVE_C99_INTTYPES",
                run =>   1
              },

            ]
);
### }}}

my %options = ();

my @switches = ();
foreach my $k (keys %features) {
  if (defined $features{$k}{'fmt'}) {
    push (@switches, "$k$features{$k}{'fmt'}");
  } else {
    push (@switches, "$k");
  }
}
GetOptions (\%options, "help", @switches) or die;
foreach my $k (keys %options) {
  if ($k ne "help") {
    $features{$k}{'value'} = $options{$k};
  }
}

### dump help {{{

if (defined $options{"help"}) {
  print "".
"`configure' is a small perl-based replacement for autoconf.\n" .
"\n".
"Usage:\n".
"  ./configure --help           # get help\n".
"  ./configure [OPTIONS]        # get list of options\n".
"\n".
"Options:\n";
  foreach my $k (sort keys %features) {
    my $l = 25;
    print "  --$k";
    $l -= length("  --$k");
    if (defined $features{$k}{'fmt'}) {
      print "=value";
      $l -= length("=value");
    }
    print " "x$l;
    print "$features{$k}{'help'}";
    if (defined $features{$k}{'value'}) {
      print " (default: \"$features{$k}{'value'}\")";
    }
    print "\n";
  }
  exit 0;
}
### }}}

### normal run

### detect GNU make {{{
if (not defined $options{"make"}) {
  if (`make -v 2>&1` =~ /^GNU Make/) {
    $options{"make"} = "make";
  } elsif (`gmake -v 2>&1` =~ /^GNU Make/) {
    $options{"make"} = "gmake";
  } else {
    die "Failed to find GNU make: please specify --make=(make|gmake) option.\n";
  }
}
# }}}

my $basedir = $0;
$basedir =~ s#^(.+)/[^/]+$#$1#;
my $author = `perl -v 2>&1`;
$author =~ s/\n//g;
$author =~ s/^[\s]*This is perl, (v[^\s]+).*$/perl(1) $1/;

### dump GNUmakefile.config.mine {{{
open (OUT, ">$basedir/GNUmakefile.config.mine") or die "Cannot open $basedir/GNUmakefile.config.mine: $!\n";
print "Writing $basedir/GNUmakefile.config.mine...";
print OUT "# configuration makefile for muttng\nMAKE?=$options{make}\n";
foreach my $k (keys %features) {
  if (defined $features{$k}{'conf'}) {
    print OUT "# $features{$k}{'help'}\n";
    if (not defined $options{$k}) {
      print OUT "# $features{$k}{'conf'}=$features{$k}{'value'}\n";
    } else {
      print OUT "$features{$k}{'conf'}=$options{$k}\n";
    }
  }
}
### detect compilers and flags via environment
my @comp = ("CC", "CFLAGS", "CXX", "CXXFLAGS");
foreach my $k (@comp) {
  if (defined $ENV{$k}) {
    print OUT "$k=$ENV{$k}\n";
  }
}
close (OUT);
print "done\n";
### }}}

my %conffiles = ();
my @groups = ("core", "libmuttng", "muttng");

### exec make to get compilers and flags+compile/run tests {{{
my %run = ();
chomp ($run{"c"} = `$options{"make"} -f $basedir/GNUmakefile.sysconf get_c 2>&1`) or die "Don't know how to run CC: $!\n";
chomp ($run{"cpp"} = `$options{"make"} -f $basedir/GNUmakefile.sysconf get_cpp 2>&1`) or die "Don't know how to run CXX: $!\n";

sub compile ($$$$) {
  my ($cmd, $file, $msg, $def) = (@_);
  print "Testing for $msg...";
  `$cmd $basedir/src/sysconf/$file >/dev/null 2>&1`;
  my $succ = $?>>8;
  print ($succ == 0 ? "yes\n" : "no\n");
  return $succ==0;
}

foreach my $k (keys %tests) {
  foreach my $t (@{$tests{$k}}) {
    my $succ = &compile ($run{$$t{"lang"}}, $$t{"file"}, $$t{"msg"}, $$t{"def"});
    $conffiles{$k} .= "/** $$t{'msg'} */\n#define $$t{'def'} $succ\n";
    if ($$t{"run"}) {
      $conffiles{$k} .= `./a.out` or die "Failed to run test: $!\n";
    }
  }
}
unlink ("./a.out");
### }}}

### construct rest of compile-time files from command line {{{
foreach my $k (keys %features) {
  if (defined $features{$k}{'key'}) {
    foreach my $g (@{$features{$k}{'group'}}) {
      if (defined $features{$k}{'text'}) {
        $conffiles{$g} .= "/** $features{$k}{'text'} */\n";
      } else {
        $conffiles{$g} .= "/** $features{$k}{'help'} */\n";
      }
      if (defined $options{$k}) {
        $conffiles{$g} .= "#define $features{$k}{'key'} 1\n";
      } else {
        $conffiles{$g} .= "/* #undef $features{$k}{'key'} */\n";
      }
    }
  }
}
### }}}

### sepcial treatment {{{

my $iconv_const = "/**/";
if (defined $options{"with-iconv"}) {
  if (open (IN, "<$features{'with-iconv'}{'value'}/include/iconv.h")) {
    while (<IN>) {
      chomp;
      if ($_ =~ / iconv[\s]*\(iconv_t/) {
        my @f = split(/,/,$_);
        $iconv_const = $f[1];
        $iconv_const =~ s/ char.*//;
        $iconv_const =~ s/^[\s]*//;
      }
    }
    close (IN);
  }
  $conffiles{"core"} .= "/** modifier for inbuf parameter of iconv() */\n";
  $conffiles{"core"} .= "#define CORE_ICONV_CONST $iconv_const\n";
}
  
### }}}
  
### dump compile-time files {{{
foreach my $g (keys %conffiles) {
  open (OUT, ">$basedir/src/${g}/${g}_features.h") or die "Cannot open $basedir/src/${g}/${g}_features.h $!\n";
  print "Writing $basedir/src/${g}/${g}_features.h...";
  print OUT "/**\n";
  print OUT " * \@file ${g}/${g}_features.h\n";
  print OUT " * \@author $author ;-)\n";
  print OUT " * \@brief (AUTO) Compile-time features for \"${g}\"\n";
  print OUT " */\n";
  print OUT "#ifndef \U${g}\E_\U${g}\E_FEATURES_H\n";
  print OUT "#define \U${g}\E_\U${g}\E_FEATURES_H\n\n";
  print OUT $conffiles{$g};
  print OUT "\n#endif /* !\U${g}\E_\U${g}\E_FEATURES_H */\n";
  close (OUT);
  print "done\n";
}
### }}}

print "\nThe source code has been successfully prepared.\n";
print "Run \"$options{'make'} depend && $options{'make'}\" for compilation.\n";
