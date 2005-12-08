
/* Example: format buffer */

/* for write(2) */
#include <sys/types.h>
#include <sys/uio.h>
#include <unistd.h>
/* for buffer_*() */
#include "core/buffer.h"

int main(int argc,char** argv) {
  buffer_t buf;
  int i=0;

  /* important! */
  buffer_init(&buf);

  /* printf() format: 'argc: dual=%032d/oct=%d/dec=%d/hex=0x%d\n' */
  buffer_add_str(&buf,"argc: dual=",-1);
  buffer_add_num2(&buf,argc,32,2); /* binary: exactly 32 digits */
  buffer_add_str(&buf,"/oct=",-1);
  buffer_add_num2(&buf,argc,-1,8);
  buffer_add_str(&buf,"/dec=",-1);
  buffer_add_num(&buf,argc,-1);
  buffer_add_str(&buf,"/hex=0x",-1);
  buffer_add_num2(&buf,argc,8,16); /* hex: exactly 8 digits */
  buffer_add_ch(&buf,'\n');

  for(i=0; i<argc; i++) {
    /* for each argument, printf() format: 'argv[%d]=\'%s\'\n' */
    buffer_add_str(&buf,"argv[",-1);
    buffer_add_num(&buf,i,-1);
    buffer_add_str(&buf,"]='",-1);
    buffer_add_str(&buf,argv[i],-1);
    buffer_add_str(&buf,"'\n",-1);
  }

  /*
   * all of the output is now in one large buffer
   * we already know the length of so even str_len() can
   * be avoided.
   */
  write(1,buf.str,buf.len);

  /* important! */
  buffer_free(&buf);

  return 0;
}
