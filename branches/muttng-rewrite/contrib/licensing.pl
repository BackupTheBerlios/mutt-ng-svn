#!/usr/bin/perl -w

use strict;

# This file is published as public domain.

my $magic1 = ' This file is published as ([^.]+)\.';
my $magic2 = ' This file is published under the ([^.]+)\.';
my %table = ();

while (<>) {
  chomp;
  my $file=$_;
  my $done=0;
  if (open(INPUT,"<".$file)) {
    while (<INPUT>) {
      if ($done==0 and ($_ =~ /$magic1/ or $_ =~ /$magic2/)) {
        $table{$1} .= " ".$file;
        $done=1;
      }
    }
    close(INPUT);
  }
}

print "<?xml version=\"1.0\" encoding=\"utf-8\"?>\n".
      "<chapter id=\"licensing\">\n\n".
      "  <title>Licensing</title>\n".
      "    <ol>\n";
for my $l (sort keys %table) {
  print "  <li><b>$l</b>:";
  my $w=100;
  for my $f (split(/\ /,$table{$l})) {
    next if (length($f)==0);
    $w+=length($f)+9;
    if ($w>80) {
      $w=0;
      print "\n      ";
    }
    $f=~s#^./../##;
    print "<tt>$f</tt> ";
  }
  print "\n    </li>\n";
}
print "\n  </ol>\n".
      "</chapter>\n";
