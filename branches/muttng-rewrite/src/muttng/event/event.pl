#!/usr/bin/perl -w

my $first = 1;

# parse ./CONTEXTS {{{
my $context_enum = "";
my $context_descr = "";
open (IN, "< ./CONTEXTS") or die "cannot open ./CONTEXTS: $!\n";
while (<IN>) {
  chomp;
  if ($_ =~ /^(C_[A-Z]+)[\s\t]+([A-Za-z]+)[\s\t]*$/) {
    if (!$first) {
      $context_enum .= "\n";
      $context_descr .= "\n";
    }
    $context_enum .= "      /** context ID for \"$2\" */\n";
    $context_enum .= "      ".$1;
    if ($first == 1) {
      $first = 0;
      $context_enum .= " = 0";
    }
    $context_enum .= ",";
    $context_descr .= "  /** context string for $1 */\n";
    $context_descr .= "  \"$2\",";
  }
}
close (IN);
$first = 1;
# }}}

# parse ./EVENTS {{{
my @events = ();
my $event_enum = "";
my $event_descr = "";
my $event_valid = "";
open (IN, "< ./EVENTS") or die "cannot open ./EVENTS: $!\n";
while (<IN>) {
  chomp;
  if ($_ =~ /^([A-Z|_]+)[\s\t]+(E_[A-Z_]+)[\s\t]+([^\s]+)[\s\t]+([^\s]+)[\s\t]+([^\s]+)[\s\t]+(.+)$/) {
    #warn "ctx='$1', ev='$2', func='$3', cat='$4', key='$5', descr='$6'\n";
    if (!$first) {
      $event_enum .= "\n";
      $event_descr .= "\n";
      $event_valid .= "\n";
    }
    $event_enum .= "      /** event ID for <code>\&lt;$3\&gt;</code> */\n";
    $event_enum .= "      $2";
    $event_descr .= "  /** event string for $2 */\n";
    $event_descr .= "  \"$2\",";
    if ($first == 1) {
      $first = 0;
      $event_enum .= " = 0";
    }
    $event_enum .= ",";
    my @ctx = split (/\|/, $1);
    $event_valid .= "  /** for which contexts $2 is valid */\n";
    my $delim = "  ";
    foreach my $c (@ctx) {
      $event_valid .= $delim."CTX(Event::$c)";
      $delim = " | ";
    }
    $event_valid .= ",";
  }
}
close (IN);
$first = 1;
# }}}

open (IN, "< event.h.in") or die "cannot open event.in.h: $!\n";
$content = "";
while (<IN>) { $content .= $_; }
close (IN);
$content =~ s#__CONTEXT_ENUM__#$context_enum#g;
$content =~ s#__EVENT_ENUM__#$event_enum#g;
open (OUT, "> event.h") or die "cannot open event.h: $!\n";
print OUT $content;
close (OUT);

$content = "";
open (IN, "< event.cpp.in") or die "cannot open event.cpp.h: $!\n";
while (<IN>) { $content .= $_; }
close (IN);
$content =~ s#__CONTEXT_DESCR__#$context_descr#g;
$content =~ s#__EVENT_DESCR__#$event_descr#g;
$content =~ s#__EVENT_VALID__#$event_valid#g;
open (OUT, "> event.cpp") or die "cannot open event.cpp: $!\n";
print OUT $content;
close (OUT);
