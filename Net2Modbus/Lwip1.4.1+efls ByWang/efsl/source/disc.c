/*****************************************************************************/
/*              efs - General purpose Embedded Filesystem library              *
*          --------------------- -----------------------------------          *
*                                                                             *
* Filename : disc.c                                                           *
* Description : This file contains the functions regarding the whole disc     *
*               such as loading the MBR and performing read/write tests.      *
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
#include "disc.h"
/*****************************************************************************/

/* ****************************************************************************  
 * void disc_initDisc(Disc *disc,hcInterface* source)
 * Description: This initialises the disc by loading the MBR and setting the
 * pointer to the hardware object.
*/
void disc_initDisc(Disc *disc,IOManager* ioman)
{
	disc->ioman=ioman; //磁盘IO管理使用上面对IO初始化的IO管理实体
	disc_setError(disc,DISC_NOERROR);//设置磁盘错误为没有
	disc_loadMBR(disc);	//加载MBR到磁盘 disc.partitions
	//加载完毕后系统可以知道启动扇区的位置和扇区数量 
}
/*****************************************************************************/ 

/* ****************************************************************************  
 * void disc_loadMBR(Disc *disc)
 * Description: This functions copies the partitiontable to the partitions field.
*/
void disc_loadMBR(Disc *disc)
{

	euint8 x;
	euint8 *buf;
		//取得扇区	LBA_ADDR_MBR这个是0，但是2.0的卡就不是0而
		//而是读取0扇区后BPB的指向地址，这个文件系统并没有做处理，需要打补丁。
		// ioman_getSector针对于MBR的话就是从底层读取逻辑0扇区的数据到高速缓存中
	buf=ioman_getSector(disc->ioman,LBA_ADDR_MBR,IOM_MODE_READONLY|IOM_MODE_EXP_REQ);
	for(x=0;x<4;x++){
	//加载MBR中的信息到分区表中。MBR一共4个分区表所以这里定义4个
		ex_getPartitionField(buf,&(disc->partitions[x]),PARTITION_TABLE_OFFSET+(x*SIZE_PARTITION_FIELD));
	}
	ioman_releaseSector(disc->ioman,buf);
}
/*****************************************************************************/ 


