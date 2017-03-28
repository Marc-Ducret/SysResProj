#ifndef SYSCALL_H
#define SYSCALL_H
pid_t kfork(void);
pid_t kwait(int *status);
void kexit(int status);

// Utilisera-t-on des file descriptors ? TODO
int ksend(chanid channel, void *buffer, int len);
int kreceive(chanid channel, void *buffer, int len);
chanid knew_channel();
void new_launch();
#endif /* SYSCALL_H */

