#!/usr/bin/perl -w

my %vars = ();
my $mode = 0;
my $last = "";

my %pretty = (
  "BOOL"        => "boolean",
  "NUM"         => "number",
  "STRING"      => "string"
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
  if ($_ =~ /^[\s\t]+{[\s\t]+([^\s\t,]+)[\s\t]*,[\s\t]+([^\s\t,]+)[\s\t]*,[\s\t]+"(.+)$/) {
    if ($1 eq "0") {
      next;
    }
    my $type = $1;
    my $var = $2;
    my $init = clean (get_init ($3));
    $type =~ s#^[^:]+::T_##,
    $var =~ s#^"##; $var =~ s#"$##;
    # warn ":: type=$pretty{$type} name=$var init=$init\n";
    $vars{$var}{'type'} = $pretty{$type};
    $vars{$var}{'init'} = $init;
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
open (OUT, "> ./../../../doc/var_def.xml") or die "Cannot open var_def.xml: $!\n";
print OUT "<definitions>\n";
foreach my $v (sort keys (%vars)) {
  print OUT "  <variable name=\"$v\"/>\n";
}
print OUT "</definitions>\n";
close (OUT);

# dump XML descriptions file
open (OUT, "> ./../../../doc/var_descr.xml") or die "Cannot open var_descr.xml: $!\n";
print OUT "<descriptions>\n";
foreach my $v (sort keys (%vars)) {
  print OUT "  <variable name=\"$v\" type=\"$vars{$v}{'type'}\">\n";
  print OUT "    <init>$vars{$v}{'init'}</init>\n";
  if (defined $vars{$v}{'avail'}) {
    print OUT "    <avail>$vars{$v}{'avail'}</avail>\n";
  }
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

# dump doxygen related page
open (OUT, "> ./vars.h") or die "Cannot open vars.h: $!\n";
print OUT "/**\n* \@file muttng/config/vars.h\n* \@brief (AUTO) Option Reference\n*/\n";
print OUT "/**\n* \@page page_options Option Reference\n* \n";
foreach my $v (sort keys (%vars)) {
  my $init = $vars{$v}{'init'};
#  $init =~ s#&#&amp;#g;
#  $init =~ s#<#&lt;#g;
#  $init =~ s#>#&gt;#g;
  $init =~ s#\\#\\\\#g;
  #warn "$init\n";
  print OUT "* \@section option_$v \$$v\n";
  print OUT "*   Type: <code>$vars{$v}{'type'}</code>, ";
  print OUT " Initial value: <code>'$init'</code>";
  if (defined $vars{$v}{'avail'}) {
    print OUT ", Availability: $vars{$v}{'avail'}";
  }
  print OUT "<br> <br>\n";
  if (defined $vars{$v}{'descr'}) {
    foreach my $l (split (/\n/, $vars{$v}{'descr'})) {
      print OUT "*   $l\n";
    }
  }
}
print OUT "*/\n";
close (OUT);
