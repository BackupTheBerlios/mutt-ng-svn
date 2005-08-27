#!/usr/bin/perl -w

# purpose:
# update var names for mutt-ng
# update:
# grep ', DT_SYN' init.h | awk -F , '{print $1 $4}' | sed -e 's/[{UL]//g' -e 's/"  "/" => "/g'

use strict;

my %opts = (
  # left from the first run of cleanup+removal
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
  " shorten_hierarchy"           => " sidebar_shorten_hierarchy",
  # current run
  " ask_followup_to"              => " nntp_ask_followup_to", 
  " ask_x_comment_to"             => " nntp_ask_x_comment_to", 
  " catchup_newsgroup"            => " nntp_catchup", 
  " followup_to_poster"           => " nntp_followup_to_poster", 
  " group_index_format"           => " nntp_group_index_format", 
  " inews"                        => " nntp_inews", 
  " mime_subject"                 => " nntp_mime_subject", 
  " news_cache_dir"               => " nntp_cache_dir", 
  " news_server"                  => " nntp_host", 
  " newsrc"                       => " nntp_newsrc", 
  " nntp_poll"                    => " nntp_mail_check", 
  " pop_checkinterval"            => " pop_mail_check", 
  " post_moderated"               => " nntp_post_moderated", 
  " save_unsubscribed"            => " nntp_save_unsubscribed", 
  " show_new_news"                => " nntp_show_new_news", 
  " show_only_unread"             => " nntp_show_only_unread", 
  " x_comment_to"                 => " nntp_x_comment_to",
  # libesmtp
  " smtp_auth_username"           => " smtp_user",
  " smtp_auth_password"           => " smtp_pass",
  " user_agent"                   => " agent_string"
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
