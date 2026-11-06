/*
  user_config_secrets.h - credentials for the Gruff relay firmware. Not in
  git: compile.sh copies it next to user_config_override.h, which includes it.
  Keep the values in Bitwarden. Copy this file to user_config_secrets.h.
*/

#undef  STA_SSID1
#define STA_SSID1              "<wifi ssid>"
#undef  STA_PASS1
#define STA_PASS1              "<wifi password>"

// On site, tried when the first network isn't available.
#undef  STA_SSID2
#define STA_SSID2              "<on-site wifi ssid>"
#undef  STA_PASS2
#define STA_PASS2              "<on-site wifi password>"

// The relay's web UI and console: user admin. Letters and digits only, so it
// also works in Backlog commands.
#undef  WEB_PASSWORD
#define WEB_PASSWORD           "<24 random letters and digits>"
