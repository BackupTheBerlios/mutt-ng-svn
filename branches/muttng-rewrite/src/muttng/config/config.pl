#!/usr/bin/perl -w

my %vars = ();
my $mode = 0;
my $last = "";

my %pretty = (
  "BOOL"        => "boolean",
  "NUM"         => "number",
  "STRING"      => "string",
  "QUAD"        => "quad-option",
  "URL"         => "url"
);

my %sigs = (
  "0"           => "no",
  "1"           => "yes"
);

sub get_init ($) { # get initial value from initial portion of string {{{

  my ($line) = @_;
  for (my $i = 0; $i < length ($line); $i++) {
    if (substr ($line, $i, 1) eq "\\") {
      $i++;
      next;
    } elsif (substr ($line, $i, 1) eq "\"") {
      return (substr ($line, 0, $i));
    }
  }
  return ($line);
} # }}}

sub clean ($) { # strip one level of \ from string {{{
  my ($init) = @_;
  my @line = ();
  my $c = 0;
  for (my $i = 0; $i < length ($init); $i++) {
    if (substr ($init, $i, 1) eq "\\") {
      $i++;
    }
    $line[$c++] = substr ($init, $i, 1);
  }
  return (join ('', @line));
} # }}}

sub todoxy($) {
  my ($line) = (@_);
  my $clean = $line;
  $clean =~ s#<man>([^<]+)</man>#<tt>$1(1)</tt>#g;
  $clean =~ s#<varref>([^<]+)</varref>#\@ref option_$1#g;
  $clean =~ s#<man sect="([1-9])">([^<]+)</man>#<tt>$2($1)</tt>#g;
  $clean =~ s#([</])val>#$1tt>#g;
  $clean =~ s#([</])enc>#$1tt>#g;
  return ("$clean");
}

while (<STDIN>) {
  chomp;

  if ($mode == 0 and $_ =~ /^[\s\t]+\/\* START \*\/[\s\t]*$/) {
    $mode = 1;
  } elsif ($mode == 1 and $_ =~ /^[\s\t]+\/\* END \*\/[\s\t]*$/) {
    $mode = 0;
  } elsif ($mode == 0) {
    next;
  }

  # match against variable descr
  if ($_ =~ /^[\s\t]+{[\s\t]+([^\s\t,]+)[\s\t]*,[\s\t]+([^\s\t,]+)[\s\t]*,[\s\t]+"(.+)[\s\t]*([0-9])[\s\t]*}[\s\t]*,[\s\t]*$/) {
    if ($1 eq "0") {
      next;
    }
    my $type = $1;
    my $var = $2;
    my $init = clean (get_init ($3));
    my $sig = $4;
    $type =~ s#^[^:]+::T_##,
    $var =~ s#^"##; $var =~ s#"$##;
    # warn ":: type=$pretty{$type} name=$var init=$init sig=$sig\n";
    $vars{$var}{'type'} = $pretty{$type};
    $vars{$var}{'init'} = $init;
    $vars{$var}{'sig'} = $sig;
    $last = $var;
  } elsif ($_ =~ /^[\s\t]+\*\* (.+)$/) {
    if ($1 =~ /^avail: (.+)$/) {
      $vars{$last}{'avail'} = $1;
    } else {
      $vars{$last}{'descr'} .= $1."\n";
    }
  }
}

# dump XML definitions file
open (OUT, "> ./../../../doc/manual_en/var_def.xml") or die "Cannot open var_def.xml: $!\n";
print OUT "<definitions>\n";
foreach my $v (sort keys (%vars)) {
  print OUT "  <variable name=\"$v\"/>\n";
}
print OUT "</definitions>\n";
close (OUT);

# dump XML descriptions file
open (OUT, "> ./../../../doc/manual_en/var_descr.xml") or die "Cannot open var_descr.xml: $!\n";
print OUT "<descriptions>\n";
foreach my $v (sort keys (%vars)) {
  print OUT "  <variable name=\"$v\" type=\"$vars{$v}{'type'}\">\n";
  print OUT "    <init>$vars{$v}{'init'}</init>\n";
  if (defined $vars{$v}{'avail'}) {
    print OUT "    <avail>$vars{$v}{'avail'}</avail>\n";
  }
  print OUT "    <sig>$sigs{$vars{$v}{'sig'}}</sig>\n";
  if (defined $vars{$v}{'descr'}) {
    my @descr = split (/\n/, $vars{$v}{'descr'});
    print OUT "    <descr>\n";
    foreach my $l (@descr) {
      print OUT "      $l\n";
    }
    print OUT "    </descr>\n";
  }
  print OUT "  </variable>\n";
}
print OUT "</descriptions>\n";
close (OUT);
