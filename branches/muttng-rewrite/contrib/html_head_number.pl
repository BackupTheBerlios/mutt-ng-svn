#!/usr/bin/perl -w

# Description:
# trivial perl script taking a HTML filename as arg and properly
# numbering headlines according to <h[x]>...</h[x]> tags; already
# existing numbers are ignored so that it's only useful for
# postprocessing if you have some kind of build process
#
# If you need unnumbered headlines, apply class nonumber as in:
#   <h1 class="nonumber">Contents</h1>
#
# Changes:
# · usage: find . -name '*.html' -print0 | /path/to/html_head...
# · class="nonumber" is ignored while all other attributes in <hX> are
#   kept (e.g. <h1 class="blink">...</h1>)
# · s/$traildot/$trail/, add $trailseq
#
# $Id$

# This file is published as public domain.

# if $trail==1 => the sequence in $tailseq is appended (e.g. a dot)
my $trail = 1;
my $trailseq = '.&nbsp;';

while (<STDIN>) {
  chomp;
  my $filename = $_;
  my @no = (0, 0, 0, 0, 0, 0);
  my $curlevel = 0;
  my $oldlevel = 0;
  my @file = ();

  open (INPUT, "<".$filename) or 
    die "Couldn't open ".$filename." for reading: ".$!."\n";
  while (<INPUT>) {
    if ($_ =~ /<h([2-6])([^>]*)>/) {

      if ($2 =~ /class="nonumber"/) {
        push (@file, $_);
        next;
      }
      
      $curlevel = $1-1;
      my $info = $2;
      
      my $nostr = "";
      if ($curlevel < $oldlevel) {
        for (my $i = $curlevel+2; $i <= 6; $i++) {
          $no[$i] = 0;
        }
      }
      $no[$curlevel + 1]++;
      for (@no) {
        if ($_ != 0) {
          $nostr .= $_.".";
        }
      }
      # first: remove trailing dot
      $nostr =~ s/\.$//;
      # second append $trailseq (...which then may be a dot...)
      if ($trail) {
        $nostr .= $trailseq;
      }
      $_ =~ s/(<h[2-6]$info>)/$1$nostr /;
      $oldlevel = $curlevel;
    }
    push (@file, $_);
  }
  close (INPUT);

  open (OUTPUT, ">".$filename) or
    die "Couldn't open ".$filename." for writing: ".$!."\n";
  print OUTPUT @file;
  close (OUTPUT);
}
