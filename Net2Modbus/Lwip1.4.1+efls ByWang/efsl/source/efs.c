/*****************************************************************************/
/*              efs - General purpose Embedded Filesystem library              *
*          --------------------- -----------------------------------          *
*                                                                             *
* Filename : efs.h                                                            *
* Description : This should become the wrapper around efs. It will contain    *
*               functions like efs_init etc.                                  *
*                                                                             *
* This program is free software; you can redistribute it and/or               *
* modify it under the terms of the GNU General Public License                 *
* as published by the Free Software Foundation; version 2                     *
* of the License.                                                             *
                                                                              *
* This program is distributed in the hope that it will be useful,             *
* but WITHOUT ANY WARRANTY; without even the implied warranty of              *
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the               *
* GNU General Public License for more details.                                *
*                                                                             *
* As a special exception, if other files instantiate templates or             *
* use macros or inline functions from this file, or you compile this          *
* file and link it with other works to produce a work based on this file,     *
* this file does not by itself cause the resulting work to be covered         *
* by the GNU General Public License. However the source code for this         *
* file must still be made available in accordance with section (3) of         *
* the GNU General Public License.                                             *
*                                                                             *
* This exception does not invalidate any other reasons why a work based       *
* on this file might be covered by the GNU General Public License.            *
*                                                                             *
*                                                    (c)2006 Lennart Yseboodt *
*                                                    (c)2006 Michael De Nil   */
/*****************************************************************************/

/*****************************************************************************/
#include "efs.h"
/*****************************************************************************/

/* ****************************************************************************  
 * esint8 efs_init(EmbeddedFileSystem * efs,eint8* opts)
 * Description: This function initialises all subelements of a filesystem.
 * It sets the pointerchain and verifies each step.
 * Return value: 0 on success and -1 on failure.
*/
esint8 efs_init(EmbeddedFileSystem * efs,eint8* opts)
{
	if(if_initInterface(&efs->myCard, opts)==0)	//初始化SD卡的底层和SD卡，并返回总的扇区数。
	{	   //输入输出管理初始化
		ioman_init(&efs->myIOman,&efs->myCard,0);
		//磁盘初始化
		disc_initDisc(&efs->myDisc, &efs->myIOman);
		//分区初始化，主要是做了找到哪一个分区是有效的，因为SD卡只有一个分区表可用。虽然有4分区
		//part->activePartition有效的分区消息被放入这个里面
		part_initPartition(&efs->myPart, &efs->myDisc);

		if(efs->myPart.activePartition==-1){//磁盘里面没有找到有效的分区
			efs->myDisc.partitions[0].type=0x0B;
			efs->myDisc.partitions[0].LBA_begin=0;
			efs->myDisc.partitions[0].numSectors=efs->myCard.sectorCount;	
			/*efs->myPart.activePartition = 0;*/
			/*efs->myPart.disc = &(efs->myDisc);*/
			part_initPartition(&efs->myPart, &efs->myDisc);
		} 
		 //磁盘里面找到了有效的分区，开始初始化文件系统
		if(fs_initFs(&efs->myFs, &efs->myPart))
			return(-2);
		return(0);
	}
	return(-1);
}
/*****************************************************************************/


