/* Copyright (C) 1998, 1999, 2000, 2001, 2002 Red Hat, Inc.
   Written by Ulrich Drepper <drepper@redhat.com>, 1998.

   This program is Open Source software; you can redistribute it and/or
   modify it under the terms of the Open Software License version 1.0 as
   published by the Open Source Initiative.

   You should have received a copy of the Open Software License along
   with this program; if not, you may obtain a copy of the Open Software
   License version 1.0 from http://www.opensource.org/licenses/osl.php or
   by writing the Open Source Initiative c/o Lawrence Rosen, Esq.,
   3001 King Ranch Road, Ukiah, CA 95482.   */

#include <config.h>

#include <errno.h>
#include <fcntl.h>
#include <gelf.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int
main (int argc, char *argv[])
{
  Elf *elf;
  int fd;
  GElf_Ehdr ehdr;
  int cnt;

  fd = open (argv[1], O_RDONLY);
  if (fd == -1)
    {
      printf ("cannot open \"%s\": %s\n", argv[1], strerror (errno));
      exit (1);
    }

  elf_version (EV_CURRENT);

  elf = elf_begin (fd, ELF_C_READ, NULL);
  if (elf == NULL)
    {
      printf ("cannot open ELF file: %s\n", elf_errmsg (-1));
      exit (1);
    }

  if (elf_kind (elf) != ELF_K_ELF)
    {
      printf ("\"%s\" is not an ELF file\n", argv[1]);
      exit (1);
    }

  if (gelf_getehdr (elf, &ehdr) == NULL)
    {
      printf ("cannot get the ELF header: %s\n", elf_errmsg (-1));
      exit (1);
    }

  printf ("idx type    %*s %*s %*s %*s %*s  align flags\n",
	  gelf_getclass (elf) == ELFCLASS32 ? 9 : 17, "offset",
	  gelf_getclass (elf) == ELFCLASS32 ? 10 : 18, "vaddr",
	  gelf_getclass (elf) == ELFCLASS32 ? 10 : 18, "paddr",
	  gelf_getclass (elf) == ELFCLASS32 ? 9 : 12, "filesz",
	  gelf_getclass (elf) == ELFCLASS32 ? 9 : 12, "memsz");

  for (cnt = 0; cnt < ehdr.e_phnum; ++cnt)
    {
      static const char *typenames[] =
      {
	[PT_NULL] = "NULL",
	[PT_LOAD] = "LOAD",
	[PT_DYNAMIC] = "DYNAMIC",
	[PT_INTERP] = "INTERP",
	[PT_NOTE] = "NOTE",
	[PT_SHLIB] = "SHLIB",
	[PT_PHDR] = "PHDR"
      };
      GElf_Phdr mem;
      GElf_Phdr *phdr = gelf_getphdr (elf, cnt, &mem);
      char buf[19];
      const char *p_type = typenames[phdr->p_type];

      /* If we don't know the name of the type we use the number value.  */
      if (phdr->p_type >= PT_NUM)
	{
	  snprintf (buf, sizeof (buf), "%x", phdr->p_type);
	  p_type = buf;
	}

      printf ("%3d %-7s %#0*llx %#0*llx %#0*llx %#0*llx %#0*llx %#6llx ",
	      cnt, p_type,
	      gelf_getclass (elf) == ELFCLASS32 ? 9 : 17,
	      (unsigned long long int) phdr->p_offset,
	      gelf_getclass (elf) == ELFCLASS32 ? 10 : 18,
	      (unsigned long long int) phdr->p_vaddr,
	      gelf_getclass (elf) == ELFCLASS32 ? 10 : 18,
	      (unsigned long long int) phdr->p_paddr,
	      gelf_getclass (elf) == ELFCLASS32 ? 9 : 12,
	      (unsigned long long int) phdr->p_filesz,
	      gelf_getclass (elf) == ELFCLASS32 ? 9 : 12,
	      (unsigned long long int) phdr->p_memsz,
	      (unsigned long long int) phdr->p_align);

      putc_unlocked ((phdr->p_flags & PF_X) ? 'X' : ' ', stdout);
      putc_unlocked ((phdr->p_flags & PF_W) ? 'W' : ' ', stdout);
      putc_unlocked ((phdr->p_flags & PF_R) ? 'R' : ' ', stdout);

      putc_unlocked ('\n', stdout);

      if (phdr->p_type == PT_INTERP)
	{
	  /* We can show the user the name of the interpreter.  */
	  size_t maxsize;
	  char *filedata = elf_rawfile (elf, &maxsize);

	  if (filedata != NULL && phdr->p_offset < maxsize)
	    printf ("\t[Requesting program interpreter: %s]\n",
		    filedata + phdr->p_offset);
	}
    }

  if (elf_end (elf) != 0)
    {
      printf ("error while freeing ELF descriptor: %s\n", elf_errmsg (-1));
      exit (1);
    }

  return 0;
}
