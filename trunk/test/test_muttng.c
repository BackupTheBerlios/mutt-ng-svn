/* based on MinUnit */
#include <stdio.h>
#include <stdlib.h>
#include "test_muttng.h"

#include <lib.h>

int tests_run = 0;
int asserts_run = 0;

/* XXXXXXXXXXXXXXXXXXXXX */
/* the following things are only here to satisfy the linker */
int Umask;
void (*mutt_error) (const char *, ...);
const char * gettext(const char * a) { return a; }
void mutt_exit(int i) { exit(i); }
/* XXXXXXXXXXXXXXXXXXXX */


/* tests the basic functions in lib.c */
static char * test_lib(void) {
  /* first check whether allocating 0 bytes behaves correctly */
  mu_assert("safe_malloc(0)!=NULL",safe_malloc(0)==NULL);
  mu_assert("safe_calloc(0,0)!=NULL",safe_calloc(0,0)==NULL);
  mu_assert("safe_calloc(1,0)!=NULL",safe_calloc(1,0)==NULL);
  mu_assert("safe_calloc(0,1)!=NULL",safe_calloc(0,1)==NULL);

  /* check whether safe_malloc()/safe_free work correctly */
  {
    char * ptr;
    ptr = safe_malloc(1);
    mu_assert("safe_malloc(1)==NULL",ptr!=NULL);
    safe_free(&ptr);
    mu_assert("ptr!=NULL",ptr==NULL);
  }

  /* check whether safe_strdup works correctly */
  {
    char * ptr;
    mu_assert("safe_strdup(NULL)!=NULL",safe_strdup(NULL)==NULL);
    mu_assert("safe_strdup("")!=NULL",safe_strdup("")==NULL);
    ptr = safe_strdup("teststring");
    mu_assert("safe_strdup(\"teststring\")!=\"teststring\"",strcmp(ptr,"teststring")==0);
    safe_free(&ptr);

  }

  /* check whether safe_strcat works correctly */
  {
    char buf[16] = { 0 };
    safe_strcat(buf,sizeof(buf),"asdf");
    mu_assert("buf != \"asdf\"",strcmp(buf,"asdf")==0);
    safe_strcat(buf,sizeof(buf),"qwer");
    mu_assert("buf != \"asdfqwer\"",strcmp(buf,"asdfqwer")==0);
    safe_strcat(buf,sizeof(buf),"0123456789");
    mu_assert("buf != \"asdfqwer0123456\"",strcmp(buf,"asdfqwer0123456")==0);
    safe_strcat(buf,sizeof(buf),"trash");
    mu_assert("buf != \"asdfqwer0123456\" (2)",strcmp(buf,"asdfqwer0123456")==0);
  }

  /* check whether safe_strncat works correctly */
  {
    char buf[16] = { 0 };
    safe_strncat(buf,sizeof(buf),"asdf",3);
    mu_assert("buf != \"asd\"",strcmp(buf,"asd")==0);
    safe_strncat(buf,sizeof(buf),"fghj",3);
    mu_assert("buf != \"asdfgh\"",strcmp(buf,"asdfgh")==0);
    safe_strncat(buf,sizeof(buf),"",10);
    mu_assert("buf != \"asdfgh\" (2)",strcmp(buf,"asdfgh")==0);
    safe_strncat(buf,sizeof(buf),"qwertzuiopyxcvvbnm",255);
    mu_assert("buf != \"asdfghqwertzuio\"",strcmp(buf,"asdfghqwertzuio")==0);
  }

  /* check whether mutt_str_replace works correctly */
  {
    char * ptr = NULL;
    mutt_str_replace(&ptr,"foobar");
    mu_assert("ptr != \"foobar\"",strcmp(ptr,"foobar")==0);
    mutt_str_replace(&ptr,"quux");
    mu_assert("ptr != \"quux\"",strcmp(ptr,"quux")==0);
    mutt_str_replace(&ptr,NULL);
    mu_assert("ptr != NULL",ptr==NULL);
  }

  /* check whether mutt_str_adjust works correctly */
  {
    char * ptr = safe_strdup("some teststring");
    mutt_str_adjust(&ptr);
    mu_assert("ptr != \"some teststring\"",strcmp(ptr,"some teststring")==0);
    safe_free(&ptr);
  }

  /* check whether mutt_strlower works correctly */
  {
     char * supposed_result = "all you need is love";
     char * ptr = safe_strdup("ALL YOU NEED IS LOVE");
     mu_assert("ptr != \"all you need is love\"",strcmp(mutt_strlower(ptr),supposed_result)==0);
     mutt_str_replace(&ptr,"All You Need Is LovE");
     mu_assert("ptr != \"all you need is love\" (2)",strcmp(mutt_strlower(ptr),supposed_result)==0);
     mutt_str_replace(&ptr,"all you need is love");
     mu_assert("ptr != \"all you need is love\" (3)",strcmp(mutt_strlower(ptr),supposed_result)==0);
     safe_free(&ptr);
  }

  return 0;
}

static char * all_tests(void) {
  mu_run_test(test_lib);
  return 0;
}

int main(int argc, char **argv) {
  char *result = all_tests();
  if (result != 0) {
    printf("ERROR: %s\n", result);
  } else {
    printf("ALL TESTS PASSED\n");
  }
  printf("Tests run: %d\nAssertions checked: %d\n", tests_run,asserts_run);
  return result != 0;
}
