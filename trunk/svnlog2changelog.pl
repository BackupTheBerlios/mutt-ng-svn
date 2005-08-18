#!/usr/bin/perl -w

# Purpose:
# summarize and re-format subversion's log output to plain text
#
# Written by Rocco Rutte <pdmef@cs.tu-berlin.de>
# for internal use with mutt-ng <http://mutt-ng.berlios.de/>.
#
# License: GPL
#
# Usage/Options:
#       -t              print only today's messages
#       -s YYYY-MM-DD   print only messages since (and including) date
#       -i string       use string for indentation
#       -m number       break lines at the latest at number columns
#       -p string       prefix for filenames in 'svn log -v' output;
#                       for things like 'A /mutt-ng/trunk/foo' set this
#                       to 'mutt-ng/', i.e. exclude the leading / from
#                       it but include everything up to the last char
#                       before (trunk|tags|branches)
#       -h              help
#
# Note: lines matching ``^([^:]+):?$'' will be ignored if the first
# submatch in brackets is a known author; i.e. the logs
#
# | Foo Bar
# | added l33t c0d3
#
# and
#
# | Foo Bar:
# | added l33t c0d3
#
# will be interpreted as only 'added l33t c0d3' if 'Foo Bar' is known as
# an author. This does not count when grouping messages for authors,
# those lines are just skipped. This is hard-coded, see below.
#
# Also, put
# | From: ...
# as the first line of log messages when change is only committed by
# someone with access but the original author is someone else. If
# there's no such author line, the author's name will be grabbed from
# the %committers table below.

use strict;
use POSIX;
use Getopt::Std;

# hard-coded configuration: this maps user to full names
my %committers = (
  "ak1"         => "Andreas Krennmair <ak\@synflood.at>",
  "nion"        => "Nico Golde <nion\@muttng.org>",
  "pdmef"       => "Rocco Rutte <pdmef\@cs.tu-berlin.de>",
  "dkg1"        => "Daniel K. Gebhart <dpkg1\@users.berlios.de>"
);

my %fmap = (
  "filesA"      => "Added",
  "filesM"      => "Modified",
  "filesD"      => "Deleted"
);

# default config
my %options = ();
my $linemax = 70;
my $today = "";
my $since = "";
my $indent = "    ";
my $pfx = "";

# some stuff we need
my $currev = 0;
my $lastrev = 0;
my @curlog = ();
my $curentry = "";
my $curauthor = "";
my $curcomm = "";
my $count = 0;
my %changes = ();

# nicely print log lines in itemized style with eye-candy indentation and
# somewhat smart line-breaking
sub niceline {
  my ($text) = (@_);
  my @lines = split (/\n/, $text);
  for my $l (@lines) {
    print "$indent$indent-";
    my @words = split (/\ /, $l);
    my $c = length ($indent);
    for my $w (@words) {
      if (length ($w) + $c > $linemax) {
        print "\n$indent$indent  $w";
        $c = length ($indent);
      } else {
        print " $w";
      }
      $c += length ($w) + 1;
    }
    print "\n";
  }
}

sub usage {
  print <<EOF
This is: svnlog2changelog.pl
written by Rocco Rutte <pdmef\@cs.tu-berlin.de>
for use with mutt-ng <http://mutt-ng.berlios.de/>

Usage:
  svnlog2changelog.pl -h
  svn log -v | svnlog2changelog.pl [-t] [-i YYYY-MM-DD] [-m number] [-p string]

Options:
       -t              print only today's messages
       -s YYYY-MM-DD   print only messages since (and including) date
       -i string       use string for indentation
       -m number       break lines at the latest at number columns
       -p string       prefix for filenames in 'svn log -v' output;
                       for things like 'A /mutt-ng/trunk/foo' set this
                       to 'mutt-ng/', i.e. exclude the leading / from
                       it but include everything up to the last char
                       before (trunk|tags|branches)
       -h              help

Examples:
  - print Subversion's log for today:
    svn log -v -r "{`date "+%Y-%m-%d"`}:HEAD" | svnlog2changelog.pl -t [-i string]

  - print Subversion's log since (and including) YYYY-MM-DD
    svn log -v | svnlog2changelog.pl -s YYYY-MM-DD [-i string]
EOF
  ;
}

sub isknown {
  my ($name) = (@_);
  for my $k (keys %committers) {
    if (substr ($committers{$k}, 0, length ($name)) eq $name) {
      return (1);
    }
  }
  return (0);
}

# get and process options
getopts ("tm:s:hi:p:", \%options);
if (defined $options{'t'}) {
  $today = strftime ("%Y-%m-%d", localtime (time ()));
}
if (defined $options{'m'} and $options{'m'} =~ /^[0-9]{2,}$/) {
  $linemax = $options{'m'};
}
if (defined $options{'s'} and $options{'s'} =~ /^[0-9]{4}-[0-9]{2}-[0-9]{2}$/) {
  $since = $options{'s'};
}
if (defined $options{'h'}) {
  &usage ();
  exit (0);
}
if (defined $options{'i'}) {
  $indent = $options{'i'};
}
if (defined $options{'p'}) {
  $pfx = $options{'p'};
  $pfx =~ s#/#\\/#g;
}

# parse log
while (<STDIN>) {
  chomp;
  if ($_ =~ /^r([0-9]+)/) {
    $currev = $1;
    @curlog = ();
    $curauthor = "";
    $count = 0;
    my @items = split (/\ \|\ /, $_);
    my @dateinfo = split (/\ /, $items[2]);
    $curentry = $dateinfo[0];
    $curauthor = $items[1];
    $curcomm = $items[1];
    # _keep_ latest rev. number for day
    if (not defined ${$changes{$curentry}}{'rev'}) {
      ${$changes{$curentry}}{'rev'} = substr ($items[0], 1);
    }
    # _keep_ latest commit time for day
    if (not defined ${$changes{$curentry}}{'time'}) {
      ${$changes{$curentry}}{'time'} = "$dateinfo[1] $dateinfo[2]";
    }
    next;
  }
  $count++;
  if ($count > 0) {
    # check log line: contains author?
    if ($_ =~ /^From: (.*)$/) {
      $curauthor = "$1 ($curcomm)";
      next;
    }
    elsif (defined $committers{$curauthor}) {
      $curauthor = "$committers{$curcomm} ($curcomm)";
    }
    # check log line: contains noise?
    if (length ($_) == 0 or $_ =~ /^[-]+$/ or 
        $_ =~ /^([^:]+):?$/ and &isknown ($1) or
        $_ eq "Changed paths:") {
      next;
    }
    # check log line: contains list of changes/deleted/added files?
    if ((length ($pfx) > 0 and $_ =~ /([AMD]) \/($pfx?.*)?$/) or
        (length ($pfx) == 0 and $_ =~ /([AMD]) \/(.*)?$/)) {
      my $what = $1;
      my $target = "";
      if (defined $2) {
        $target = $2;
      }
      $target =~ s#$pfx##g;
      ${${$changes{$curentry}}{"files$what"}}{$target} = 1;
    } else {
      # here the line really contains the log message
      # try to be smart and remove itemizations people make
      my $clean = $_;
      $clean =~ s/^[- \t*]*//;
      if (length ($clean) > 0) {
        ${${$changes{$curentry}}{'log'}}{$curauthor} .= "$clean (r$currev)\n";
        $lastrev = $currev;
      }
    }
  }
}

my $first = "";
my $first2 = "";

for my $k (sort { $b cmp $a } (keys (%changes))) {
  # ignore noise
  if (not defined %{${$changes{$k}}{'log'}} or
      (length ($since) > 0 && length ($today) == 0 && ($k lt $since)) or
      (length ($today) > 0 && ($k ne $today))) {
    next;
  }
  # print first line with date, time and latest revision for current day
  print "$first$k  ${$changes{$k}}{'time'}  ";
  $first = "\n";
  $first2 = "";
  print "Latest Revision: ${$changes{$k}}{'rev'}\n\n";
  # per author: print his name and an itemized list of his log msgs.
  # with smart line-breaking and indentation
  for my $a (keys %{${$changes{$k}}{'log'}}) {
    print "$first2$indent";
    if (defined $committers{$a}) {
      print $committers{$a};
    } else {
      print $a;
    }
    print ":\n";
    &niceline (${${$changes{$k}}{'log'}}{$a});
    $first2 = "\n";
  }
  for my $a (keys %fmap) {
    if (defined %{${$changes{$k}}{$a}}) {
      print "$first$indent$fmap{$a} Files:\n";
      my $fixme = join (", ", keys %{${$changes{$k}}{$a}});
      &niceline ($fixme);
    }
  }
}
