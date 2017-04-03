# ELFparasite

Simple parasite for ELF binaries. Tested on Linux x86_64. Adds a
bindshell on port 4444 to an existing binary.

This is probably the most simple method of infecting an ELF
binary. The malicious payload is prepended to an existing binary with
code to figure out where the original program starts, copying the
program to a temporary location, and executing it.

