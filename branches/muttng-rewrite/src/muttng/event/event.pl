#!/usr/bin/perl -w

my $first = 1;

sub xmlclean ($) { # {{{
  my ($input) = (@_);
  my $output = $input;
  $output =~ s#&#&amp;#g;
  $output =~ s#<#&lt;#g;
  $output =~ s#>#&gt;#g;
  return ("$output");
} # }}}

# parse ./CONTEXTS {{{
my @contexts = ();
my $context_enum = "";
my $context_descr = "";
my %context_str = ();
open (IN, "< ./CONTEXTS") or die "cannot open ./CONTEXTS: $!\n";
while (<IN>) {
  chomp;
  if ($_ =~ /^(C_[A-Z]+)[\s\t]+([A-Za-z]+)[\s\t]*$/) {
    push (@contexts, $1);
    if (!$first) {
      $context_enum .= "\n";
      $context_descr .= "\n";
    }
    $context_enum .= "      /** context ID for \"$2\" */\n";
    $context_enum .= "      ".$1;
    $context_str{$1} = $2;
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

# parse ./GROUPS {{{
my @groups = ();
my $group_enum = "";
my $group_descr = "";
my %group_str = ();
open (IN, "< ./GROUPS") or die "cannot open ./GROUPS: $!\n";
while (<IN>) {
  chomp;
  if ($_ =~ /^(G_[A-Z]+)[\s\t]+(.+)$/) {
    push (@groups, $1);
    if (!$first) {
      $group_enum .= "\n";
      $group_descr .= "\n";
    }
    $group_enum .= "      /** group ID for \"$2\" */\n";
    $group_enum .= "      ".$1;
    $group_str{$1} = $2;
    if ($first == 1) {
      $first = 0;
      $group_enum .= " = 0";
    }
    $group_enum .= ",";
    $group_descr .= "  /** group string for $1 */\n";
    $group_descr .= "  \"$2\",";
  }
}
close (IN);
$first = 1;
# }}}

# parse ./EVENTS {{{
my @events = ();
my $event_enum = "";
my $event_descr = "";
my $event_help = "";
my $event_valid = "";
my %event_doc = ();
my %map1 = ();
my %map2 = ();
my $bind = "";
open (IN, "< ./EVENTS") or die "cannot open ./EVENTS: $!\n";
while (<IN>) {
  chomp;
  if ($_ =~ /^([A-Z|_]+)[\s\t]+(E_[A-Z_]+)[\s\t]+([^\s]+)[\s\t]+([^\s]+)[\s\t]+([^\s]+)[\s\t]+(.+)$/) {
    #warn "ctx='$1', ev='$2', func='$3', cat='$4', key='$5', descr='$6'\n";
    push (@events, $2);
    if (!$first) {
      $event_enum .= "\n";
      $event_cat .= "\n";
      $event_descr .= "\n";
      $event_help .= "\n";
      $event_valid .= "\n";
    }
    $map2{$4}{$2} = 1;
    $event_enum .= "      /** event ID for <code>\&lt;$3\&gt;</code> */\n";
    $event_enum .= "      $2";
    $event_cat .= "  /** event category for $2 */\n";
    if ($4 eq "NULL") {
      $event_cat .= "  NULL,";
    } else {
      $event_cat .= "  N_(\"$4\"),";
    }
    $event_descr .= "  /** event string for $2 */\n";
    $event_help .= "  /** help string for $2 */\n";
    if ($3 ne "NULL") {
      $event_descr .= "  \"$3\",";
    } else {
      $event_descr .= "  \"$2\",";
    }
    if ($6 ne "NULL") {
      $event_help .= "  N_(\"$6\"),";
    } else {
      $event_help .= "  \"$2\",";
    }
    if ($first == 1) {
      $first = 0;
      $event_enum .= " = 0";
    }
    $event_enum .= ",";
    my @ctx = split (/\|/, $1);
    $event_valid .= "  /** for which contexts $2 is valid */\n";
    my $delim = "  ";
    foreach my $c (@ctx) {
      $map1{$c}{$4} = 1;
      $event_valid .= $delim."CTX(Event::$c)";
      $delim = " | ";
      # if description is not NULL, we have a user function
      if ($6 ne "NULL") {
        $event_doc{$c}{$3}{'context'} = $context_str{$c};
        $event_doc{$c}{$3}{'func'} = $3;
        $event_doc{$c}{$3}{'cat'} = $4;
        $event_doc{$c}{$3}{'key'} = &xmlclean("$5");
        $event_doc{$c}{$3}{'descr'} = $6;
        $bind .= "  bindings[$c][$2].key = str_dup (\"$5\");\n";
        $bind .= "  bindings[$c][$2].defkey = \"$5\";\n";
        $bind .= "  bindings[$c][$2].name = EvStr[$2];\n";
        $bind .= "  bindings[$c][$2].help = EvHelp[$2];\n";
      }
    }
    $event_valid .= ",";
  }
}
close (IN);

$first = 1;
# }}}

my $group_valid = "";
foreach my $c (@contexts) {
  my $first1 = "  ";
  my $count = 0;
  $group_valid .= "  /** valid groups for context Event::$c */\n";
  foreach my $g (@groups) {
    if (defined ($map1{$c}{$g})) {
      $group_valid .= "${first1}CTX(Event::$g)";
      $first1 = "|";
      $count++;
    }
  }
  if ($count == 0) {
    $group_valid .= "  0";
  }
  $group_valid .= ",\n";
}

my $group_valid2 = "";
foreach my $c (@events) {
  my $first1 = "  ";
  my $count = 0;
  $group_valid2 .= "  /** valid groups for event Event::$c */\n";
  foreach my $g (@groups) {
    if (defined ($map2{$g}{$c})) {
      $group_valid2 .= "${first1}Event::$g";
      $first1 = "|";
      $count++;
    }
  }
  if ($count == 0) {
    $group_valid2 .= "  0";
  }
  $group_valid2 .= ",\n";
}

open (IN, "< event.h.in") or die "cannot open event.in.h: $!\n";
$content = "";
while (<IN>) { $content .= $_; }
close (IN);
$content =~ s#__CONTEXT_ENUM__#$context_enum#g;
$content =~ s#__EVENT_ENUM__#$event_enum#g;
$content =~ s#__GROUP_ENUM__#$group_enum#g;
open (OUT, "> event.h") or die "cannot open event.h: $!\n";
print OUT $content;
close (OUT);

$content = "";
open (IN, "< event.cpp.in") or die "cannot open event.cpp.h: $!\n";
while (<IN>) { $content .= $_; }
close (IN);
$content =~ s#__CONTEXT_DESCR__#$context_descr#g;
$content =~ s#__EVENT_DESCR__#$event_descr#g;
$content =~ s#__EVENT_HELP__#$event_help#g;
$content =~ s#__GROUP_DESCR__#$group_descr#g;
$content =~ s#__EVENT_VALID__#$event_valid#g;
$content =~ s#__GROUP_VALID__#$group_valid#g;
$content =~ s#__GROUP_VALID2__#$group_valid2#g;
$content =~ s#__BIND__#$bind#g;
open (OUT, "> event.cpp") or die "cannot open event.cpp: $!\n";
print OUT $content;
close (OUT);

open (OUT, "> ./../../../doc/manual_en/func_def.xml") or die "Cannot open func_def.xml: $!\n";
print OUT "<definitions>\n";
foreach my $ctx (sort keys (%context_str)) {
  my $key = 
  print OUT "  <context name=\"$context_str{$ctx}\">\n";
  foreach my $func (sort keys (%{$event_doc{$ctx}})) {
    print OUT "    <function name=\"$func\"/>\n";
  }
  print OUT "  </context>\n";
}
print OUT "</definitions>\n";
close (OUT);

open (OUT, "> ./../../../doc/manual_en/func_descr.xml") or die "Cannot open func_descr.xml: $!\n";
print OUT "<descriptions>\n";
foreach my $ctx (sort keys (%context_str)) {
  my $key = 
  print OUT "  <context name=\"$context_str{$ctx}\">\n";
  foreach my $func (sort keys (%{$event_doc{$ctx}})) {
    print OUT "    <function name=\"$func\" ".
                         "default=\"$event_doc{$ctx}{$func}{'key'}\" ".
                         "group=\"$event_doc{$ctx}{$func}{'cat'}\"".
          ">\n      $event_doc{$ctx}{$func}{'descr'}</function>\n";
  }
  print OUT "  </context>\n";
}
print OUT "</descriptions>\n";
close (OUT);
