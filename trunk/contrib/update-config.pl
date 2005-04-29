#!/usr/bin/perl -w

# purpose:
# update var names for mutt-ng
# update:
# grep ', DT_SYN' init.h | awk -F , '{print $1 $4}' | sed -e 's/[{UL]//g' -e 's/"  "/" => "/g'

use strict;

my %opts = (
  " edit_hdrs"                   => " edit_headers",
  " forw_decode"                 => " forward_decode",
  " forw_format"                 => " forward_format",
  " forw_quote"                  => " forward_quote",
  " hdr_format"                  => " index_format",
  " indent_str"                  => " indent_string",
  " mime_fwd"                    => " mime_forward",
  " msg_format"                  => " message_format",
  " pgp_autosign"                => " crypt_autosign",
  " pgp_autoencrypt"             => " crypt_autoencrypt",
  " pgp_replyencrypt"            => " crypt_replyencrypt",
  " pgp_replysign"               => " crypt_replysign",
  " pgp_replysignencrypted"      => " crypt_replysignencrypted",
  " pgp_verify_sig"              => " crypt_verify_sig",
  " pgp_create_traditional"      => " pgp_autoinline",
  " pgp_auto_traditional"        => " pgp_replyinline",
  " forw_decrypt"                => " forward_decrypt",
  " smime_sign_as"               => " smime_default_key",
  " post_indent_str"             => " post_indent_string",
  " print_cmd"                   => " print_command",
  " shorten_hierarchy"           => " sidebar_shorten_hierarchy"
);

if (@ARGV == 0) {
  die "Usage: $0 <conffiles>\n";
}

foreach my $f (@ARGV) {
  if (open (INPUT, "<" . $f)) {
    my @l = ();
    while (<INPUT>) {
      foreach my $v (keys %opts) {
        $_ =~ s/($v)/$opts{$1}/;
      }
      push (@l, $_);
    }
    close (INPUT);
    if (open (OUTPUT, ">" . $f)) {
      print OUTPUT @l;
      close (OUTPUT);
    }
  }
}
