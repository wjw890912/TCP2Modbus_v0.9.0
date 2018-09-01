


#ifndef __FILESYSTEM_H
#define __FILESYSTEM_H

#include "types.h"
#include "config.h"
#include "interface.h"
#include "disc.h"
#include "partition.h"
#include "fs.h"
#include "file.h"
#include "time.h"
#include "efs.h"

void FileSystemInit(void);
void FileSystemClose(EmbeddedFile * file,FileSystem* efs);
void FileSystemDelete(FileSystem *fs,euint8* filename);
char FS_Write(int f,char *p, unsigned int size);



#endif




